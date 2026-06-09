#!/usr/bin/env bash
set -e

echo "开始在 Ubuntu VM 上安装常用依赖（需要 sudo）..."

sudo apt update && sudo apt upgrade -y
sudo apt install -y build-essential cmake git ninja-build pkg-config curl wget \
    libssl-dev libcurl4-openssl-dev libopencv-dev libmysqlclient-dev \
    libprotobuf-dev protobuf-compiler libboost-all-dev librabbitmq-dev || true

# 尝试安装 MySQL Connector/C++
sudo apt install -y libmysqlcppconn-dev || echo "libmysqlcppconn-dev 未在仓库中找到，请手动安装。"

# 构建并安装 muduo
if [ ! -d "$HOME/muduo" ]; then
    git clone https://github.com/chenshuo/muduo.git $HOME/muduo
fi
cd $HOME/muduo
mkdir -p build && cd build
cmake -G Ninja ..
ninja -j$(nproc)
sudo ninja install || true

# 构建并安装 SimpleAmqpClient
if [ ! -d "/tmp/SimpleAmqpClient" ]; then
    git clone https://github.com/alanxz/SimpleAmqpClient.git /tmp/SimpleAmqpClient
fi
cd /tmp/SimpleAmqpClient
mkdir -p build && cd build
cmake -G Ninja ..
ninja -j$(nproc)
sudo ninja install || true

# 更新库缓存
sudo ldconfig || true

echo "依赖安装完成。接下来在项目根目录执行："
echo "  cmake -S . -B build -G \"Ninja\""
echo "  cmake --build build -j$(nproc)"

echo "如果需要不同配置，请参阅 docs/BUILD_ON_VM.md"

exit 0
