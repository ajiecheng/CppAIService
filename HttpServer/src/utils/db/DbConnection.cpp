#include "../../../include/utils/db/DbConnection.h"

namespace http {
namespace db {

DbConnection::DbConnection(const std::string& host,
                           const std::string& user,
                           const std::string& password,
                           const std::string& database)
    : host_(host), user_(user), password_(password), database_(database)
{
    sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
    conn_.reset(driver->connect(host, user, password));
    conn_->setSchema(database);
}

DbConnection::~DbConnection()
{
    cleanup();
}

bool DbConnection::isValid()
{
    return conn_ && ping();
}

void DbConnection::reconnect()
{
    cleanup();
    sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
    conn_.reset(driver->connect(host_, user_, password_));
    conn_->setSchema(database_);
}

void DbConnection::cleanup()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (conn_) {
        conn_->close();
        conn_.reset();
    }
}

bool DbConnection::ping()
{
    try {
        std::unique_ptr<sql::Statement> stmt(conn_->createStatement());
        stmt->execute("SELECT 1");
        // Must consume the result set to leave connection clean
        sql::ResultSet* res = stmt->getResultSet();
        while (res->next()) {}
        return true;
    } catch (const sql::SQLException&) {
        return false;
    }
}

} // namespace db
} // namespace http
