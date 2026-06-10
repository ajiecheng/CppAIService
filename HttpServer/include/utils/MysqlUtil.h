 #pragma once
 #include "db/DbConnectionPool.h"

#include <string>
#include <memory>

namespace http
{

struct QueryResult {
    std::shared_ptr<db::DbConnection> conn;
    sql::ResultSet* res;

    sql::ResultSet* operator->() { return res; }
    bool next() { return res->next(); }
};

class MysqlUtil
{
public:
    static void init(const std::string& host, const std::string& user,
                    const std::string& password, const std::string& database,
                    size_t poolSize = 10)
    {
        http::db::DbConnectionPool::getInstance().init(
            host, user, password, database, poolSize);
    }

    template<typename... Args>
    QueryResult executeQuery(const std::string& sql, Args&&... args)
    {
        auto conn = http::db::DbConnectionPool::getInstance().getConnection();
        sql::ResultSet* res = conn->executeQuery(sql, std::forward<Args>(args)...);
        return QueryResult{conn, res};
    }

    template<typename... Args>
    int executeUpdate(const std::string& sql, Args&&... args)
    {
        auto conn = http::db::DbConnectionPool::getInstance().getConnection();
        return conn->executeUpdate(sql, std::forward<Args>(args)...);
    }
};

} // namespace http
