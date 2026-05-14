@echo off
chcp 65001 >nul

echo [TEST] Checking Node.js version parsing...
for /f "tokens=1" %%a in ('node --version') do set "NODE_VERSION=%%a"
echo Node.js installed: %NODE_VERSION%

echo [TEST] Checking npm version parsing...
for /f "tokens=1" %%a in ('npm --version') do set "NPM_VERSION=%%a"
echo npm installed: v%NPM_VERSION%