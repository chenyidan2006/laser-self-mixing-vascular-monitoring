@echo off
chcp 65001 >nul
title Health Monitor - Stop Server

echo.
echo ================================
echo Stopping Health Monitor Services
echo ================================
echo.

pm2 --version >nul 2>&1
if %errorLevel% neq 0 (
    echo PM2 not installed!
    pause
    exit /b 1
)

echo Stopping all services...
pm2 stop all

echo Deleting processes...
pm2 delete all

echo.
echo ================================
echo Services Stopped!
echo ================================
echo.
pause