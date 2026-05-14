@echo off
chcp 65001 >nul
title Health Monitor - Reset Database

echo.
echo ================================
echo Reset Database
echo ================================
echo.

set "SCRIPT_DIR=%~dp0"
set "ENV_FILE=%SCRIPT_DIR%health-backend\.env"

if not exist "%ENV_FILE%" (
    set "ENV_FILE=%SCRIPT_DIR%.env"
)

if not exist "%ENV_FILE%" (
    set "ENV_FILE=health-backend\.env"
)

if not exist "%ENV_FILE%" (
    set "ENV_FILE=.env"
)

if not exist "%ENV_FILE%" (
    echo Error: .env file not found!
    echo Tried paths:
    echo   1. %SCRIPT_DIR%health-backend\.env
    echo   2. %SCRIPT_DIR%.env
    echo   3. health-backend\.env
    echo   4. .env
    pause
    exit /b 1
)

for /f "tokens=1,2 delims==" %%a in (%ENV_FILE%) do (
    if "%%a"=="DB_HOST" set "DB_HOST=%%b"
    if "%%a"=="DB_PORT" set "DB_PORT=%%b"
    if "%%a"=="DB_USER" set "DB_USER=%%b"
    if "%%a"=="DB_PASSWORD" set "DB_PASSWORD=%%b"
    if "%%a"=="DB_NAME" set "DB_NAME=%%b"
)

echo Using config from .env file:
echo   DB_HOST: %DB_HOST%
echo   DB_USER: %DB_USER%
echo   DB_NAME: %DB_NAME%
echo.

echo WARNING: This will delete existing database and recreate it!
echo.
set "CONFIRM="
set /p "CONFIRM=Type YES to confirm: "

if /i not "%CONFIRM%"=="YES" (
    echo Operation cancelled
    pause
    exit /b 0
)

echo.
echo Dropping database...
mysql -u %DB_USER% -p%DB_PASSWORD% -e "DROP DATABASE IF EXISTS %DB_NAME%;"

if %errorLevel% equ 0 (
    echo Database dropped successfully
) else (
    echo Failed to drop database. Check MySQL connection.
    pause
    exit /b 1
)

echo.
echo Creating database...
mysql -u %DB_USER% -p%DB_PASSWORD% -e "CREATE DATABASE %DB_NAME% CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;"

if %errorLevel% equ 0 (
    echo Database created successfully
) else (
    echo Failed to create database
    pause
    exit /b 1
)

echo.
echo ================================
echo Database Reset Complete!
echo ================================
echo.
echo Database: %DB_NAME%
echo Username: %DB_USER%
echo.
echo Using config from health-backend/.env
pause