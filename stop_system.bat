@echo off
echo ================================
echo Stop Health Monitoring System
echo ================================
echo.

echo Stopping Node.js processes...
taskkill /f /im node.exe >nul 2>&1
if %errorlevel% equ 0 (
    echo Node.js processes stopped
) else (
    echo No Node.js processes found
)
echo.

echo System stopped
echo.
pause