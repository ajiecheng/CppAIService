#include "../../../include/utils/db/DbConnectionPool.h"
#include <muduo/base/Logging.h>

namespace http {
namespace db {

DbConnectionPool::DbConnectionPool()
    : initialized_(false)
{
}

DbConnectionPool::~DbConnectionPool()
{
    if (checkThread_.joinable()) {
        checkThread_.join();
    }
}

void DbConnectionPool::init(const std::string& host,
                            const std::string& user,
                            const std::string& password,
                            const std::string& database,
                            size_t poolSize)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (initialized_) return;

    host_ = host;
    user_ = user;
    password_ = password;
    database_ = database;

    for (size_t i = 0; i < poolSize; ++i) {
        auto conn = createConnection();
        connections_.push(conn);
    }

    initialized_ = true;
}

std::shared_ptr<DbConnection> DbConnectionPool::getConnection()
{
    std::unique_lock<std::mutex> lock(mutex_);

    if (connections_.empty()) {
        // Pool exhausted: create a new connection dynamically
        return createConnection();
    }

    auto conn = connections_.front();
    connections_.pop();

    if (!conn->isValid()) {
        conn->reconnect();
    }

    return conn;
}

void DbConnectionPool::returnConnection(std::shared_ptr<DbConnection> conn)
{
    if (!conn) return;
    std::lock_guard<std::mutex> lock(mutex_);
    connections_.push(std::move(conn));
    cv_.notify_one();
}

std::shared_ptr<DbConnection> DbConnectionPool::createConnection()
{
    return std::make_shared<DbConnection>(host_, user_, password_, database_);
}

void DbConnectionPool::checkConnections()
{
    // placeholder
}

} // namespace db
} // namespace http
