# -*- coding: utf-8 -*-
"""Python bridge for the legacy file-based callPredictFixed.exe.

The deployed MATLAB entry point accepts an input-file path.  That file contains
float64 sampling rate, int32 point count and float64 samples, all little-endian.
The broken rollback bridge wrote those bytes to stdin, so the EXE ignored them
and read a stale temp_signal.dat instead.
"""

import os
import random
import re
import struct
import subprocess
import sys
import tempfile
from pathlib import Path

import numpy as np


# Node reads child-process output as UTF-8.  Force Python to emit UTF-8 even
# when the Windows system code page is GBK, and never let a log character stop
# a completed prediction.
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="backslashreplace")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8", errors="backslashreplace")


EXE_PATH = os.environ.get(
    "PREDICTOR_EXE",
    str(Path(__file__).resolve().with_name("callPredictFixed.exe")),
)
# A frame must produce either a real model result or a fallback result within
# one minute.  An environment value may shorten this limit, but cannot extend
# it beyond 60 seconds.
try:
    _configured_timeout = int(os.environ.get("PREDICTOR_TIMEOUT_SECONDS", "60"))
except ValueError:
    _configured_timeout = 60
PREDICTOR_TIMEOUT_SECONDS = max(1, min(60, _configured_timeout))


def _decode_process_output(data: bytes) -> str:
    """Decode MATLAB Runtime output from either UTF-8 or Chinese Windows."""
    if not data:
        return ""
    for encoding in ("utf-8", "gb18030"):
        try:
            return data.decode(encoding).strip()
        except UnicodeDecodeError:
            pass
    return data.decode("utf-8", errors="replace").strip()


def _decode_result_file(data: bytes, original_size: int):
    """Read the deployed MATLAB EXE's file-based result contract.

    Known builds either truncate the input file to three little-endian doubles
    or append those 24 bytes after the original input.  Never interpret an
    unchanged signal file as a result.
    """
    payload = None
    if len(data) == 24:
        payload = data
    elif len(data) >= original_size + 24:
        payload = data[-24:]
    if payload is None:
        return None
    values = struct.unpack("<ddd", payload)
    if not all(np.isfinite(value) for value in values):
        return None
    return tuple(float(value) for value in values)


def predict_blood_pressure(signal_raw, fs, client_port=None):
    if client_port is None:
        client_port = random.randint(10000, 99999)

    if signal_raw is None or len(signal_raw) == 0:
        raise ValueError("[algorithm] input signal is empty")

    signal_raw = np.asarray(signal_raw, dtype=np.float64)
    current_len = len(signal_raw)
    print(
        f"[algorithm] input points={current_len}, sample_rate={float(fs):.2f}Hz",
        flush=True,
    )

    if current_len < 20000:
        duration = 100000.0 / float(fs)
        final_fs = float(current_len) / duration
        print(f"[algorithm] adjusted sample_rate={final_fs:.2f}Hz", flush=True)
    else:
        final_fs = float(fs)

    exe_path = Path(EXE_PATH)
    if not exe_path.is_file():
        raise FileNotFoundError(f"predictor executable not found: {exe_path}")

    input_path = None
    result_file_values = None
    original_file_size = 12 + current_len * 8
    try:
        # Keep the exact legacy EXE file contract.  A unique file avoids stale
        # temp_signal.dat data and prevents concurrent jobs sharing one input.
        with tempfile.NamedTemporaryFile(
            mode="wb",
            prefix="ppg_legacy_",
            suffix=".dat",
            dir=exe_path.parent,
            delete=False,
        ) as input_file:
            input_file.write(struct.pack("<dI", final_fs, current_len))
            input_file.write(signal_raw.astype("<f8", copy=False).tobytes())
            input_path = Path(input_file.name)

        print(f"[algorithm] starting predictor: {exe_path}", flush=True)
        proc = subprocess.run(
            [str(exe_path), str(input_path)],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            timeout=PREDICTOR_TIMEOUT_SECONDS,
            check=False,
            cwd=str(exe_path.parent),
        )
        # The deployed EXE reports its numeric result through the same physical
        # file.  Capture it before the temporary file is removed in finally.
        try:
            result_file_values = _decode_result_file(
                input_path.read_bytes(), original_file_size
            )
        except OSError:
            result_file_values = None
    except subprocess.TimeoutExpired as exc:
        raise RuntimeError(
            f"predictor timed out after {PREDICTOR_TIMEOUT_SECONDS} seconds"
        ) from exc
    finally:
        if input_path is not None:
            try:
                input_path.unlink(missing_ok=True)
            except OSError:
                pass

    matlab_log = _decode_process_output(proc.stdout)
    matlab_error = _decode_process_output(proc.stderr)
    if matlab_log:
        print("[algorithm] predictor stdout:", flush=True)
        print(matlab_log, flush=True)
    if matlab_error:
        print("[algorithm] predictor stderr:", file=sys.stderr, flush=True)
        print(matlab_error, file=sys.stderr, flush=True)
    if proc.returncode != 0:
        raise RuntimeError(
            f"MATLAB predictor exited {proc.returncode}: {matlab_error or matlab_log}"
        )

    # The executable explicitly tells us when feature cleaning failed.  Its
    # file in that branch contains only an internal placeholder, not a model
    # prediction, so do not promote that value to a successful measurement.
    if "特征为空" in matlab_log or "核心功能区拦截" in matlab_log:
        raise RuntimeError(f"MATLAB feature extraction rejected the waveform: {matlab_log}")

    dbp_match = re.search(r"DBP\s*=\s*([-+0-9.eE]+)", matlab_log)
    sbp_match = re.search(r"SBP\s*=\s*([-+0-9.eE]+)", matlab_log)
    hr_match = re.search(r"(?:心率|HR)\s*=\s*([-+0-9.eE]+)", matlab_log)
    if dbp_match and sbp_match:
        dbp = float(dbp_match.group(1))
        sbp = float(sbp_match.group(1))
        hr = float(hr_match.group(1)) if hr_match else None
        result_source = "stdout"
    elif result_file_values is not None:
        dbp, sbp, hr = result_file_values
        result_source = "result-file"
    else:
        raise RuntimeError(
            f"cannot parse predictor output or result file: {matlab_log or matlab_error}"
        )

    if int(round(dbp)) == 20000 or (dbp == 0.0 and sbp == 0.0):
        print(
            f"[algorithm] rejected model sentinel: DBP={dbp}, SBP={sbp}",
            flush=True,
        )
        return None, None, None

    print(f"[algorithm] prediction succeeded via {result_source}", flush=True)
    print(f"  DBP={dbp:.2f}mmHg", flush=True)
    print(f"  SBP={sbp:.2f}mmHg", flush=True)
    if hr is not None:
        print(f"  HR={hr:.1f}BPM", flush=True)
    return dbp, sbp, hr
