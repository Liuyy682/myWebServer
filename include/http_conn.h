#pragma once

#include <netinet/in.h>
#include <cassert>
#include <cerrno>
#include "buffer.h"
#include "httprequest.h"
#include "httpresponse.h"
#include "log.h"

class http_conn {
public:
    http_conn();
    ~http_conn();

    bool process();
    void close_conn();
    void init(int sock_fd, const sockaddr_in& addr);
    ssize_t read(int* save_errno);
    ssize_t write(int* save_errno);
    bool is_keep_alive() const { return request.is_keep_alive(); }
    int get_fd() const { return sock_fd; }
    size_t to_write_bytes() const { return iov[0].iov_len + iov[1].iov_len; }

    static bool is_ET;
    static std::atomic<int> user_count;
    static const char* src_dir;

private:
    int sock_fd;
    struct sockaddr_in addr;
    bool is_close{false};
    buffer read_buf;
    buffer write_buf;
    http_request request;
    http_response response;
    struct iovec iov[2];
    int iov_cnt{0};
};