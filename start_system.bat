@echo off
echo ================================
echo Health Monitoring System
echo ================================
echo.

echo [1/3] Starting Auth Server (端口3001)...
cd health-backend
start "Auth Server" cmd /k "node auth-server.js"
timeout /t 3 >nul
echo Auth Server started on port 3001
echo.

echo [2/3] Starting Data Server (端口3002)...
start "Data Server" cmd /k "node data-server.js"
timeout /t 3 >nul
echo Data Server started on port 3002
echo.

echo [3/3] Starting Device Simulator...
start "Device Simulator" cmd /k "node scripts/simulate_device.js --server http://localhost:3002"
echo Simulator started (sends data every 8 seconds)
echo.

echo ================================
echo System Ready!
echo ================================
echo.
echo Auth Server:   http://localhost:3001
echo Data Server:   http://localhost:3002
echo.
echo How to use:
echo 1. Open project with HBuilderX
echo 2. Run to browser (H5) or WeChat Mini Program
echo 3. You will see real-time health data
echo.
echo Available APIs on Auth Server (3001):
echo - POST /api/user/register       - 用户注册
echo - POST /api/user/login          - 密码登录
echo - POST /api/user/sendSms        - 发送验证码
echo - POST /api/user/loginBySms     - 验证码登录
echo - POST /api/user/devices/bind   - 绑定设备
echo.
echo Available APIs on Data Server (3002):
echo - GET  /api/data/realtime       - 实时数据
echo - GET  /api/data/historyTrend   - 历史趋势
echo - GET  /api/device/status       - 设备状态
echo - POST /api/upload/raw          - 上传原始数据
echo - GET  /api/data/mock/realtime  - 模拟实时数据
echo.
echo Press any key to close this window...
pause >nul