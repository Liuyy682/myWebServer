#include "sql_conn_pool.h"

sql_conn_pool::~sql_conn_pool() {
    std::unique_lock<std::mutex> lock(sql_mutex);
    while (!conn_queue.empty()) {
        MYSQL* conn = conn_queue.front();
        mysql_close(conn);
        conn_queue.pop();
    }
    mysql_library_end();
}

void sql_conn_pool::init(const char* host, int port, const char* user, const char* password, const char* db_name, int conn_size = 10) {
    for (int i = 0; i < conn_size; ++i) {
        MYSQL* conn = mysql_init(nullptr);
        if (conn == nullptr) {
            LOG_ERROR("MySQL init error!");
            exit(1);
        }
        conn = mysql_real_connect(conn, host, user, password, db_name, port, nullptr, 0);
        if (conn == nullptr) {
            LOG_ERROR("MySQL connect error!");
            exit(1);
        }
        conn_queue.push(conn);
    }
    this->conn_size = conn_size;
    this->is_initialized = true;
}

sql_conn_pool* sql_conn_pool::get_instance() {
    static sql_conn_pool instance;
    return &instance;
}

MYSQL* sql_conn_pool::get_conn() {
    std::unique_lock<std::mutex> lock(sql_mutex);
    if (!is_initialized) {
        LOG_ERROR("MySQL pool is not initialized");
        return nullptr;
    }
    sql_cv.wait(lock, [this] { return !conn_queue.empty(); });
    MYSQL* conn = conn_queue.front();
    conn_queue.pop();
    return conn;
}

void sql_conn_pool::release_conn(MYSQL* conn) {
    std::unique_lock<std::mutex> lock(sql_mutex);
    conn_queue.push(conn);
    sql_cv.notify_one();
}