#pragma once
#include <string>
#include <vector>
#include <utility>
#include <iostream>
#include <sstream>
#include <memory>
#include <fstream>

#include "../../../../HttpServer/include/utils/JsonUtil.h"

// 辅助函数：优先从环境变量读取 API Key，失败时回退到配置文件
inline std::string readApiKey(const char* envName) {
    const char* key = std::getenv(envName);
    if (key && strlen(key) > 0) {
        return std::string(key);
    }
    // 回退到家目录下的配置文件（避免密钥入库）
    const char* home = std::getenv("HOME");
    std::string confPath = home ? (std::string(home) + "/.cppaiservice/apikey.conf") : "AIApps/ChatServer/resource/apikey.conf";
    std::ifstream conf(confPath);
    if (conf.is_open()) {
        std::string line;
        if (std::getline(conf, line) && !line.empty()) {
            return line;
        }
    }
    throw std::runtime_error(std::string(envName) + " not found in env or config file!");
}


class AIStrategy {
public:
    virtual ~AIStrategy() = default;

    
    virtual std::string getApiUrl() const = 0;

    // API Key
    virtual std::string getApiKey() const = 0;


    virtual std::string getModel() const = 0;


    virtual json buildRequest(const std::vector<std::pair<std::string, long long>>& messages) const = 0;


    virtual std::string parseResponse(const json& response) const = 0;

    bool isMCPModel = false;

};

class AliyunStrategy : public AIStrategy {

public:
    AliyunStrategy() {
        apiKey_ = readApiKey("DASHSCOPE_API_KEY");
        isMCPModel = false;
    }

    std::string getApiUrl() const override;
    std::string getApiKey() const override;
    std::string getModel() const override;

    json buildRequest(const std::vector<std::pair<std::string, long long>>& messages) const override;
    std::string parseResponse(const json& response) const override;

private:
    std::string apiKey_;
};

class DouBaoStrategy : public AIStrategy {

public:
    DouBaoStrategy() {
        apiKey_ = readApiKey("DOUBAO_API_KEY");
        isMCPModel = false;
    }
    std::string getApiUrl() const override;
    std::string getApiKey() const override;
    std::string getModel() const override;

    json buildRequest(const std::vector<std::pair<std::string, long long>>& messages) const override;
    std::string parseResponse(const json& response) const override;

private:
    std::string apiKey_;
};

class AliyunRAGStrategy : public AIStrategy {

public:
    AliyunRAGStrategy() {
        apiKey_ = readApiKey("DASHSCOPE_API_KEY");
        isMCPModel = false;
    }

    std::string getApiUrl() const override;
    std::string getApiKey() const override;
    std::string getModel() const override;

    json buildRequest(const std::vector<std::pair<std::string, long long>>& messages) const override;
    std::string parseResponse(const json& response) const override;

private:
    std::string apiKey_;
};

class AliyunMcpStrategy : public AIStrategy {

public:
    AliyunMcpStrategy() {
        apiKey_ = readApiKey("DASHSCOPE_API_KEY");
        isMCPModel = true;
    }

    std::string getApiUrl() const override;
    std::string getApiKey() const override;
    std::string getModel() const override;

    json buildRequest(const std::vector<std::pair<std::string, long long>>& messages) const override;
    std::string parseResponse(const json& response) const override;

private:
    std::string apiKey_;
};







