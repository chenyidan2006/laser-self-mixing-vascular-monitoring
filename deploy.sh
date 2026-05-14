#!/bin/bash

# 健康监测系统 - 服务器部署脚本
# 使用方法: bash deploy.sh

set -e

echo "========================================"
echo "健康监测系统 - 服务器部署"
echo "========================================"
echo ""

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 检查是否为root用户
if [ "$EUID" -ne 0 ]; then 
    echo -e "${RED}请使用root用户运行此脚本${NC}"
    exit 1
fi

# 检测操作系统
if [ -f /etc/os-release ]; then
    . /etc/os-release
    OS=$NAME
    VER=$VERSION_ID
else
    echo -e "${RED}无法检测操作系统${NC}"
    exit 1
fi

echo "检测到操作系统: $OS $VER"
echo ""

# 安装Node.js
echo "[1/8] 安装Node.js..."
if ! command -v node &> /dev/null; then
    curl -fsSL https://deb.nodesource.com/setup_18.x | bash -
    apt-get install -y nodejs
    echo -e "${GREEN}Node.js安装成功${NC}"
else
    echo -e "${GREEN}Node.js已安装: $(node -v)${NC}"
fi

# 安装MySQL
echo "[2/8] 安装MySQL..."
if ! command -v mysql &> /dev/null; then
    apt-get update
    DEBIAN_FRONTEND=noninteractive apt-get install -y mysql-server
    echo -e "${GREEN}MySQL安装成功${NC}"
else
    echo -e "${GREEN}MySQL已安装${NC}"
fi

# 启动MySQL服务
echo "[3/8] 启动MySQL服务..."
systemctl start mysql
systemctl enable mysql
echo -e "${GREEN}MySQL服务已启动${NC}"

# 安装PM2
echo "[4/8] 安装PM2..."
if ! command -v pm2 &> /dev/null; then
    npm install -g pm2
    echo -e "${GREEN}PM2安装成功${NC}"
else
    echo -e "${GREEN}PM2已安装${NC}"
fi

# 安装Nginx(可选)
echo "[5/8] 安装Nginx..."
if ! command -v nginx &> /dev/null; then
    apt-get install -y nginx
    echo -e "${GREEN}Nginx安装成功${NC}"
else
    echo -e "${GREEN}Nginx已安装${NC}"
fi

# 配置防火墙
echo "[6/8] 配置防火墙..."
if command -v ufw &> /dev/null; then
    ufw allow 3000/tcp
    ufw allow 80/tcp
    ufw allow 443/tcp
    echo -e "${GREEN}防火墙配置完成${NC}"
else
    echo -e "${YELLOW}未检测到UFW防火墙${NC}"
fi

# 初始化数据库
echo "[7/8] 初始化数据库..."
read -p "请输入MySQL root密码: " -s MYSQL_PASSWORD
echo ""

# 创建数据库
mysql -u root -p"$MYSQL_PASSWORD" <<EOF
CREATE DATABASE IF NOT EXISTS health_monitor DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
EOF

echo -e "${GREEN}数据库创建成功${NC}"

# 安装后端依赖
echo "[8/8] 安装后端依赖..."
if [ -d "health-backend" ]; then
    cd health-backend
    npm install --production
    echo -e "${GREEN}依赖安装完成${NC}"
    
    # 创建.env文件
    if [ ! -f .env ]; then
        cat > .env <<EOF
DB_HOST=localhost
DB_PORT=3306
DB_USER=root
DB_PASSWORD=$MYSQL_PASSWORD
DB_NAME=health_monitor
PORT=3000

PYTHON_COMMAND=python
PYTHON_SCRIPT_PATH=./python/ppg_processor.py
PYTHON_TIMEOUT=15000
PYTHON_MAX_RETRIES=3
PYTHON_RETRY_DELAY=1000

LOG_LEVEL=info
EOF
        echo -e "${GREEN}配置文件创建成功${NC}"
    fi
    
    cd ..
else
    echo -e "${YELLOW}未找到health-backend目录${NC}"
fi

echo ""
echo "========================================"
echo -e "${GREEN}部署完成!${NC}"
echo "========================================"
echo ""
echo "后续步骤:"
echo "1. 配置前端 config/env.prod.js"
echo "2. 运行数据库初始化: cd health-backend && node scripts/init_db.js"
echo "3. 启动后端: pm2 start health-backend/app.js --name health-backend"
echo "4. 构建前端并部署到Nginx"
echo ""
echo "查看后端日志: pm2 logs health-backend"
echo "查看后端状态: pm2 status"
echo ""
