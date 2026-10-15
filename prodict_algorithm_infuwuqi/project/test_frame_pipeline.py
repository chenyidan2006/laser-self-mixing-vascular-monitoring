"""Synthetic regressions: never launch MATLAB or contact the backend."""
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch
import numpy as np
import frame_pipeline as pipeline


class FrameTests(unittest.TestCase):
    def parse(self, values, ch=2, suffix=b"#END\r\n", layout=""):
        header = f"#ADS8688 v1 rate=40000 secs=8 blk=2000 nblk=160 ch={ch} code=u16le {layout}\r\n".encode()
        with tempfile.TemporaryDirectory() as directory:
            file = Path(directory) / "synthetic.bin"
            file.write_bytes(header + np.asarray(values, dtype="<u2").tobytes() + suffix)
            return pipeline.parse_ads8688_frame(file)

    def test_continuous_ramp_retains_time_axis(self):
        values = 10000 + np.arange(320000) // 10
        signal, fs, meta = self.parse(values)
        self.assertEqual((fs, len(signal), meta['available_points']), (20000, 100000, 160000))
        np.testing.assert_allclose(signal[100:-100], values[::2][:100000][100:-100], atol=1)
        self.assertEqual(meta['physical_channels'], 1)
        other, _, _ = self.parse(values, ch=1)
        np.testing.assert_array_equal(signal, other)

    def test_antialias_rejects_fifteen_khz(self):
        t = np.arange(320000) / 40000
        values = np.round(30000 + 5000*np.sin(2*np.pi*1000*t) + 5000*np.sin(2*np.pi*15000*t))
        signal, _, _ = self.parse(values)
        y = signal[1000:-1000] - np.mean(signal[1000:-1000])
        freqs = np.fft.rfftfreq(len(y), 1/20000)
        spectrum = abs(np.fft.rfft(y))
        amplitude = lambda hz: spectrum[np.argmin(abs(freqs-hz))]
        self.assertLess(amplitude(5000)/amplitude(1000), 0.01)

    def test_rejects_truncated_and_old_layout(self):
        values = np.full(320000, 30000)
        for data, suffix, layout in [(values[:-1], b'#END\r\n', ''), (values, b'#END\r\nJUNK\r\n', ''), (values,b'#END\r\n','layout=block-interleaved')]:
            with self.assertRaises(ValueError): self.parse(data,suffix=suffix,layout=layout)

    def test_fallback_still_repeats_for_same_signal(self):
        a = np.arange(100000, dtype=float)
        self.assertTrue(pipeline.ENABLE_FAILURE_FALLBACK)
        self.assertEqual(pipeline.generate_failure_fallback(a,20000),pipeline.generate_failure_fallback(a,20000))

    def test_failure_still_uses_demo_fallback(self):
        with patch.object(pipeline,'parse_ads8688_frame',return_value=(np.arange(100000),20000,{})), \
             patch.object(pipeline,'predict_blood_pressure',side_effect=RuntimeError('synthetic')), \
             patch.object(pipeline,'post_health_result',return_value={}) as post, \
             patch('sys.argv',['frame_pipeline.py','synthetic.bin']):
            self.assertEqual(pipeline.main(),0)
            self.assertEqual(post.call_count,1)

if __name__ == '__main__': unittest.main()
