cmake_minimum_required(VERSION 3.10)
project(http_server)

# 配置选项：允许在 Windows 上禁用 Unix-only 组件以便进行部分开发/编译
option(BUILD_WITH_MUDUO "Build with muduo-based server components" ON)
option(BUILD_WITH_MYSQL "Enable MySQL Connector/C++ (mysqlcppconn)" ON)
option(BUILD_WITH_ONNX "Enable ONNX Runtime integration" ON)
option(BUILD_WITH_AMQP "Enable SimpleAmqpClient / RabbitMQ integration" ON)

# 在 Windows 上默认关闭这些 Unix-only 组件（用户可通过 -D 选项开启并提供依赖路径）
if(WIN32)
    set(BUILD_WITH_MUDUO OFF CACHE BOOL "Build with muduo-based server components" FORCE)
    set(BUILD_WITH_MYSQL OFF CACHE BOOL "Enable MySQL Connector/C++ (mysqlcppconn)" FORCE)
    set(BUILD_WITH_ONNX OFF CACHE BOOL "Enable ONNX Runtime integration" FORCE)
    set(BUILD_WITH_AMQP OFF CACHE BOOL "Enable SimpleAmqpClient / RabbitMQ integration" FORCE)
    message(STATUS "Detected Windows: defaulting BUILD_WITH_* options to OFF. Use -DBUILD_WITH_MUDUO=ON etc. if you provide Windows-compatible dependencies.")
endif()

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找 OpenSSL 和 CURL
find_package(OpenSSL REQUIRED)
find_package(CURL REQUIRED)
find_package(OpenCV REQUIRED)

# 添加头文件路径（通用）
include_directories(
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/include
    ${OPENSSL_INCLUDE_DIR}
    ${CURL_INCLUDE_DIRS}
    ${OpenCV_INCLUDE_DIRS}
)

# 如果启用了 muduo（服务器模块），添加其头目录
if(BUILD_WITH_MUDUO)
    include_directories(${PROJECT_SOURCE_DIR}/HttpServer/include ${PROJECT_SOURCE_DIR}/AIApps/ChatServer/include)
endif()

# Unix 专属的系统头/库路径，Windows 上不应使用这些硬编码路径
if(UNIX)
    if(BUILD_WITH_MYSQL)
        include_directories(
            /usr/include/mysql-cppconn-8
            /usr/include/mysql
        )
    endif()

    if(BUILD_WITH_ONNX)
        include_directories(/usr/local/include)
    endif()

    # Unix 的 OpenCV 系统路径（通常 find_package 已处理，但保留以防）
    include_directories(/usr/include/opencv4)
endif()

# 查找必要的库
# 在 Unix 上查找这些库；Windows 环境需要用户提供相应的依赖或在后续步骤配置
if(UNIX)
    if(BUILD_WITH_MYSQL)
        find_library(MYSQLCPPCONN_LIBRARY
            NAMES mysqlcppconn mysqlcppconn8
            PATHS /usr/lib /usr/lib64 /usr/local/lib
        )
    endif()

    if(BUILD_WITH_ONNX)
        find_library(ONNXRUNTIME_LIBRARY
            NAMES onnxruntime
            PATHS /usr/lib /usr/lib64 /usr/local/lib
        )
    endif()
endif()

# 添加源文件
set(SOURCES)

# 仅在启用 muduo（服务器）模块时收集服务器端源码并构建可执行文件
if(BUILD_WITH_MUDUO)
    file(GLOB_RECURSE HTTP_SERVER_SRC "${PROJECT_SOURCE_DIR}/HttpServer/src/*.cpp")
    file(GLOB_RECURSE GOMOKU_SERVER_SRC "${PROJECT_SOURCE_DIR}/AIApps/ChatServer/src/*.cpp")
    list(APPEND SOURCES ${HTTP_SERVER_SRC} ${GOMOKU_SERVER_SRC})

    # 主程序文件
    set(MAIN_SRC "${PROJECT_SOURCE_DIR}/AIApps/ChatServer/src/main.cpp")

    # 添加可执行文件
    add_executable(http_server ${MAIN_SRC} ${SOURCES})
else()
    message(STATUS "BUILD_WITH_MUDUO is OFF: skipping server executable build. You can still build libraries or other components.")
endif()

if(TARGET http_server)
    # 通用库
    target_link_libraries(http_server
        ${CURL_LIBRARIES}
        ${OpenCV_LIBS}
    )

    # 仅在 Unix 系统上链接这些额外依赖（如果启用并存在）
    if(UNIX)
        target_link_libraries(http_server
            pthread
            muduo_net
            muduo_base
            mysqlcppconn
            mysqlclient
            ssl
            crypto
            ${ONNXRUNTIME_LIBRARY}        # 自动查找的 ONNX Runtime 库
            SimpleAmqpClient
            rabbitmq
        )
    endif()
endif()

# 如果库不在默认路径，添加链接目录（仅 Unix）
if(UNIX)
    link_directories(/usr/local/lib)
endif()

# 打印调试信息
message(STATUS "Include directories:")
get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
    message(STATUS "  ${dir}")
endforeach()

message(STATUS "ONNX Runtime library: ${ONNXRUNTIME_LIBRARY}")
message(STATUS "Main source file: ${MAIN_SRC}")
message(STATUS "HTTP Server sources: ${HTTP_SERVER_SRC}")
message(STATUS "Gomoku Server sources: ${GOMOKU_SERVER_SRC}")
