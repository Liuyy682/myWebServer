#pragma once

#include <sys/socket.h>
#include <sys/epoll.h>
#include <cassert>
#include <netinet/in.h>
#include "log.h"
#include "threadpool.h"
#include "http_conn.h"
#include "epoller.h"

class webserver {
public:
    webserver(int port, bool open_linger, size_t core_poolsize);
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

    int port;
    
    int listen_fd;
    int epoll_fd;
    bool open_linger;
    char* src_dir; 
    bool is_close{false};

    uint32_t conn_event;
    std::unique_ptr<threadpool> pool;
    std::unordered_map<int, http_conn> users;
    std::unique_ptr<epoller> epollers;
};