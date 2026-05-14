// simulate_device_data.js
// 模拟当前绑定设备的健康数据上传，波动平稳，适合让首页实时连接显示正常。

const http = require('http');
const url = require('url');

const args = process.argv.slice(2);
const options = {};

for (let i = 0; i < args.length; i++) {
  const arg = args[i];
  if (arg.startsWith('--')) {
    const key = arg.slice(2);
    const value = args[i + 1] && !args[i + 1].startsWith('--') ? args[i + 1] : true;
    options[key] = value;
    if (value !== true) i++;
  }
}

if (!options.deviceId) {
  console.error('请提供设备ID，例如：node simulate_device_data.js --deviceId 9c139eAAdf70');
  process.exit(1);
}

const deviceId = options.deviceId;
const intervalSeconds = Number(options.interval || 30);
const durationMinutes = options.duration ? Number(options.duration) : null;
const serverHost = options.host || '192.144.185.64';
const serverPort = Number(options.port || 3002);
const userId = Number(options.userId || 1);

let heartRate = Number(options.heartRate || 72);
let systolicBp = Number(options.systolicBp || 118);
let diastolicBp = Number(options.diastolicBp || 78);
let spo2 = Number(options.spo2 || 97.0);

// 更新设备状态
function updateDeviceStatus(isOnline) {
  return new Promise((resolve, reject) => {
    const postData = JSON.stringify({ deviceId, isOnline });
    const requestOptions = {
      hostname: serverHost,
      port: serverPort,
      path: '/api/device/updateStatus',
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
        'Content-Length': Buffer.byteLength(postData)
      }
    };

    const req = http.request(requestOptions, (res) => {
      let body = '';
      res.on('data', (chunk) => { body += chunk; });
      res.on('end', () => {
        console.log(`[${new Date().toLocaleTimeString()}] 设备状态更新为: ${isOnline ? '在线' : '离线'}`);
        resolve();
      });
    });

    req.on('error', (err) => {
      console.error(`[${new Date().toLocaleTimeString()}] 更新设备状态失败:`, err.message);
      reject(err);
    });

    req.write(postData);
    req.end();
  });
}

function clamp(value, min, max) {
  return Math.max(min, Math.min(max, value));
}

function randomWalk(value, step, min, max) {
  const delta = (Math.random() * 2 - 1) * step;
  return clamp(Math.round((value + delta) * 10) / 10, min, max);
}

function buildHealthData() {
  heartRate = randomWalk(heartRate, 5, 55, 110);
  systolicBp = randomWalk(systolicBp, 6, 90, 160);
  diastolicBp = randomWalk(diastolicBp, 4, 55, 100);
  spo2 = randomWalk(spo2, 1.5, 92.0, 100.0);

  return {
    deviceId,
    userId,
    measureTime: new Date().toISOString(),
    heartRate,
    systolicBp,
    diastolicBp,
    spo2
  };
}

function uploadData() {
  const data = buildHealthData();
  const postData = JSON.stringify(data);

  const requestOptions = {
    hostname: serverHost,
    port: serverPort,
    path: '/api/upload/health',
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
      'Content-Length': Buffer.byteLength(postData)
    }
  };

  const req = http.request(requestOptions, (res) => {
    let body = '';
    res.on('data', (chunk) => { body += chunk; });
    res.on('end', () => {
      try {
        const json = JSON.parse(body);
        if (json.errcode === 0) {
          console.log(`[${new Date().toLocaleTimeString()}] 上传成功 ->`, {
            deviceId: data.deviceId,
            heartRate: data.heartRate,
            bp: `${data.systolicBp}/${data.diastolicBp}`,
            spo2: data.spo2
          });
        } else {
          console.warn(`[${new Date().toLocaleTimeString()}] 上传接口返回失败:`, json.msg || body);
        }
      } catch (err) {
        console.warn(`[${new Date().toLocaleTimeString()}] 响应解析失败:`, body);
      }
    });
  });

  req.on('error', (err) => {
    console.error(`[${new Date().toLocaleTimeString()}] 请求失败:`, err.message);
  });

  req.write(postData);
  req.end();
}

console.log('================ 模拟设备数据上传脚本 ================');
console.log('设备ID:', deviceId);
console.log('目标:', `${serverHost}:${serverPort}`);
console.log('上传间隔:', intervalSeconds, '秒');
console.log('初始数据:', { heartRate, systolicBp, diastolicBp, spo2 });
if (durationMinutes) {
  console.log('自动停止时间:', durationMinutes, '分钟后');
}
console.log('按 Ctrl+C 停止脚本');
console.log('===================================================\n');

// 启动时设置设备为在线状态
updateDeviceStatus(true);

uploadData();
const timer = setInterval(uploadData, intervalSeconds * 1000);

// 优雅退出处理
let isExiting = false;
async function gracefulExit() {
  if (isExiting) return;
  isExiting = true;
  
  console.log('\n正在停止模拟脚本...');
  clearInterval(timer);
  
  // 同步发送离线请求（使用同步方式确保发送成功）
  const http = require('http');
  const postData = JSON.stringify({ deviceId, isOnline: false });
  
  try {
    await new Promise((resolve, reject) => {
      const req = http.request({
        hostname: serverHost,
        port: serverPort,
        path: '/api/device/updateStatus',
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
          'Content-Length': Buffer.byteLength(postData)
        },
        timeout: 3000
      }, (res) => {
        res.on('data', () => {});
        res.on('end', () => {
          console.log(`[${new Date().toLocaleTimeString()}] 设备已设置为离线状态`);
          resolve();
        });
      });
      
      req.on('error', (err) => {
        console.error(`[${new Date().toLocaleTimeString()}] 更新离线状态失败:`, err.message);
        resolve(); // 即使失败也继续退出
      });
      
      req.on('timeout', () => {
        req.destroy();
        console.error(`[${new Date().toLocaleTimeString()}] 更新离线状态超时`);
        resolve();
      });
      
      req.write(postData);
      req.end();
    });
  } catch (err) {
    console.error('退出处理异常:', err.message);
  }
  
  // 等待一下确保请求发送完成
  await new Promise(resolve => setTimeout(resolve, 1000));
  process.exit(0);
}

process.on('SIGINT', gracefulExit);
process.on('SIGTERM', gracefulExit);

if (durationMinutes) {
  setTimeout(() => {
    console.log(`\n已运行 ${durationMinutes} 分钟，模拟上传已停止。`);
    gracefulExit();
  }, durationMinutes * 60 * 1000);
}
