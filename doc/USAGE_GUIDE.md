# Health Monitor App - 完整使用手册

---

## 一、项目概述

本项目是一款基于 uni-app 的健康监测应用，支持实时监测心率、血压、血氧等健康数据，并提供历史趋势分析和AI健康建议功能。

### 主要功能

| 模块 | 功能 | 说明 |
|------|------|------|
| **实时监测** | 心率、血压、血氧实时数据展示 | 支持设备连接状态显示 |
| **历史趋势** | 日/周/月数据趋势图表 | 支持不同指标切换 |
| **健康输入** | 饮水量、咖啡因、睡眠、BMI等输入 | 用户自行录入健康数据 |
| **AI建议** | 基于健康数据的智能建议 | 支持DeepSeek API |
| **用户管理** | 注册、登录、个人资料 | 支持多种登录方式 |

---

## 二、系统架构

```
┌─────────────────────────────────────────────────────────────┐
│                      前端 (uni-app)                        │
│  ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌──────────┐      │
│  │  首页    │ │ 健康输入 │ │ 历史趋势 │ │ 个人中心 │      │
│  └────┬─────┘ └────┬─────┘ └────┬─────┘ └────┬─────┘      │
└───────┼───────────┼───────────┼───────────┼───────────────┘
        │           │           │           │
        ▼           ▼           ▼           ▼
┌─────────────────────────────────────────────────────────────┐
│                     后端服务 (Node.js)                      │
│                                                             │
│  ┌─────────────────────────┐  ┌─────────────────────────┐   │
│  │   认证服务 (auth-server)│  │   数据服务 (data-server)│   │
│  │   端口: 3001           │  │   端口: 3002           │   │
│  │   - 用户注册/登录       │  │   - 健康数据存储        │   │
│  │   - 验证码发送         │  │   - MySQL数据库        │   │
│  │   - JWT认证            │  │   - 设备模拟数据        │   │
│  └─────────────────────────┘  └─────────────────────────┘   │
│                                                             │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
                    ┌─────────────────┐
                    │   MySQL数据库   │
                    │   端口: 3306    │
                    │ health_monitor  │
                    └─────────────────┘
```

---

## 三、环境准备

### 3.1 软件要求

| 软件 | 版本 | 说明 |
|------|------|------|
| Node.js | ≥ 18.x | 后端运行环境 |
| MySQL | ≥ 8.0 | 数据库 |
| HBuilderX | ≥ 3.6.x | 前端开发工具 |
| npm | ≥ 9.x | 包管理器 |

### 3.2 端口占用

| 服务 | 端口 | 用途 |
|------|------|------|
| 认证服务 | 3001 | 用户认证、验证码 |
| 数据服务 | 3002 | 健康数据、设备管理 |
| MySQL | 3306 | 数据库 |
| H5预览 | 5173 | 前端开发预览 |

---

## 四、MySQL数据库部署

### 4.1 安装MySQL

**Windows**:
1. 下载MySQL安装包: https://dev.mysql.com/downloads/installer/
2. 安装时选择"Developer Default"或"Server Only"
3. 设置root密码（建议: 1234）

**Linux (Ubuntu/Debian)**:
```bash
sudo apt update
sudo apt install mysql-server
sudo systemctl start mysql
sudo mysql_secure_installation
```

### 4.2 创建数据库

```bash
# 登录MySQL
mysql -u root -p

# 创建数据库
CREATE DATABASE health_monitor CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;

# 创建用户（可选）
CREATE USER 'health_user'@'localhost' IDENTIFIED BY 'password';
GRANT ALL PRIVILEGES ON health_monitor.* TO 'health_user'@'localhost';
FLUSH PRIVILEGES;
```

### 4.3 初始化表结构

运行数据库初始化脚本：
```bash
cd health-backend
node scripts/init_db.js
```

或手动执行SQL文件：
```bash
mysql -u root -p health_monitor < database/init.sql
```

---

## 五、后端服务部署

### 5.1 安装依赖

```bash
cd health-backend
npm install
```

### 5.2 启动认证服务（端口3001）

**方式一：直接启动**
```bash
cd health-backend
node auth-server.js
```

**预期输出：**
```
🚀 用户认证服务已启动
📍 服务地址: http://localhost:3001
📋 支持的接口:
   - POST /api/user/sendSms (发送验证码)
   - POST /api/user/loginBySms (验证码登录)
   - POST /api/user/login (密码登录)
   - POST /api/user/register (用户注册)
   - POST /api/user/guestLogin (游客登录)
```

### 5.3 启动数据服务（端口3002）

**方式一：直接启动**
```bash
cd health-backend
node data-server.js
```

**预期输出：**
```
✅ MySQL数据库连接成功
🚀 健康数据服务已启动
📍 服务地址: http://localhost:3002
📊 数据库模式: MySQL
📋 支持的接口:
   - GET /api/data/realtime (实时数据)
   - GET /api/data/historyTrend (历史趋势)
   - GET /api/data/rangeRecords (范围记录)
   - GET /api/device/status (设备状态)
```

### 5.4 使用PM2管理（生产环境）

```bash
# 安装PM2
npm install -g pm2

# 启动服务
pm2 start auth-server.js --name auth-server
pm2 start data-server.js --name data-server

# 查看状态
pm2 status

# 查看日志
pm2 logs auth-server
pm2 logs data-server
```

---

## 六、前端部署

### 6.1 开发环境运行

1. 使用HBuilderX打开项目 `d:\code\app\appprogram`
2. 点击工具栏"运行" → "运行到浏览器" → "Chrome"
3. 或右键项目 → "运行" → "运行到H5"

### 6.2 构建H5版本

```bash
# 在HBuilderX中
# 点击"发行" → "网站-H5手机端"
# 或使用命令行
npm run build:h5
```

构建产物位于 `dist/build/h5` 目录。

### 6.3 部署到服务器

**方式一：使用Nginx**

```nginx
server {
    listen 80;
    server_name your-domain.com;
    
    location / {
        root /path/to/dist/build/h5;
        index index.html;
        try_files $uri $uri/ /index.html;
    }
    
    # 代理API请求
    location /api/user/ {
        proxy_pass http://localhost:3001/api/user/;
    }
    
    location /api/data/ {
        proxy_pass http://localhost:3002/api/data/;
    }
    
    location /api/device/ {
        proxy_pass http://localhost:3002/api/device/;
    }
}
```

**方式二：使用Node.js静态服务**

```bash
# 安装serve
npm install -g serve

# 启动静态服务
cd dist/build/h5
serve -s . -l 8080
```

---

## 七、设备模拟器使用

### 7.1 启动设备模拟器

```bash
cd health-backend/scripts
node simulate_device.js
```



### 7.2 原始数据模拟器

```bash
cd health-backend/scripts
node simulate_device_raw.js
```
*由于端口冲突*
自定义端口
node simulate_device.js --server http://localhost:3002
node simulate_device_raw.js --server http://localhost:3002 --interval 10


---

## 八、API接口列表

### 8.1 用户认证（端口3001）

| 接口 | 方法 | 说明 |
|------|------|------|
| `/api/user/register` | POST | 用户注册 |
| `/api/user/login` | POST | 密码登录 |
| `/api/user/sendSms` | POST | 发送验证码 |
| `/api/user/loginBySms` | POST | 验证码登录 |
| `/api/user/guestLogin` | POST | 游客登录 |
| `/api/user/wechatLogin` | POST | 微信登录 |

### 8.2 健康数据（端口3002）

| 接口 | 方法 | 说明 |
|------|------|------|
| `/api/data/realtime` | GET | 获取实时数据 |
| `/api/data/mock/realtime` | GET | 获取模拟实时数据 |
| `/api/data/historyTrend` | GET | 获取历史趋势 |
| `/api/data/mock/historyTrend` | GET | 获取模拟历史趋势 |
| `/api/data/rangeRecords` | GET | 获取范围记录 |
| `/api/device/status` | GET | 获取设备状态 |

### 8.3 接口调用示例

**注册用户**：
```bash
curl -X POST http://localhost:3001/api/user/register \
  -H "Content-Type: application/json" \
  -d '{"phone": "13800138000", "password": "123456"}'
```

**发送验证码**：
```bash
curl -X POST http://localhost:3001/api/user/sendSms \
  -H "Content-Type: application/json" \
  -d '{"phone": "13800138000"}'
```

**获取实时数据**：
```bash
curl "http://localhost:3002/api/data/mock/realtime?deviceId=ESP32_001"
```

**获取历史趋势**：
```bash
curl "http://localhost:3002/api/data/mock/historyTrend?startDate=2026-05-01&endDate=2026-05-07&metric=heart_rate"
```

---

## 九、测试账号

### 9.1 注册新用户

```bash
POST http://localhost:3001/api/user/register
{
  "phone": "13800138000",
  "password": "123456",
  "nickname": "Test User"
}
```

### 9.2 登录测试

**密码登录**：
```bash
POST http://localhost:3001/api/user/login
{
  "phone": "13800138000",
  "password": "123456"
}
```

**验证码登录**：
```bash
# 1. 发送验证码
POST http://localhost:3001/api/user/sendSms
{ "phone": "13800138000" }

# 2. 查看控制台获取验证码
# 3. 使用验证码登录
POST http://localhost:3001/api/user/loginBySms
{ "phone": "13800138000", "smsCode": "123456" }
```

---

## 十、完整启动流程

```bash
# 1. 启动MySQL服务
# Windows: net start MySQL96
# Linux: sudo systemctl start mysql

# 2. 启动认证服务（新终端1）
cd health-backend
node auth-server.js

# 3. 启动数据服务（新终端2）
cd health-backend
node data-server.js

# 4. 启动设备模拟器（可选，新终端3）
cd health-backend/scripts
node simulate_device.js --server http://localhost:3002

# 5. 启动前端（HBuilderX）
# 在HBuilderX中运行到浏览器或微信小程序
```

---

## 十一、Windows服务器一键部署

### 11.1 部署脚本说明

项目提供了完整的Windows服务器部署脚本，简化部署流程：

| 脚本文件 | 功能说明 |
|----------|----------|
| `deploy_windows.bat` | **部署初始化** - 检查依赖、安装PM2、创建配置文件 |
| `start_server.bat` | **启动服务** - 使用PM2启动认证服务和数据服务 |
| `stop_server.bat` | **停止服务** - 停止所有PM2进程 |
| `reset_database.bat` | **重置数据库** - 删除并重建数据库 |
| `init_database.bat` | **初始化表** - 创建数据库表结构 |

### 11.2 部署步骤

**步骤1：安装依赖软件**

```
├── Node.js 18.x (必须)
│   下载: https://nodejs.org/dist/v18.19.0/node-v18.19.0-x64.msi
│
└── MySQL 8.0+ (必须)
    下载: https://dev.mysql.com/downloads/installer/
    建议密码: 1234
```

**步骤2：运行部署脚本**

```bash
# 右键以管理员身份运行
deploy_windows.bat
```

**步骤3：重置数据库（首次部署或需要清空数据）**

```bash
# 双击运行，输入 YES 确认
reset_database.bat
```

**步骤4：初始化数据库表**

```bash
# 双击运行
init_database.bat
```

**步骤5：启动服务**

```bash
# 双击运行
start_server.bat
```

### 11.3 服务管理

```bash
# 查看服务状态
pm2 status

# 查看日志
pm2 logs auth-server
pm2 logs data-server

# 重启服务
pm2 restart auth-server
pm2 restart data-server

# 停止服务
pm2 stop all

# 设置开机自启
pm2 save
pm2 startup
```

### 11.4 脚本功能详解

**deploy_windows.bat**
```
功能：部署初始化
包含：
├── 检查Node.js版本
├── 检查npm版本
├── 安装后端依赖
├── 创建.env配置文件
├── 安装PM2进程管理器
└── 配置PM2开机自启
```

**start_server.bat**
```
功能：启动服务
包含：
├── 停止现有服务（如果运行）
├── 启动认证服务 → 端口3001
├── 启动数据服务 → 端口3002
└── 保存PM2配置
```

**reset_database.bat**
```
功能：重置数据库
包含：
├── 删除现有数据库
├── 创建新数据库
└── 创建用户并授权
```

---

## 十二、常见问题

### Q1: 端口被占用？

```bash
# Windows
netstat -ano | findstr ":3001"
taskkill /PID <PID> /F

# Linux/Mac
lsof -i :3001
kill -9 <PID>
```

### Q2: MySQL连接失败？

检查配置文件 `data-server.js`：
```javascript
const dbConfig = {
  host: 'localhost',
  user: 'root',
  password: '1234',  // 修改为您的密码
  database: 'health_monitor'
};
```

### Q3: 验证码收不到？

验证码会打印在**认证服务**的控制台中：
```
========================================
📱 验证码已发送到手机: 13800138000
🔑 验证码: 123456
⏱️  有效期: 5分钟
========================================
```

### Q4: 前端无法访问API？

检查是否配置了正确的API地址，配置文件：`src/config/api.js`

### Q5: 趋势图数据异常？

确保数据服务已启动，且API返回正确的数据格式。

---

## 十二、技术栈

| 分类 | 技术 | 版本 |
|------|------|------|
| 前端框架 | uni-app | 3.x |
| 语言 | Vue 2 | - |
| 图表 | uCharts | 2.x |
| 后端 | Express | 4.x |
| 数据库 | MySQL | 8.x |
| 认证 | JWT | - |

---

## 十三、目录结构

```
health-monitor-app/
├── health-backend/              # 后端服务
│   ├── auth-server.js          # 认证服务（端口3001）
│   ├── data-server.js          # 数据服务（端口3002）
│   ├── app_mysql.js            # 旧版单文件服务
│   ├── package.json            # 依赖配置
│   ├── .env                    # 环境变量配置
│   ├── config/                 # 配置文件
│   ├── controllers/            # 控制器
│   ├── routes/                 # 路由
│   ├── models/                 # 数据模型
│   ├── scripts/                # 脚本（设备模拟器等）
│   └── database/               # 数据库初始化脚本
├── src/                        # 前端源码
│   ├── pages/                  # 页面
│   │   ├── index/              # 首页
│   │   ├── health/             # 健康模块
│   │   ├── profile/            # 个人中心
│   │   ├── login/              # 登录
│   │   └── register/           # 注册
│   ├── config/                 # 配置
│   │   └── api.js             # API配置
│   ├── utils/                  # 工具函数
│   └── static/                 # 静态资源
├── doc/                        # 文档
│   ├── USAGE_GUIDE.md          # 使用手册（本文件）
│   ├── TECHNICAL_DOCUMENTATION.md  # 技术文档
│   ├── DEBUG_GUIDE.md          # 调试指南
│   ├── AI_INTEGRATION_GUIDE.md # AI集成指南
│   └── DEVELOPMENT_NOTES.md    # 开发笔记
├── deploy_windows.bat          # Windows部署脚本
├── start_server.bat            # 服务启动脚本
├── stop_server.bat             # 服务停止脚本
├── reset_database.bat          # 数据库重置脚本
├── init_database.bat           # 数据库初始化脚本
├── start_system.bat            # 开发环境启动脚本
├── stop_system.bat             # 开发环境停止脚本
└── deploy.sh                   # Linux部署脚本
```

---

## 十四、更新日志

### v1.2.0 (2026-05-09)

- ✅ Windows服务器一键部署脚本
- ✅ 数据库重置脚本 (reset_database.bat)
- ✅ 数据库初始化脚本 (init_database.bat)
- ✅ PM2进程管理集成
- ✅ 开机自启配置
- ✅ 完整的部署文档

### v1.1.0 (2026-05-08)

- ✅ WiFi配网功能（AP模式 + Web配置）
- ✅ ESP32代码优化
- ✅ AI API集成（DeepSeek、OpenAI）
- ✅ 设备绑定功能修复
- ✅ UI优化（移除quick-action-btn和health data）

### v1.0.0 (2026-05-07)

- ✅ 分离认证服务和数据服务
- ✅ 实时健康数据监测
- ✅ 历史趋势图表（日/周/月）
- ✅ 用户注册与登录（验证码/密码）
- ✅ 健康数据输入模块
- ✅ AI健康建议功能
- ✅ 设备模拟器支持
- ✅ MySQL数据库集成

---

## 十五、联系方式

如有问题或建议，请联系开发团队：
- 邮箱: dev@healthmonitor.com
- GitHub: https://github.com/healthmonitor/app
