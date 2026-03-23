#pragma once

#include <sys/socket.h>
#include <sys/epoll.h>
#include <cassert>
#include <netinet/in.h>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <string>
#include "log/log.h"
#include "util/threadpool.h"
#include "http/http_conn.h"
#include "epoller/epoller.h"
#include "timer/heap_timer.h"
#include "util/sql_conn_RAII.h"

class webserver {
public:
    webserver(int port,
              bool open_linger,
              size_t core_poolsize,
              size_t max_task_queue,
              int trig_mode,
              int is_log_write,
              const std::string& db_host,
              int db_port,
              const std::string& db_user,
              const std::string& db_password,
              const std::string& db_name,
              int db_conn_size);
    ~webserver();

    void init_socket();
    void start();

private:
    void deal_read(http_conn* client);
    void deal_write(http_conn* client);
    void deal_client();
    void on_read(http_conn* client);
    void on_write(http_conn* client);
    void close_conn(http_conn* client);
    void on_process(http_conn* client);
    int set_nonblock(int fd);
    void extent_time(http_conn* client);
    void init_event_mode(int trig_mode);
    char* resolve_src_dir();
    void init_db_pool(const std::string& db_host,
                      int db_port,
                      const std::string& db_user,
                      const std::string& db_password,
                      const std::string& db_name,
                      int db_conn_size);
    void respond_service_unavailable_and_close(http_conn* client);
    void log_overload_metrics(const char* stage);

    int port;
    
    int listen_fd;
    int epoll_fd;
    bool open_linger;
    char* src_dir; 
    bool is_close{false};
    int timeout_ms{60000};
    int trig_mode;
    int is_log_write;

    uint32_t conn_event;
    uint32_t listen_event;
    std::unique_ptr<threadpool> pool;
    std::unordered_map<int, http_conn> users;
    std::unique_ptr<epoller> epollers;
    std::unique_ptr<heap_timer> timer;
};