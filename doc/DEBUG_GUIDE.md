# Health Monitor App - Debug问题排查手册

---

## 问题目录

| 序号 | 问题 | 状态 | 日期 |
|------|------|------|------|
| 1 | 前端模块导入错误 (api.js export) | ✅ 已修复 | 2026-05-07 |
| 2 | 设备模拟器上传失败 (404错误) | ✅ 已修复 | 2026-05-07 |
| 3 | 验证码可见性问题 | ✅ 已修复 | 2026-05-07 |
| 4 | 端口冲突 (EADDRINUSE) | ✅ 已修复 | 2026-05-07 |
| 5 | MySQL日期格式错误 | ✅ 已修复 | 2026-05-07 |
| 6 | 设备模拟器默认端口错误 | ✅ 已修复 | 2026-05-07 |
| 7 | API接口404 (data-server缺少接口) | ✅ 已修复 | 2026-05-07 |
| 8 | 模拟器间隔NaN错误 | ✅ 已修复 | 2026-05-07 |
| 9 | simulate_device_raw.js默认端口错误 | ✅ 已修复 | 2026-05-07 |

---

## 问题详情

---

### 问题1：前端模块导入错误

**错误信息**：
```
SyntaxError: The requested module '/src/config/api.js' does not provide an export named 'getDataUrl'
```

**发生时间**：2026-05-07
**影响页面**：index.vue等所有使用API配置的页面

**根本原因**：
- `api.js` 使用了 `module.exports`（CommonJS语法）
- 前端Vue组件使用 `import ... from`（ES6模块语法）
- 两者不兼容，导致导入失败

**解决方案**：
将 `module.exports` 改为 ES6 的 `export`

**修改文件**：`src/config/api.js`

**修改后**：
```javascript
export const API_CONFIG = { ... };
export function getAuthUrl(path) { ... };
export function getDataUrl(path) { ... };
```

---

### 问题2：设备模拟器上传失败 (404错误)

**错误信息**：
```
数据上传失败: Request failed with status code 404
```

**发生时间**：2026-05-07
**影响功能**：设备模拟器无法上传数据到服务器

**根本原因**：
1. `data-server.js` 缺少 `/api/upload/health` 和 `/api/upload/raw` 接口
2. 模拟器发送请求的路径与服务器不匹配

**解决方案**：
在 `data-server.js` 中添加缺失的上传接口

---

### 问题3：验证码可见性问题

**错误信息**：
用户反馈在登录时无法看到验证码发送位置

**发生时间**：2026-05-07

**解决方案**：
增强验证码输出的可见性，使用分隔线和emoji

**输出示例**：
```
========================================
📱 验证码已发送到手机: 13800138000
🔑 验证码: 123456
⏱️  有效期: 5分钟
========================================
```

---

### 问题4：端口冲突 (EADDRINUSE)

**错误信息**：
```
Error: listen EADDRINUSE: address already in use :::3002
```

**发生时间**：2026-05-07

**解决方案**：
1. 查找占用端口的进程
2. 终止该进程后重新启动服务

**排查命令**：
```bash
# Windows - 查找端口占用
netstat -ano | findstr ":3002" | findstr "LISTENING"

# Windows - 终止进程
taskkill /F /PID <进程ID>

# Linux/Mac
lsof -i :3002
kill -9 <进程ID>
```

---

### 问题5：MySQL日期格式错误

**错误信息**：
```
Incorrect datetime value: '2026-05-07T10:53:02.329Z' for column 'measure_time'
```

**发生时间**：2026-05-07

**解决方案**：
添加日期格式转换函数，将ISO格式转换为MySQL格式

```javascript
function formatDateTime(date) {
  const d = new Date(date);
  return `${d.getFullYear()}-${String(d.getMonth() + 1).padStart(2, '0')}-${String(d.getDate()).padStart(2, '0')} ${String(d.getHours()).padStart(2, '0')}:${String(d.getMinutes()).padStart(2, '0')}:${String(d.getSeconds()).padStart(2, '0')}`;
}
```

---

### 问题6：设备模拟器默认端口错误

**错误信息**：
```
服务器地址: http://localhost:3000
```

**发生时间**：2026-05-07

**解决方案**：
修改 `simulate_device.js` 默认端口为3002，并支持命令行参数

**使用方式**：
```bash
# 默认连接3002端口
node simulate_device.js

# 自定义服务器地址
node simulate_device.js --server http://localhost:3002
node simulate_device.js -s http://localhost:3002
```

---

### 问题7：API接口404 (data-server缺少接口)

**错误信息**：
```
数据上传失败: Request failed with status code 404
```

**发生时间**：2026-05-07

**解决方案**：
在 `data-server.js` 中补充所有缺失的接口

**必须包含的接口**：
| 接口 | 方法 | 功能 |
|------|------|------|
| `/api/upload/health` | POST | 上传健康数据 |
| `/api/upload/raw` | POST | 上传原始电压数据 |
| `/api/data/realtime` | GET | 获取实时数据 |
| `/api/device/status` | GET | 获取设备状态 |

---

### 问题8：模拟器间隔NaN错误

**错误信息**：
```
上传间隔: NaN秒
TimeoutNaNWarning: NaN is not a number
```

**发生时间**：2026-05-07

**解决方案**：
修复命令行参数解析逻辑，正确处理interval参数

```javascript
process.argv.forEach((arg, index) => {
  if (arg === '--interval' || arg === '-i') {
    uploadInterval = parseInt(process.argv[index + 1]) * 1000 || 8000;
  }
});
```

---

### 问题9：simulate_device_raw.js默认端口错误

**错误信息**：
```
服务器地址: http://localhost:3000
数据上传失败: Request failed with status code 404
```

**发生时间**：2026-05-07

**根本原因**：
`simulate_device_raw.js` 硬编码端口为3000，未支持命令行参数修改

**解决方案**：
1. 修改默认端口为3002
2. 添加命令行参数支持 `--server/-s` 和 `--interval/-i`
3. 添加详细的使用说明输出

**修改后的使用方式**：
```bash
# 默认使用3002端口
node simulate_device_raw.js

# 自定义服务器地址和间隔
node simulate_device_raw.js --server http://localhost:3002 --interval 10
node simulate_device_raw.js -s http://localhost:3002 -i 10
```

---

## 服务端口分配

| 服务 | 端口 | 说明 |
|------|------|------|
| 认证服务 (auth-server.js) | 3001 | 用户注册、登录、验证码 |
| 数据服务 (data-server.js) | 3002 | 健康数据、设备管理 |
| 设备模拟器 | 客户端 | 通过--server指定服务器 |

---

## 快速诊断流程

### 1. 无法启动data-server

```bash
# 检查端口占用
netstat -ano | findstr ":3002" | findstr "LISTENING"

# 终止占用进程
taskkill /F /PID <进程ID>
```

### 2. 模拟器上传失败

```bash
# 确认data-server正在运行
curl http://localhost:3002/api/health

# 确认使用正确的端口
node simulate_device.js --server http://localhost:3002
node simulate_device_raw.js --server http://localhost:3002
```

### 3. 前端无法连接

```bash
# 检查认证服务
curl http://localhost:3001/api/health

# 检查数据服务
curl http://localhost:3002/api/health

# 清除浏览器缓存并刷新
```

---

## 日志关键词

| 关键词 | 含义 | 处理 |
|--------|------|------|
| `EADDRINUSE` | 端口被占用 | 查找并终止占用进程 |
| `404` | 接口不存在 | 检查data-server是否包含该接口 |
| `ECONNREFUSED` | 连接被拒绝 | 确认服务已启动 |
| `MySQL数据库连接成功` | 数据库正常 | - |
| `数据上传失败` | 上传接口问题 | 确认使用正确的服务器地址 |

---

### 问题10：API配置缺少端点

**错误信息**：
```
getAuthUrl('avatar') // 返回 http://localhost:3001undefined
getAuthUrl('profile') // 返回 http://localhost:3001undefined
```

**发生时间**：2026-05-07
**影响功能**：editProfile无法保存、device绑定失败

**根本原因**：
`src/config/api.js` 中的 `API_CONFIG.auth` 对象缺少以下端点配置：
- `avatar`
- `profile`
- `devices`
- `devicesBind`
- `devicesSearch`

**解决方案**：
在 `api.js` 的 `auth` 配置中添加缺失的端点：

```javascript
export const API_CONFIG = {
  auth: {
    baseUrl: 'http://localhost:3001',
    // ... 原有端点 ...
    profile: '/api/user/profile',
    avatar: '/api/user/avatar',
    devices: '/api/user/devices',
    devicesBind: '/api/user/devices/bind',
    devicesSearch: '/api/user/devices/search'
  },
  // ...
};
```

---

### 问题11：设备绑定流程说明

**问题描述**：
用户不清楚如何绑定设备，以及ESP32设备ID在哪里查看

**发生时间**：2026-05-07

**解答**：

#### 什么是ESP32设备ID？
每个ESP32设备在出厂时有一个唯一标识符，用于APP中识别设备。

#### 当前系统的设备ID
在模拟环境中，预设了以下测试设备ID：

| 设备ID | 设备名称 | 状态 |
|--------|----------|------|
| `ESP32_001` | ESP32 测试设备 | online |
| `ESP32_002` | ESP32 备用设备 | offline |

#### 如何绑定设备？

**方式一：搜索绑定**
1. 进入"Me → Device Management"
2. 在"Search Device"区域输入 `ESP32_001`
3. 点击"Bind"

**方式二：手动绑定**
1. 进入"Me → Device Management"
2. 在"Manual Bind"输入框输入 `ESP32_001`
3. 点击"Bind Device"

#### 真实ESP32场景
在真实场景中：
- 设备ID通常印在设备机身上
- 或通过蓝牙扫描获取
- 或扫描设备上的QR码

详细说明请参考：[TECHNICAL_DOCUMENTATION.md](TECHNICAL_DOCUMENTATION.md)（硬件章节）

---

## 服务端口分配

| 服务 | 端口 | 说明 |
|------|------|------|
| 认证服务 (auth-server.js) | 3001 | 用户注册、登录、验证码、设备绑定API |
| 数据服务 (data-server.js) | 3002 | 健康数据、设备模拟器上传 |
| 设备模拟器 | 客户端 | 通过--server指定服务器地址 |

**最后更新**：2026-05-07
**版本**：v1.2
