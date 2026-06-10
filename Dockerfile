FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive \
    TZ=Asia/Shanghai

# 基础工具链与系统依赖
RUN apt update && apt install -y --no-install-recommends \
    build-essential cmake ninja-build git pkg-config curl wget ca-certificates \
    libssl-dev libcurl4-openssl-dev libopencv-dev libmysqlclient-dev \
    libmysqlcppconn-dev libprotobuf-dev protobuf-compiler \
    libboost-all-dev librabbitmq-dev \
    && rm -rf /var/lib/apt/lists/*

# ========== muduo ==========
RUN cd /tmp && \
    git clone --depth=1 https://github.com/chenshuo/muduo.git && \
    cd muduo && mkdir build && cd build && \
    cmake -G Ninja .. && ninja && ninja install && \
    rm -rf /tmp/muduo

# ========== SimpleAmqpClient ==========
RUN cd /tmp && \
    git clone --depth=1 https://github.com/alanxz/SimpleAmqpClient.git && \
    cd SimpleAmqpClient && mkdir build && cd build && \
    cmake -G Ninja .. && ninja && ninja install && \
    rm -rf /tmp/SimpleAmqpClient

# ========== ONNX Runtime ==========
RUN cd /tmp && \
    curl -sLO https://github.com/microsoft/onnxruntime/releases/download/v1.18.0/onnxruntime-linux-x64-1.18.0.tgz && \
    tar xzf onnxruntime-linux-x64-1.18.0.tgz && \
    cp onnxruntime-linux-x64-1.18.0/lib/* /usr/local/lib/ && \
    cp -r onnxruntime-linux-x64-1.18.0/include/* /usr/local/include/ && \
    ldconfig && rm -rf /tmp/onnxruntime*

# ========== 编译项目 ==========
COPY . /app
WORKDIR /app

RUN cmake -S . -B build -G Ninja && \
    cmake --build build -j$(nproc)

# 暴露端口
EXPOSE 8080

# 启动服务
CMD ["./build/http_server", "-p", "8080"]
