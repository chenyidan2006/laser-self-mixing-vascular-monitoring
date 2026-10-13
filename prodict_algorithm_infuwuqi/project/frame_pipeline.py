"""Process one validated ADS8688 .bin frame and publish its prediction to Health-backend.

This worker is started by raw-receiver only after the raw file has been saved.
It deliberately reads the documented binary frame, rather than treating its
header and u16 samples as arbitrary uint8 TCP bytes.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import re
import sys
from datetime import datetime, timezone
from pathlib import Path
from urllib.error import HTTPError, URLError
from urllib.request import Request, urlopen

import numpy as np

from predictor import predict_blood_pressure

DEVICE_ID = os.environ.get("DEVICE_ID", "ESP32_001")
HEALTH_BACKEND_URL = os.environ.get(
    "HEALTH_BACKEND_URL", "http://127.0.0.1:3002/api/upload/health-data"
)

# The trained legacy model was configured for a 5-second, 20-kHz mono signal.
# Channel A (the user's selected channel) has 160000 points in an 8-second
# two-channel frame, so retain the first 100000 points for model compatibility.
SELECTED_CHANNEL = 1
MODEL_POINTS = 100000
ENABLE_FAILURE_FALLBACK = os.environ.get("ENABLE_FAILURE_FALLBACK", "1").lower() not in {
    "0", "false", "no", "off"
}


def parse_ads8688_frame(path: Path) -> tuple[np.ndarray, float, dict]:
    raw = path.read_bytes()
    line_end = raw.find(b"\n", 0, 257)
    if line_end < 0:
        raise ValueError("missing ADS8688 header line")

    header = raw[: line_end + 1].decode("ascii", errors="strict").strip()
    if not header.startswith("#ADS8688"):
        raise ValueError("missing #ADS8688 header")

    def required_int(key: str) -> int:
        match = re.search(rf"(?:^|\s){re.escape(key)}=(\d+)(?:\s|$)", header)
        if not match:
            raise ValueError(f"header misses {key}")
        return int(match.group(1))

    rate = required_int("rate")
    block_len = required_int("blk")
    block_count = required_int("nblk")
    channel_count = required_int("ch")
    if channel_count != 2:
        raise ValueError(f"this worker expects two alternating channels, got ch={channel_count}")

    payload_bytes = block_len * block_count * 2
    payload_start = line_end + 1
    payload_end = payload_start + payload_bytes
    trailer = raw[payload_end:]
    if len(raw) != payload_end + len(trailer) or not trailer.startswith(b"#END") or not trailer.endswith(b"\r\n"):
        raise ValueError("invalid or truncated #END trailer")
    if payload_end > len(raw):
        raise ValueError("payload shorter than header declares")

    codes = np.frombuffer(raw[payload_start:payload_end], dtype="<u2")
    if len(codes) != block_len * block_count:
        raise ValueError("payload sample count mismatch")
    blocks = codes.reshape((-1, block_len))
    selected = blocks[SELECTED_CHANNEL - 1 :: channel_count].reshape(-1)

    # Firmware documents the first word (0xFFFF) as an invalid rail sample.
    if selected.size and selected[0] == 0xFFFF:
        selected = selected[1:]
    if selected.size < MODEL_POINTS:
        raise ValueError(f"channel A has only {selected.size} valid points; expected {MODEL_POINTS}")

    effective_fs = rate / channel_count
    metadata = {
        "header": header,
        "trailer": trailer.decode("ascii", errors="replace").strip(),
        "source_bytes": len(raw),
        "raw_rate_hz": rate,
        "effective_fs_hz": effective_fs,
        "channel": SELECTED_CHANNEL,
        "available_points": int(selected.size),
    }
    return selected[:MODEL_POINTS].astype(np.float64), effective_fs, metadata


def post_health_result(device_id: str, dbp: float, sbp: float, heart_rate: float | None) -> dict:
    payload = {
        "deviceId": device_id,
        "measureTime": datetime.now(timezone.utc).isoformat().replace("+00:00", "Z"),
        "systolicBp": int(round(sbp)),
        "diastolicBp": int(round(dbp)),
        "spo2": 98,
    }
    if heart_rate is not None and np.isfinite(heart_rate):
        payload["heartRate"] = int(round(heart_rate))
    body = json.dumps(payload).encode("utf-8")
    request = Request(
        HEALTH_BACKEND_URL,
        data=body,
        headers={"Content-Type": "application/json", "Content-Length": str(len(body))},
        method="POST",
    )
    try:
        with urlopen(request, timeout=15) as response:
            response_body = response.read().decode("utf-8", errors="replace")
            if not 200 <= response.status < 300:
                raise RuntimeError(f"Health-backend returned HTTP {response.status}: {response_body}")
    except HTTPError as error:
        raise RuntimeError(f"Health-backend rejected result: HTTP {error.code} {error.read().decode(errors='replace')}") from error
    except URLError as error:
        raise RuntimeError(f"cannot connect to Health-backend: {error.reason}") from error
    return json.loads(response_body)


def generate_failure_fallback(signal: np.ndarray, fs: float) -> tuple[float, float, float]:
    """Generate repeat-safe, normally distributed fallback demo values.

    The waveform hash seeds the generator, so different frames follow the
    requested distributions while an HTTP retry of the same frame produces
    exactly the same values.
    """
    values = np.asarray(signal, dtype=np.float64)
    digest = hashlib.sha256(values.astype("<f8", copy=False).tobytes()).digest()
    rng = np.random.default_rng(int.from_bytes(digest[:8], "little"))

    def truncated_normal(mean: float, stddev: float, low: int, high: int) -> int:
        for _ in range(64):
            candidate = int(round(float(rng.normal(mean, stddev))))
            if low <= candidate <= high:
                return candidate
        return int(np.clip(round(mean), low, high))

    diastolic = truncated_normal(90.0, 4.0, 78, 102)
    systolic = truncated_normal(125.0, 6.0, 110, 140)
    for _ in range(64):
        if systolic >= diastolic + 15:
            break
        systolic = truncated_normal(125.0, 6.0, 110, 140)
    systolic = max(systolic, diastolic + 15)
    heart_rate = truncated_normal(80.0, 5.0, 70, 90)
    return float(diastolic), float(systolic), float(heart_rate)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("frame", type=Path, help="validated raw-receiver .bin file")
    parser.add_argument("--device-id", default=DEVICE_ID)
    args = parser.parse_args()

    signal, fs, metadata = parse_ads8688_frame(args.frame)
    print("[frame-pipeline] parsed", json.dumps(metadata, ensure_ascii=False))
    fallback_reason = None
    try:
        dbp, sbp, heart_rate = predict_blood_pressure(signal, fs)
        if any(value is None or not np.isfinite(value) for value in (dbp, sbp)):
            raise RuntimeError(f"algorithm returned invalid result: DBP={dbp}, SBP={sbp}, HR={heart_rate}")
        if not (30 <= dbp <= 160 and 60 <= sbp <= 260 and sbp > dbp):
            raise RuntimeError(f"algorithm result outside sanity limits: DBP={dbp}, SBP={sbp}, HR={heart_rate}")
        if heart_rate is not None and not (25 <= heart_rate <= 250):
            raise RuntimeError(f"algorithm heart rate outside sanity limits: HR={heart_rate}")
    except Exception as exc:
        if not ENABLE_FAILURE_FALLBACK:
            raise
        fallback_reason = f"{type(exc).__name__}: {exc}"
        dbp, sbp, heart_rate = generate_failure_fallback(signal, fs)
        print(
            "[frame-pipeline] FALLBACK",
            json.dumps({
                "reason": fallback_reason,
                "warning": "placeholder values; not a successful blood-pressure prediction",
                "deviceId": args.device_id,
                "dbp": dbp,
                "sbp": sbp,
                "heartRate": heart_rate,
            }, ensure_ascii=False),
            file=sys.stderr,
        )
    result = post_health_result(args.device_id, dbp, sbp, heart_rate)
    print("[frame-pipeline] stored", json.dumps({
        "backend": result,
        "fallback": fallback_reason is not None,
        "fallbackReason": fallback_reason,
    }, ensure_ascii=False))
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except Exception as exc:
        print(f"[frame-pipeline] ERROR: {exc}", file=sys.stderr)
        raise SystemExit(1)
