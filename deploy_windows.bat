@echo off
chcp 65001 >nul
title Health Monitor Deployment

echo.
echo ================================
echo Health Monitor Deployment
echo ================================
echo.

set "PROJECT_DIR=%~dp0"
set "BACKEND_DIR=%PROJECT_DIR%health-backend"

echo [1/6] Checking Node.js...
node --version > "%TEMP%\node_version.tmp" 2>&1
if %errorLevel% equ 0 (
    set /p NODE_VERSION=<"%TEMP%\node_version.tmp"
    echo Node.js installed: %NODE_VERSION%
) else (
    echo ERROR: Node.js not installed!
    echo Download: https://nodejs.org/dist/v18.19.0/node-v18.19.0-x64.msi
    pause
    exit /b 1
)

echo.
echo [2/6] Checking npm...
call npm --version > "%TEMP%\npm_version.tmp" 2>&1
if %errorLevel% equ 0 (
    set /p NPM_VERSION=<"%TEMP%\npm_version.tmp"
    echo npm installed: %NPM_VERSION%
) else (
    echo ERROR: npm not installed!
    pause
    exit /b 1
)

echo.
echo [3/6] Installing dependencies...
if not exist "%BACKEND_DIR%\node_modules" (
    cd /d "%BACKEND_DIR%"
    echo Running: npm install --production
    call npm install --production
    if %errorLevel% equ 0 (
        echo Dependencies installed successfully
    ) else (
        echo ERROR: Failed to install dependencies!
        pause
        exit /b 1
    )
) else (
    echo Dependencies already installed
)

echo.
echo [4/6] Creating config file...
set "ENV_FILE=%BACKEND_DIR%\.env"
if not exist "%ENV_FILE%" (
    (
        echo DB_HOST=localhost
        echo DB_PORT=3306
        echo DB_USER=root
        echo DB_PASSWORD=1234
        echo DB_NAME=health_monitor
        echo PORT=3002
        echo JWT_SECRET=health_monitor_jwt_secret_key_2026
        echo JWT_EXPIRES_IN=7d
    ) > "%ENV_FILE%"
    echo Config file created: %ENV_FILE%
) else (
    echo Config file already exists
)

echo.
echo [5/6] Installing PM2...
call pm2 --version >nul 2>&1
if %errorLevel% neq 0 (
    echo Installing PM2...
    call npm install -g pm2
    if %errorLevel% equ 0 (
        echo PM2 installed successfully
    ) else (
        echo ERROR: Failed to install PM2!
        pause
        exit /b 1
    )
) else (
    call pm2 --version > "%TEMP%\pm2_version.tmp" 2>&1
    set /p PM2_VERSION=<"%TEMP%\pm2_version.tmp"
    echo PM2 installed: %PM2_VERSION%
)

echo.
echo [6/6] Configuring PM2 startup...
call pm2 startup >nul 2>&1
if %errorLevel% equ 0 (
    echo PM2 startup configured
) else (
    echo WARNING: PM2 startup config failed, run manually: pm2 startup
)

echo.
echo ================================
echo Deployment Complete!
echo ================================
echo.
echo Next steps:
echo 1. Ensure MySQL is running
echo 2. Create database: health_monitor
echo 3. Update %BACKEND_DIR%\.env with correct password
echo 4. Run: start_server.bat
echo.
pause