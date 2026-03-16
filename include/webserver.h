#pragma once

#include <sys/socket.h>
#include <sys/epoll.h>
#include <cassert>
#include <netinet/in.h>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <string>
#include "log.h"
#include "threadpool.h"
#include "http_conn.h"
#include "epoller.h"
#include "heap_timer.h"

class webserver {
public:
    webserver(int port, bool open_linger, size_t core_poolsize, int trig_mode);
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

    int port;
    
    int listen_fd;
    int epoll_fd;
    bool open_linger;
    char* src_dir; 
    bool is_close{false};
    int timeout_ms{60000};
    int trig_mode;

    uint32_t conn_event;
    uint32_t listen_event;
    std::unique_ptr<threadpool> pool;
    std::unordered_map<int, http_conn> users;
    std::unique_ptr<epoller> epollers;
    std::unique_ptr<heap_timer> timer;
};