@echo off
chcp 65001 >nul
title Health Monitor - Initialize Database

echo.
echo ================================
echo Initialize Database Tables
echo ================================
echo.

echo Initializing database tables...
cd health-backend
node scripts/init_db.js

if %errorLevel% equ 0 (
    echo Database tables initialized successfully
) else (
    echo Failed to initialize database tables
    pause
    exit /b 1
)

echo.
echo ================================
echo Database Initialized!
echo ================================
echo.
pause