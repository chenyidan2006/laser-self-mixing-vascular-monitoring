// 现役原始波形接收服务；部署位置见项目审计文档。
const express = require('express');
const fs = require('fs');
const path = require('path');
const crypto = require('crypto');
const { spawn } = require('child_process');

const app = express();
const PORT = Number(process.env.RAW_RECEIVER_PORT || 3000);
const PYTHON_BIN = process.env.PYTHON_BIN || 'python';
const ALGORITHM_WORKER = process.env.ALGORITHM_WORKER || path.resolve(
  __dirname, '..', 'prodict_algorithm_infuwuqi', 'project', 'frame_pipeline.py'
);

// 数据保存目录
const SAVE_DIR = process.env.RAW_DATA_DIR
  ? path.resolve(process.env.RAW_DATA_DIR)
  : path.join(__dirname, 'received');
if (!fs.existsSync(SAVE_DIR)) fs.mkdirSync(SAVE_DIR, { recursive: true });
const TRAINING_MANIFEST = path.join(SAVE_DIR, 'training_manifest.ndjson');
const LABELS_CSV = path.join(SAVE_DIR, 'reference_labels.csv');
if (!fs.existsSync(LABELS_CSV)) {
  fs.writeFileSync(LABELS_CSV,
    'file,device_id,reference_time,reference_sbp,reference_dbp,reference_hr,subject_id,posture,notes\r\n');
}

// MATLAB prediction is CPU-heavy; serialize jobs so two uploaded frames never
// start competing predictor processes or overwrite model-runtime resources.
let algorithmQueue = Promise.resolve();
const RECENT_FRAME_TTL_MS = 2 * 60 * 1000;
const recentFrames = new Map();

function acknowledgeUpload(res, details = {}) {
  return res
    .status(200)
    .set('Connection', 'close')
    .json({ errcode: 0, msg: 'received', ...details });
}

function enqueueAlgorithm(filepath, deviceId) {
  const run = () => new Promise((resolve) => {
    if (!fs.existsSync(ALGORITHM_WORKER)) {
      console.error(`  ❌ 算法工作器不存在: ${ALGORITHM_WORKER}`);
      resolve();
      return;
    }
    console.log(`  🧠 加入算法队列: ${path.basename(filepath)}, device=${deviceId}`);
    const child = spawn(PYTHON_BIN, [ALGORITHM_WORKER, filepath, '--device-id', deviceId], {
      windowsHide: true,
      shell: false,
    });
    child.stdout.on('data', (data) => process.stdout.write(`[算法] ${data}`));
    child.stderr.on('data', (data) => process.stderr.write(`[算法错误] ${data}`));
    child.on('error', (error) => {
      console.error(`  ❌ 无法启动算法工作器: ${error.message}`);
      resolve();
    });
    child.on('close', (code) => {
      console.log(code === 0
        ? '  ✅ 数据处理完成并已写库（模型结果或失败兜底值，以 [frame-pipeline] 日志为准）'
        : `  ❌ 算法任务退出，code=${code}`);
      resolve();
    });
  });
  algorithmQueue = algorithmQueue.catch(() => undefined).then(run);
}

function inspectAds8688Frame(buf) {
  const lineEnd = buf.indexOf(0x0A);
  if (lineEnd < 0 || lineEnd > 256) return { ok: false, error: 'missing or oversized header' };
  const header = buf.subarray(0, lineEnd + 1).toString('ascii').trim();
  if (!header.startsWith('#ADS8688')) return { ok: false, error: 'missing #ADS8688 header' };
  const number = (key) => {
    const match = header.match(new RegExp(`${key}=(\\d+)`));
    return match ? Number(match[1]) : NaN;
  };
  const blk = number('blk');
  const nblk = number('nblk');
  const rate = number('rate');
  if (!Number.isSafeInteger(blk) || !Number.isSafeInteger(nblk) || blk < 1 || nblk < 1) {
    return { ok: false, error: 'header lacks valid blk/nblk' };
  }
  const dataBytes = blk * nblk * 2; // ADS8688 frame is u16le.
  const trailer = buf.subarray(lineEnd + 1 + dataBytes).toString('ascii');
  if (!trailer.startsWith('#END') || !trailer.endsWith('\r\n')) {
    return { ok: false, error: `invalid trailer or length (expected payload ${dataBytes} bytes)` };
  }
  return { ok: true, header, rate, blk, nblk, dataBytes, totalBytes: buf.length, trailer: trailer.trim() };
}

// 关键：接收原始二进制流，limit 设大一点
app.use('/api/upload/raw',
  express.raw({ type: 'application/octet-stream', limit: '4mb' })
);

// 接收 ESP32 上传
app.post('/api/upload/raw', (req, res) => {
  const buf = req.body;
  const len = Buffer.isBuffer(buf) ? buf.length : 0;
  const ip = req.headers['x-forwarded-for'] || req.socket.remoteAddress;
  const deviceId = String(req.headers['x-device-id'] || 'ESP32_001');

  console.log(`[${new Date().toISOString()}] 收到来自 ${ip} 的数据: ${len} 字节`);

  if (len === 0) {
    console.warn('  ⚠️ 空数据，忽略');
    return res.status(400).json({ errcode: 1, msg: 'empty body' });
  }

  const frame = inspectAds8688Frame(buf);
  if (!frame.ok) {
    console.warn(`  ⚠️ 非法 ADS8688 帧: ${frame.error}`);
    return res.status(422).json({ errcode: 2, msg: frame.error });
  }
  console.log(`  帧校验通过: rate=${frame.rate}Hz, blk=${frame.blk}, nblk=${frame.nblk}, payload=${frame.dataBytes} 字节`);

  // ESP32 retries the same upload when it does not recognize the HTTP reply.
  // Treat an identical full frame from the same device as the same measurement,
  // while still returning HTTP 200 so the sender can clear its upload slot.
  const frameHash = crypto.createHash('sha256').update(buf).digest('hex');
  const now = Date.now();
  for (const [key, item] of recentFrames) {
    if (now - item.seenAt > RECENT_FRAME_TTL_MS) recentFrames.delete(key);
  }
  const dedupeKey = `${deviceId}:${frameHash}`;
  const previous = recentFrames.get(dedupeKey);
  if (previous) {
    console.log(`  ♻️ 已确认重复上传，不重复计算: ${previous.file}`);
    return acknowledgeUpload(res, {
      duplicate: true,
      file: previous.file,
      bytes: len,
    });
  }

  // 保存到文件，文件名带时间戳
  const ts = new Date().toISOString().replace(/[:.]/g, '-');
  const filename = `raw_${ts}_${len}.bin`;
  const filepath = path.join(SAVE_DIR, filename);
  const receivedAt = new Date().toISOString();
  const metadata = {
    schema_version: 1,
    file: filename,
    received_at: receivedAt,
    device_id: deviceId,
    source_ip: ip,
    bytes: len,
    sha256: frameHash,
    frame: {
      sample_rate_hz: frame.rate,
      block_samples: frame.blk,
      block_count: frame.nblk,
      payload_bytes: frame.dataBytes,
      header: frame.header,
      trailer: frame.trailer
    }
  };

  fs.writeFile(filepath, buf, (err) => {
    if (err) {
      console.error('  ❌ 保存失败:', err.message);
      return res.status(500).json({ errcode: 1, msg: 'save failed' });
    }
    console.log(`  ✅ 已保存: ${filename}`);
    // Keep a machine-readable record alongside every binary frame.  The .bin
    // remains unchanged and is the only waveform source used by the pipeline.
    const metadataFile = `${filename}.json`;
    fs.writeFileSync(path.join(SAVE_DIR, metadataFile), JSON.stringify(metadata, null, 2));
    fs.appendFileSync(TRAINING_MANIFEST, JSON.stringify(metadata) + '\n');
    console.log(`  🧪 训练元数据: ${metadataFile}`);
    // 打印前 16 字节，方便肉眼确认数据
    console.log(`  前16字节: ${buf.slice(0, 16).toString('hex')}`);

    recentFrames.set(dedupeKey, { seenAt: Date.now(), file: filename });
    enqueueAlgorithm(filepath, deviceId);
    acknowledgeUpload(res, { bytes: len, file: filename, algorithmQueued: true });
  });
});

// 健康检查
app.get('/api/upload/raw', (req, res) => {
  res.json({ errcode: 0, msg: 'raw receiver is running', port: PORT });
});

app.listen(PORT, '0.0.0.0', () => {
  console.log(`🚀 Raw receiver 已启动: http://0.0.0.0:${PORT}`);
  console.log(`   数据保存目录: ${SAVE_DIR}`);
  console.log(`   算法工作器: ${ALGORITHM_WORKER}`);
});
