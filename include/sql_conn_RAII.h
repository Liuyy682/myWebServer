#pragma once

#include "sql_conn_pool.h"

class sql_conn_RAII {
public:
    sql_conn_RAII(MYSQL** conn, sql_conn_pool* conn_pool) {
        *conn = conn_pool->get_conn();
        sql = *conn;
        pool = conn_pool;
    }

    ~sql_conn_RAII() {
        if (pool && sql) {
            pool->release_conn(sql);
        }
    }

private:
    MYSQL* sql;
    sql_conn_pool* pool;
};