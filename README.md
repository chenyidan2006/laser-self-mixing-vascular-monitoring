# Health Support - 健康监测系统

![Health Support Logo](https://via.placeholder.com/120x120?text=Health+Support)

> 一款基于物联网的健康监测应用，支持实时监测心率、血压、血氧等健康指标，并提供AI健康建议功能。

---

## 📋 目录

- [项目概述](#项目概述)
- [功能特性](#功能特性)
- [技术栈](#技术栈)
- [快速开始](#快速开始)
- [项目结构](#项目结构)
- [API接口](#api接口)
- [部署说明](#部署说明)
- [硬件连接](#硬件连接)
- [算法集成](#算法集成)
- [贡献指南](#贡献指南)
- [许可证](#许可证)

---

## 项目概述

Health Support 是一个完整的健康监测系统，包含：

- **前端应用**：基于 uni-app 开发，支持 H5、微信小程序等多端运行
- **后端服务**：Node.js + Express 架构，采用分层设计（路由、控制器、服务、模型）
- **硬件集成**：支持 PulseSensor 心率传感器和 ESP32 数据上传
- **算法模块**：支持调用外部 Python/Matlab 封装的可执行文件进行健康指标计算

---

## 功能特性

| 模块 | 功能 | 状态 |
|------|------|------|
| **实时监测** | 心率、血压、血氧实时数据展示 | ✅ |
| **波形显示** | 脉搏波实时波形可视化 | ✅ |
| **历史趋势** | 日/周/月数据趋势图表 | ✅ |
| **AI建议** | 基于健康数据的智能建议 | ✅ |
| **用户管理** | 注册、登录、个人资料 | ✅ |
| **设备管理** | 设备绑定、状态监测 | ✅ |
| **Mock模式** | 模拟数据支持，便于开发测试 | ✅ |

---

## 技术栈

### 前端
| 技术 | 版本 | 说明 |
|------|------|------|
| uni-app | 3.x | 跨平台框架 |
| Vue.js | 2.x | 前端框架 |
| uCharts | 2.x | 图表组件 |

### 后端
| 技术 | 版本 | 说明 |
|------|------|------|
| Node.js | ≥18.x | 运行环境 |
| Express | 4.x | Web框架 |
| MySQL | ≥8.0 | 数据库 |
| JWT | - | 认证机制 |

### 硬件
| 组件 | 说明 |
|------|------|
| STM32F4 | 主控板 |
| ESP32 | WiFi通信模块 |
| PulseSensor | 心率传感器 |

---

## 快速开始

### 环境要求

- Node.js ≥ 18.x
- MySQL ≥ 8.0
- HBuilderX ≥ 3.6.x（前端开发）

### 步骤1：安装依赖

```bash
# 进入后端目录
cd health-backend

# 安装依赖
npm install
```

### 步骤2：配置数据库

```bash
# 登录MySQL
mysql -u root -p

# 创建数据库
CREATE DATABASE health_monitor CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
```

### 步骤3：初始化数据库

```bash
cd health-backend
node scripts/init_db.js
```

### 步骤4：启动服务

```bash
# 启动认证服务（端口3001）
node auth-server.js

# 启动数据服务（端口3002）
node app.js
```

### 步骤5：启动前端

使用 HBuilderX 打开项目，运行到浏览器或微信小程序。

---

## 项目结构

```
health-monitor-app/
├── health-backend/              # 后端服务
│   ├── app.js                   # 主入口（数据服务）
│   ├── auth-server.js           # 认证服务入口
│   ├── routes/                  # 路由层
│   │   ├── auth.js              # 认证路由
│   │   ├── data.js              # 数据路由
│   │   └── ai.js                # AI路由
│   ├── controllers/             # 控制器层
│   │   ├── authController.js    # 认证控制器
│   │   ├── dataController.js    # 数据控制器
│   │   └── userController.js    # 用户控制器（Legacy）
│   ├── middleware/              # 中间件层
│   │   └── auth.js              # JWT认证中间件
│   ├── services/                # 服务层
│   │   └── algorithmService.js  # 算法调用服务
│   ├── models/                  # 数据模型
│   │   └── HealthDataModel.js   # 健康数据模型
│   ├── config/                  # 配置文件
│   │   └── db.js                # 数据库连接
│   ├── utils/                   # 工具函数
│   │   └── sms.js               # 短信工具
│   └── scripts/                 # 脚本工具
│       └── simulate_device.js   # 设备模拟器
├── src/                         # 前端源码
│   ├── pages/                   # 页面目录
│   │   ├── index/               # 首页
│   │   ├── health/              # 健康模块
│   │   ├── profile/             # 个人中心
│   │   ├── device/              # 设备管理
│   │   ├── goal-settings/       # 目标设置
│   │   └── reset-password/      # 重置密码
│   ├── config/                  # 配置文件
│   │   └── api.js               # API配置
│   └── utils/                   # 工具函数
├── data/                        # 测试数据
├── prodict_algorithm_infuwuqi/  # 算法模块
└── doc/                         # 项目文档
```

---

## API接口

### 认证服务（端口3001）

| 接口 | 方法 | 说明 |
|------|------|------|
| `/api/user/register` | POST | 用户注册 |
| `/api/user/login` | POST | 密码登录 |
| `/api/user/sendSms` | POST | 发送验证码 |
| `/api/user/loginBySms` | POST | 验证码登录 |
| `/api/user/resetPassword` | POST | 重置密码 |
| `/api/user/guestLogin` | POST | 游客登录 |
| `/api/user/profile` | GET | 获取用户资料 |
| `/api/user/profile` | PUT | 更新用户资料 |
| `/api/user/devices` | GET | 获取设备列表 |
| `/api/user/devices/bind` | POST | 绑定设备 |
| `/api/user/devices/search` | GET | 搜索设备 |
| `/api/user/devices/:deviceId` | DELETE | 解绑设备 |

### 数据服务（端口3002）

| 接口 | 方法 | 说明 |
|------|------|------|
| `/api/data/realtime` | GET | 获取实时数据 |
| `/api/data/mock/realtime` | GET | 获取模拟实时数据 |
| `/api/data/historyTrend` | GET | 获取历史趋势 |
| `/api/data/rangeRecords` | GET | 获取范围记录 |
| `/api/data/device/status` | GET | 获取设备状态 |
| `/api/data/upload/health` | POST | 上传健康数据 |
| `/api/data/upload/raw` | POST | 上传原始电压数据 |
| `/api/data/process/raw` | POST | 处理原始数据（不保存） |
| `/api/data/txt` | GET | 读取txt/CSV文件 |

### AI服务（端口3002）

| 接口 | 方法 | 说明 |
|------|------|------|
| `/api/ai/health-advice` | POST | 获取AI健康建议 |
| `/api/ai/health-advice/mock` | POST | 获取模拟AI建议 |

---

## 部署说明

### Windows部署

```bash
# 一键部署
deploy_windows.bat

# 初始化数据库
init_database.bat

# 启动服务
start_server.bat
```

### Linux部署

```bash
# 安装依赖
npm install

# 使用PM2管理
pm2 start auth-server.js --name auth-server
pm2 start app.js --name data-server

# 设置开机自启
pm2 save
pm2 startup
```

### Nginx配置

```nginx
server {
    listen 80;
    server_name your-domain.com;
    
    location / {
        root /path/to/dist/build/h5;
        index index.html;
        try_files $uri $uri/ /index.html;
    }
    
    location /api/user/ {
        proxy_pass http://localhost:3001/api/user/;
    }
    
    location /api/data/ {
        proxy_pass http://localhost:3002/api/data/;
    }
}
```

---

## 硬件连接

### PulseSensor连接

| PulseSensor | STM32 | 说明 |
|-------------|-------|------|
| VCC | 3.3V | 电源 |
| GND | GND | 地线 |
| SIGNAL | PA0 | 模拟信号输入 |

### ESP32连接

| ESP32 | STM32 | 说明 |
|-------|-------|------|
| GPIO5 (RX) | PA9 | UART接收 |
| GPIO4 (TX) | PA10 | UART发送 |
| 3.3V | 3.3V | 电源 |
| GND | GND | 地线 |

---

## 算法集成

系统支持调用外部算法可执行文件进行健康指标计算。

### 算法配置

算法目录：`prodict_algorithm_infuwuqi/`

**输入格式**（input_signal.txt）：
```
0.35
0.50
0.37
0.36
0.48
0.69
```

**输出格式**（output_result.txt）：
```
74,139,73,98
```
格式：心率,收缩压,舒张压,血氧

### API调用

```bash
curl -X POST http://localhost:3002/api/data/process/raw \
  -H "Content-Type: application/json" \
  -d '{"deviceId":"ESP32_001","rawVoltage":[0.35,0.50,0.37],"sampleRate":100}'
```

---

## 贡献指南

欢迎提交 Issue 和 Pull Request！

### 开发流程

1. Fork 本仓库
2. 创建特性分支：`git checkout -b feature/your-feature`
3. 提交更改：`git commit -m 'Add some feature'`
4. 推送到分支：`git push origin feature/your-feature`
5. 创建 Pull Request

### 代码规范

- 前端：遵循 Vue 官方代码规范
- 后端：使用 ESLint 检查代码风格

---

## 许可证

MIT License

---

## 文档目录

| 文档 | 说明 |
|------|------|
| [doc/USAGE_GUIDE.md](doc/USAGE_GUIDE.md) | 使用手册 |
| [doc/TECHNICAL_DOCUMENTATION.md](doc/TECHNICAL_DOCUMENTATION.md) | 技术文档 |
| [doc/DEBUG_GUIDE.md](doc/DEBUG_GUIDE.md) | 调试指南 |
| [doc/AI_INTEGRATION_GUIDE.md](doc/AI_INTEGRATION_GUIDE.md) | AI集成指南 |
| [doc/PULSESENSOR_CONNECTION_GUIDE.md](doc/PULSESENSOR_CONNECTION_GUIDE.md) | 硬件连接指南 |

---

**版本**：v1.0  
**最后更新**：2026年5月  
**作者**：Health Support 开发团队
