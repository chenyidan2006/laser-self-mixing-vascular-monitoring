# 健康监测系统 - 综合技术文档

---

## 目录

1. [项目概述](#1-项目概述)
2. [App技术文档](#2-app技术文档)
3. [硬件技术文档](#3-硬件技术文档)
4. [App使用指南](#4-app使用指南)
5. [硬件使用指南](#5-硬件使用指南)
6. [服务器部署指南](#6-服务器部署指南)
7. [Debug调试文档](#7-debug调试文档)

---

## 1. 项目概述

### 1.1 项目简介

本项目是一个基于物联网的健康监测系统，主要功能包括：
- 脉搏波数据采集（通过PulseSensor或激光驱动板）
- 心率、血压、血氧等健康指标计算
- 数据上传至云端服务器
- AI健康分析与建议
- 移动端App数据展示

### 1.2 系统架构

```
┌─────────────────────────────────────────────────────────────────────┐
│                        健康监测系统架构图                            │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│   ┌──────────────┐        ┌──────────────┐        ┌──────────────┐ │
│   │   传感器层    │        │   网络层      │        │   应用层      │ │
│   │ PulseSensor  │───────►│   ESP32      │───────►│   后端服务器  │ │
│   │ 激光驱动板    │   UART │   WiFi       │   HTTP │   MySQL      │ │
│   └──────────────┘        └──────────────┘        └──────┬───────┘ │
│                                                           │        │
│                                                           ▼        │
│                                                   ┌──────────────┐ │
│                                                   │   移动端App  │ │
│                                                   │   (uni-app)  │ │
│                                                   └──────────────┘ │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

### 1.3 主要技术栈

| 层级 | 技术 | 版本 |
|------|------|------|
| 前端 | uni-app | 3.0+ |
| 后端 | Node.js + Express | 18.x |
| 数据库 | MySQL | 8.0+ |
| 硬件 | STM32F4 + ESP32 | - |
| AI服务 | DeepSeek/OpenAI | API |

---

## 2. App技术文档

### 2.1 项目结构

```
src/
├── pages/                    # 页面目录
│   ├── index/               # 首页（健康数据展示）
│   ├── device/              # 设备管理页
│   ├── profile/             # 个人中心页
│   └── history/             # 历史数据页
├── components/              # 组件目录
│   ├── HealthCard.vue       # 健康数据卡片
│   ├── AIAdvice.vue         # AI建议组件
│   └── DeviceItem.vue       # 设备列表项
├── utils/                   # 工具函数
│   ├── api.js               # API接口配置
│   ├── request.js           # 请求封装
│   └── auth.js              # 认证工具
├── config/                  # 配置文件
│   └── index.js             # 全局配置
└── App.vue                  # 根组件
```

### 2.2 核心API接口

| 接口 | 方法 | 路径 | 说明 |
|------|------|------|------|
| 用户登录 | POST | `/api/auth/login` | 用户登录 |
| 用户注册 | POST | `/api/auth/register` | 用户注册 |
| 获取健康数据 | GET | `/api/data/health` | 获取用户健康数据 |
| 上传原始数据 | POST | `/api/upload/raw` | 上传原始脉搏波数据 |
| 获取AI建议 | POST | `/api/ai/health-advice` | 获取AI健康建议 |
| 设备绑定 | POST | `/api/user/devices/bind` | 绑定设备 |

### 2.3 数据结构

```json
// 用户信息
{
  "id": 1,
  "username": "user@example.com",
  "gender": "male",
  "birthday": "1990-01-01",
  "height": 175,
  "weight": 70
}

// 健康数据
{
  "id": 1,
  "userId": 1,
  "heart_rate": 72,
  "systolic_bp": 120,
  "diastolic_bp": 80,
  "spo2": 98,
  "measureTime": "2024-01-01 12:00:00"
}

// AI建议
{
  "errcode": 0,
  "data": {
    "advice": "您的心率正常，建议保持规律运动...",
    "analysis": "心率在正常范围60-100次/分钟..."
  }
}
```

### 2.4 关键代码示例

**API配置** (`src/utils/api.js`):

```javascript
const API_CONFIG = {
  auth: {
    login: '/api/auth/login',
    register: '/api/auth/register',
    profile: '/api/user/profile',
    devicesBind: '/api/user/devices/bind'
  },
  data: {
    health: '/api/data/health',
    upload: '/api/upload/raw'
  },
  ai: {
    advice: '/api/ai/health-advice'
  }
}
```

**请求封装** (`src/utils/request.js`):

```javascript
async function request(options) {
  const token = uni.getStorageSync('token')
  const header = {
    'Content-Type': 'application/json',
    ...(token && { Authorization: `Bearer ${token}` })
  }
  
  return new Promise((resolve, reject) => {
    uni.request({
      ...options,
      header,
      success: (res) => {
        if (res.data.errcode === 0) resolve(res.data)
        else reject(res.data)
      },
      fail: reject
    })
  })
}
```

---

## 3. 硬件技术文档

### 3.1 硬件组成

#### 3.1.1 四层主控板（STM32F4）

| 层级 | 功能 | 主要元件 |
|------|------|----------|
| 顶层 | 控制核心 | STM32F407、用户按钮、调试排针 |
| 第二层 | 模数转换 | 16位多通道ADC |
| 第三层 | 数据存储 | SRAM、时钟晶振 |
| 底层 | 电源与地 | 电源管理、地线回流 |

**关键引脚分配**：

| 引脚 | 功能 | 连接对象 |
|------|------|----------|
| PA0 | ADC输入 | PulseSensor SIGNAL |
| PA9 | USART1_TX | ESP32 GPIO5 |
| PA10 | USART1_RX | ESP32 GPIO4 |
| 3.3V | 电源输出 | 传感器、ESP32 |
| GND | 地线 | 所有模块 |

#### 3.1.2 ESP32模块

**功能**：负责WiFi通信，将STM32数据上传至服务器

**引脚配置**：

| 引脚 | 功能 | 连接对象 |
|------|------|----------|
| GPIO5 (RX) | UART接收 | STM32 PA9 |
| GPIO4 (TX) | UART发送 | STM32 PA10 |
| 3.3V | 电源输入 | 主控板3.3V |
| GND | 地线 | 主控板GND |

#### 3.1.3 PulseSensor心率传感器

**特性**：
- 工作电压：3.3V-5V
- 输出类型：模拟信号（0-3.3V）
- 采样率：建议100Hz

**引脚定义**：

| 引脚 | 功能 |
|------|------|
| VCC | 电源输入 |
| GND | 地线 |
| SIGNAL | 模拟信号输出 |

### 3.2 连接示意图

```
                    硬件连接示意图
                    ┌───────────────────┐
                    │   PulseSensor     │
                    │                   │
                    │  VCC ────┐        │
                    │  GND ────┼──┐     │
                    │  SIGNAL ─┼──┼──┐  │
                    └───────────┼──┼──┘  │
                                │  │     │
                    ┌───────────▼──▼─────▼──┐
                    │         四层主控板     │
                    │   ┌────────────────┐  │
                    │   │ STM32F407     │  │
                    │   │                │  │
                    │   │ PA0 ◄─ SIGNAL  │  │
                    │   │ PA9 ──► TX     │  │
                    │   │ PA10 ◄─ RX     │  │
                    │   └────────────────┘  │
                    └───────────┬──┬─────┬──┘
                                │  │     │
                    ┌───────────▼──▼─────▼──┐
                    │        ESP32模块      │
                    │   ┌────────────────┐  │
                    │   │ WiFi上传        │  │
                    │   │ GPIO5 ◄─ TX    │  │
                    │   │ GPIO4 ──► RX    │  │
                    │   └────────────────┘  │
                    └───────────────────────┘
```

### 3.3 电气参数

| 参数 | 数值 | 说明 |
|------|------|------|
| 工作电压 | 3.3V | 所有模块统一供电 |
| UART波特率 | 115200 | STM32与ESP32通信 |
| ADC分辨率 | 16位 | 高精度数据采集 |
| 采样率 | 100Hz | 脉搏波采集频率 |
| 缓冲区大小 | 512KB | ESP32数据缓存 |

---

## 4. App使用指南

### 4.1 安装与启动

**环境要求**：
- Node.js 18+
- HBuilderX 3.0+

**安装步骤**：

```bash
# 克隆项目
git clone <项目地址>

# 安装依赖
npm install

# 启动开发服务器
npm run dev:h5  # H5版本
npm run dev:mp-weixin  # 微信小程序
```

### 4.2 功能说明

#### 4.2.1 首页

- 健康数据卡片展示（心率、血压、血氧）
- AI健康建议展示
- 实时数据更新

#### 4.2.2 设备管理

- 设备搜索与绑定
- 设备状态查看
- 设备解绑

#### 4.2.3 个人中心

- 用户信息展示与编辑
- 历史数据查看
- 系统设置

### 4.3 使用流程

```
用户注册/登录 ──► 绑定设备 ──► 数据采集 ──► 查看健康数据 ──► 获取AI建议
```

### 4.4 注意事项

- 确保手机与设备在同一WiFi网络下
- 首次使用需要绑定设备
- 数据上传需要网络连接

---

## 5. 硬件使用指南

### 5.1 硬件连接

**步骤1：连接PulseSensor**

```
PulseSensor ──────────► 四层主控板
     │                     │
     ├── VCC ────────────► 3.3V
     ├── GND ────────────► GND
     └── SIGNAL ─────────► PA0
```

**步骤2：连接ESP32**

```
ESP32 ─────────────────► 四层主控板
     │                     │
     ├── VCC ────────────► 3.3V
     ├── GND ────────────► GND
     ├── TX (GPIO1) ─────► PA10
     └── RX (GPIO3) ─────► PA9
```

### 5.2 软件烧录

**ESP32代码烧录**：

1. 使用Arduino IDE打开 `esp32_wifi_upload.cpp`
2. 选择正确的开发板型号（推荐：ESP32S3 Dev Module）
3. 确保安装了 ESP32 板支持包
4. 上传代码（无需手动配置WiFi，支持自动配网）

**STM32代码烧录**：

1. 使用STM32CubeIDE打开项目
2. 配置ADC和UART参数
3. 编译并烧录到主控板

### 5.3 WiFi配网流程

设备支持**自动配网功能**，用户无需硬编码WiFi信息：

**首次配网步骤**：

```
1. 给设备上电
   └── 设备检查是否有保存的WiFi配置

2. 如果没有配置或连接失败
   └── 设备自动启动配网热点 "HealthMonitor-Config"

3. 用户操作（手机端）：
   ├── 打开手机WiFi设置
   ├── 连接热点 "HealthMonitor-Config"（密码: 12345678）
   ├── 打开手机浏览器，访问 http://192.168.4.1
   └── 输入家里的WiFi名称和密码

4. 设备验证并保存配置：
   ├── 尝试连接用户提供的WiFi
   ├── 如果成功，保存配置到Flash（断电不丢失）
   └── 自动重启并连接WiFi
```

**配网页面说明**：

| 项目 | 说明 |
|------|------|
| 配网热点名称 | HealthMonitor-Config |
| 热点密码 | 12345678 |
| 配网地址 | http://192.168.4.1 |
| 配置保存位置 | ESP32 Flash（Preferences） |

**配网流程图**：

```
用户手机 ──(连接热点)──► ESP32配网服务器 ──(保存配置)──► Flash
                              │
                              ▼
                         重启设备 ──(自动连接)──► 用户WiFi
```

### 5.4 测试步骤

**硬件检查**：
```
├── ✅ 所有电源线连接正确
├── ✅ 所有地线共地
├── ✅ 信号线连接到正确引脚
├── ✅ 传感器已正确安装
```

**功能测试**：
```
1. 给系统上电
2. 观察ESP32串口输出（波特率115200）
3. 首次使用：连接配网热点，完成WiFi配置
4. 确认WiFi连接成功（串口显示IP地址）
5. 触摸PulseSensor测试数据采集
6. 查看服务器端数据接收情况
```

**串口输出示例**：

```
================= ESP32启动 =================

✅ 缓冲区初始化成功：524288字节
✅ 串口初始化完成：UART1，波特率115200，RX=5，TX=4
ℹ️ 未找到保存的WiFi配置，进入配网模式

===== 🔧 进入配网模式 =====
📶 配网热点: HealthMonitor-Config
🔑 热点密码: 12345678
🌐 配网地址: http://192.168.4.1
💡 请在手机WiFi设置中连接此热点，然后打开浏览器访问配网地址
🚀 配网服务器已启动
```

**连接成功后输出**：

```
================= ESP32启动 =================

✅ 缓冲区初始化成功：524288字节
✅ 串口初始化完成：UART1，波特率115200，RX=5，TX=4
🔄 尝试连接保存的WiFi: MyHomeWiFi
..........
✅ WiFi连接成功
📶 IP地址: 192.168.1.105
📡 信号强度: -55 dBm

✅ 系统就绪！

----- 📊 数据采集完成 -----
📥 已接收：1024字节（1.00 KB）
📤 开始上传数据：1024字节
✅ 上传成功！响应: {"errcode":0,"msg":"success"}
```

---

## 6. 服务器部署指南

### 6.1 环境要求

| 组件 | 版本 | 说明 |
|------|------|------|
| Node.js | 18.x | 运行环境 |
| MySQL | 8.0+ | 数据库 |
| Redis | 7.0+ | 缓存（可选） |
| Nginx | 1.20+ | 反向代理（可选） |

### 6.2 安装步骤

**步骤1：安装依赖**

```bash
cd health-backend
npm install
```

**步骤2：配置数据库**

```sql
-- 创建数据库
CREATE DATABASE health_monitor CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;

-- 创建用户
CREATE USER 'health_user'@'localhost' IDENTIFIED BY 'your_password';
GRANT ALL PRIVILEGES ON health_monitor.* TO 'health_user'@'localhost';
FLUSH PRIVILEGES;
```

**步骤3：配置环境变量**

创建 `.env` 文件：

```env
# 数据库配置
DB_HOST=localhost
DB_PORT=3306
DB_NAME=health_monitor
DB_USER=health_user
DB_PASSWORD=your_password

# 服务器配置
PORT=3002
HOST=0.0.0.0

# JWT配置
JWT_SECRET=your_jwt_secret_key
JWT_EXPIRES_IN=7d

# AI配置（可选）
AI_PROVIDER=deepseek
AI_API_KEY=your_ai_api_key
```

**步骤4：启动服务**

```bash
# 开发模式
npm run dev

# 生产模式
npm run start

# 使用PM2管理
pm2 start data-server.js --name health-server
```

### 6.3 服务管理

**使用PM2管理进程**：

```bash
# 启动服务
pm2 start data-server.js --name health-server

# 查看状态
pm2 status

# 查看日志
pm2 logs health-server

# 重启服务
pm2 restart health-server

# 停止服务
pm2 stop health-server
```

### 6.4 Nginx配置（可选）

```nginx
server {
  listen 80;
  server_name your-domain.com;

  location / {
    proxy_pass http://localhost:3002;
    proxy_set_header Host $host;
    proxy_set_header X-Real-IP $remote_addr;
    proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
  }
}
```

---

## 7. Debug调试文档

### 7.1 常见问题排查

#### 7.1.1 设备绑定失败

| 现象 | 可能原因 | 解决方案 |
|------|----------|----------|
| 搜索不到设备 | WiFi未连接 | 检查WiFi连接 |
| 绑定超时 | 设备离线 | 检查设备电源 |
| API返回错误 | 设备ID错误 | 检查设备ID格式 |

**调试步骤**：

```bash
# 检查网络连接
ping 192.168.1.100

# 检查API接口
curl http://localhost:3002/api/user/devices/search
```

#### 7.1.2 数据上传失败

| 现象 | 可能原因 | 解决方案 |
|------|----------|----------|
| HTTP 404 | API路径错误 | 检查API配置 |
| HTTP 401 | Token过期 | 重新登录 |
| HTTP 500 | 服务器错误 | 查看服务器日志 |

**调试步骤**：

```bash
# 查看服务器日志
pm2 logs health-server

# 测试上传接口
curl -X POST http://localhost:3002/api/upload/raw \
  -H "Content-Type: application/json" \
  -d '{"deviceId":"test","rawVoltage":[128,130,125]}'
```

#### 7.1.3 硬件通信问题

| 现象 | 可能原因 | 解决方案 |
|------|----------|----------|
| 无数据接收 | 串口波特率不匹配 | 统一波特率为115200 |
| 数据乱码 | 接线错误 | 检查TX/RX交叉连接 |
| 数据丢失 | 缓冲区溢出 | 增大缓冲区或降低波特率 |

**调试步骤**：

```bash
# 查看ESP32串口输出
screen /dev/ttyUSB0 115200

# 检查串口连接
ls /dev/ttyUSB*
```

#### 7.1.4 WiFi连接问题

| 现象 | 可能原因 | 解决方案 |
|------|----------|----------|
| 无法连接 | SSID/密码错误 | 检查配置 |
| 连接断开 | 信号弱 | 靠近路由器 |
| IP异常 | DHCP失败 | 使用静态IP |

**调试步骤**：

```cpp
// 在ESP32代码中添加调试信息
Serial.printf("WiFi状态: %d\n", WiFi.status());
Serial.printf("IP地址: %s\n", WiFi.localIP().toString().c_str());
```

### 7.2 日志管理

**服务器日志**：

```bash
# 查看实时日志
pm2 logs health-server --raw

# 查看历史日志
cat ~/.pm2/logs/health-server-out.log
```

**硬件日志**：

```bash
# ESP32日志
minicom -b 115200 -D /dev/ttyUSB0

# STM32日志（使用J-Link）
JLinkRTTViewer
```

### 7.3 调试工具推荐

| 工具 | 用途 | 说明 |
|------|------|------|
| Postman | API测试 | 测试RESTful接口 |
| Wireshark | 网络抓包 | 分析网络通信 |
| Serial Monitor | 串口调试 | 查看硬件日志 |
| Chrome DevTools | 前端调试 | 调试H5页面 |

---

## 附录：文件清单

| 文件/目录 | 说明 |
|-----------|------|
| `src/` | App前端代码 |
| `health-backend/` | 后端服务代码 |
| `esp32_wifi_upload.cpp` | ESP32 WiFi上传代码 |
| `PULSESENSOR_CONNECTION_GUIDE.md` | PulseSensor连接指南 |
| `AI_INTEGRATION_GUIDE.md` | AI集成指南 |
| `WIFI_CONNECTION_GUIDE.md` | WiFi连接指南 |
| `DEBUG_GUIDE.md` | 调试指南 |

---

**文档版本**：v1.0  
**创建日期**：2026-05-09  
**作者**：健康监测系统开发团队