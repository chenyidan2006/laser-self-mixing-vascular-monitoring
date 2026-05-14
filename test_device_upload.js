// test_device_upload.js - 测试设备数据上传
const http = require('http');

const deviceId = process.argv[2] || '9c139eAAdf70'; // 从命令行参数获取设备ID，默认使用用户提到的ID
const serverHost = '192.144.185.64';
const serverPort = 3002;

// 生成随机健康数据
function generateHealthData() {
  return {
    deviceId,
    measureTime: new Date().toISOString(),
    heartRate: Math.floor(60 + Math.random() * 30),
    systolicBp: Math.floor(90 + Math.random() * 40),
    diastolicBp: Math.floor(60 + Math.random() * 25),
    spo2: Number((95 + Math.random() * 5).toFixed(2)),
    userId: 1
  };
}

// 上传健康数据
function uploadHealthData() {
  const data = generateHealthData();
  const postData = JSON.stringify(data);

  const options = {
    hostname: serverHost,
    port: serverPort,
    path: '/api/upload/health',
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
      'Content-Length': Buffer.byteLength(postData)
    }
  };

  const req = http.request(options, (res) => {
    let body = '';
    res.on('data', (chunk) => {
      body += chunk;
    });
    res.on('end', () => {
      try {
        const response = JSON.parse(body);
        console.log(`✅ 数据上传成功:`, {
          deviceId: data.deviceId,
          heartRate: data.heartRate,
          time: new Date(data.measureTime).toLocaleTimeString()
        });
      } catch (e) {
        console.log('❌ 响应解析失败:', body);
      }
    });
  });

  req.on('error', (e) => {
    console.error('❌ 上传失败:', e.message);
  });

  req.write(postData);
  req.end();
}

// 每30秒上传一次数据
console.log(`🚀 开始模拟设备 ${deviceId} 数据上传...`);
console.log(`📡 目标服务器: ${serverHost}:${serverPort}`);
console.log(`⏰ 上传间隔: 30秒`);

uploadHealthData(); // 立即上传一次
setInterval(uploadHealthData, 30000);