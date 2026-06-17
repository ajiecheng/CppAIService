#include <string>
#include <iostream>
#include <thread>
#include <chrono>
#include <muduo/net/TcpServer.h>
#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>

#include"../include/ChatServer.h"

const std::string RABBITMQ_HOST = "localhost";
const std::string QUEUE_NAME = "sql_queue";
const int THREAD_NUM = 2;

void executeMysql(const std::string sql) {
    http::MysqlUtil mysqlUtil_;
    mysqlUtil_.executeUpdate(sql);
}

// 检查端口是否可用，避免 muduo FATAL 直接 abort 崩溃
static bool checkPortAvailable(int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "[ERROR] 无法创建测试 socket" << std::endl;
        return false;
    }

    int reuse = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(sock);
        return false; // 端口被占用
    }

    close(sock);
    return true; // 端口可用
}


int main(int argc, char* argv[]) {
	LOG_INFO << "pid = " << getpid();
	std::string serverName = "ChatServer";
	int port = 8080;
    // 
    int opt;
    const char* str = "p:";
    while ((opt = getopt(argc, argv, str)) != -1)
    {
        switch (opt)
        {
        case 'p':
        {
            port = atoi(optarg);
            break;
        }
        default:
            break;
        }
    }
    muduo::Logger::setLogLevel(muduo::Logger::WARN);

    // 构造前先检查端口，因为 muduo TcpServer 在构造函数里就会 bind()
    if (!checkPortAvailable(port)) {
        std::cerr << "[ERROR] 端口 " << port << " 已被占用，无法启动服务！" << std::endl;
        std::cerr << "[提示] 1. 使用 -p <端口号> 指定其他端口" << std::endl;
        std::cerr << "[提示] 2. 执行 lsof -i :" << port << " 查看端口占用进程" << std::endl;
        exit(1);
    }
    std::cout << "[INFO] 服务启动在端口: " << port << std::endl;

    ChatServer server(port, serverName);
    server.setThreadNum(4);

    std::this_thread::sleep_for(std::chrono::seconds(2));

    server.initChatMessage();


    RabbitMQThreadPool pool(RABBITMQ_HOST, QUEUE_NAME, THREAD_NUM, executeMysql);
    pool.start();

    server.start();
}
