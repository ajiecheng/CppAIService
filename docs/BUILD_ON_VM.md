# 在虚拟机（Ubuntu）上构建 CppAIService

本文档适用于在 Linux 虚拟机（推荐 Ubuntu 20.04/22.04）上配置并构建项目的步骤。你已经通过 VS 与 VM 建立了远程连接，接下来在 VM 中执行下列命令。

注意：不同发行版包名或版本可能有所不同，部分库（如 muduo、ONNX Runtime、SimpleAmqpClient）通常需要从源码编译或从发布包安装。

1. 更新和安装基本构建工具

```sh
sudo apt update && sudo apt upgrade -y
sudo apt install -y build-essential cmake git ninja-build pkg-config curl wget \
    libssl-dev libcurl4-openssl-dev libopencv-dev libmysqlclient-dev \
    libprotobuf-dev protobuf-compiler libboost-all-dev
```

2. 安装 MySQL Connector/C++（如果可用）

```sh
# 尝试使用系统包
sudo apt install -y libmysqlcppconn-dev || true
```

如果上面没有可用包，请从 MySQL 官方或源码构建并安装。

3. 安装 / 构建 muduo

muduo 在多数发行版没有预编译包，推荐从源码构建：

```sh
cd /tmp
git clone https://github.com/chenshuo/muduo.git
cd muduo
mkdir -p build && cd build
cmake -G Ninja ..
ninja
sudo ninja install
```

4. 安装 RabbitMQ C 客户端与 SimpleAmqpClient

```sh
# rabbitmq-c
sudo apt install -y librabbitmq-dev librabbitmq4 || true

# SimpleAmqpClient (从源码):
cd /tmp
git clone https://github.com/alanxz/SimpleAmqpClient.git
cd SimpleAmqpClient
mkdir -p build && cd build
cmake -G Ninja ..
ninja
sudo ninja install
```

5. ONNX Runtime

ONNX Runtime 的安装方式较多，可以使用官方发布的 Linux 包或通过源码构建。示例：

- 下载并安装官方 Linux 二进制包（.tar.gz），将 `lib` 目录下的 .so 放到 `/usr/local/lib` 并 `ldconfig`。

6. OpenCV

通常 `libopencv-dev` 会安装必要的头与库。

7. 环境变量（如果某些库安装在非标准路径）

```sh
# 例如：如果把库放在 /usr/local/lib
sudo ldconfig
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
```

8. 在仓库根目录运行 CMake 并构建

```sh
# 在仓库根目录
cmake -S . -B build -G "Ninja"
cmake --build build -j$(nproc)
```

如果需要开启或关闭特性，可以在 cmake 时通过 -D 选项设置：

- 开启 muduo 模块（通常在 Linux 上默认启用）：
  `cmake -S . -B build -G "Ninja" -DBUILD_WITH_MUDUO=ON`
- 关闭某些模块：
  `-DBUILD_WITH_ONNX=OFF -DBUILD_WITH_AMQP=OFF`

9. 常见问题

- 报错找不到 `muduo/...`：确认 `muduo` 已安装并且头文件安装到了系统 include 路径（`/usr/local/include`）。
- 报错找不到 `mysqlcppconn`：确认 `libmysqlcppconn-dev` 已安装或手动安装后库路径已加入 `ldconfig`。



---

如果你愿意，我可以将上述步骤在当前 VM 上自动执行（脚本化），或者仅将脚本放入仓库由你手动运行。