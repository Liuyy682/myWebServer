#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <mysql/mysql.h>
#include <thread>
#include "log.h"

class sql_conn_pool {
public:
    sql_conn_pool() {}
    ~sql_conn_pool();
    
    void init(const char* host, int port, const char* user, const char* password, const char* db_name, int conn_size);
    static sql_conn_pool* get_instance();
    MYSQL* get_conn();
    void release_conn(MYSQL* conn);

private:
    int conn_size;
    sql_conn_pool* instance;
    std::queue<MYSQL*> conn_queue;
    std::mutex sql_mutex;
    std::condition_variable sql_cv;
};