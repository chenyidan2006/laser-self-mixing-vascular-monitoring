@echo off
chcp 65001 >nul
title Health Monitor - Start Server

echo.
echo ================================
echo Starting Health Monitor Services
echo ================================
echo.

set "SCRIPT_DIR=%~dp0"
set "BACKEND_DIR=%SCRIPT_DIR%health-backend"

if exist "%BACKEND_DIR%\auth-server.js" (
    cd "%BACKEND_DIR%"
) else if exist "auth-server.js" (
    set "BACKEND_DIR=%cd%"
) else (
    echo Error: Cannot find health-backend directory!
    echo Looking for: %BACKEND_DIR%
    pause
    exit /b 1
)

pm2 --version >nul 2>&1
if %errorLevel% neq 0 (
    echo PM2 not installed! Run deploy_windows.bat first.
    pause
    exit /b 1
)

echo Stopping existing services...
pm2 stop auth-server >nul 2>&1
pm2 stop data-server >nul 2>&1

echo.
echo Starting Auth Server (port 3001)...
pm2 start auth-server.js --name auth-server

echo.
echo Starting Data Server (port 3002)...
pm2 start data-server.js --name data-server

pm2 save >nul 2>&1

echo.
echo ================================
echo Services Started!
echo ================================
echo.
pm2 status

echo.
echo Service URLs:
echo   Auth:   http://localhost:3001
echo   Data:   http://localhost:3002
echo.
echo Commands:
echo   pm2 logs          - View logs
echo   pm2 stop all      - Stop services
echo   pm2 restart all   - Restart services
echo.
pause