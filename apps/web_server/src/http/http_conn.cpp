#include "http/http_conn.h"

bool http_conn::is_ET;
std::atomic<int> http_conn::user_count;
const char* http_conn::src_dir;

http_conn::http_conn() : sock_fd(-1), addr({0}), is_close(true) {}

http_conn::~http_conn() {
    close_conn();
}

void http_conn::init(int sock_fd, const sockaddr_in& addr) {
    this->sock_fd = sock_fd;
    this->addr = addr;
    is_close = false;
    read_buf.retrieve_all();
    write_buf.retrieve_all();
    request.init();
    user_count.fetch_add(1);
    if (!src_dir) {
        src_dir = "/tmp";
    }
    assert(sock_fd > 0);
    LOG_INFO("Client connected");
}

void http_conn::close_conn() {
    response.unmap_file();
    if (!is_close) {
        is_close = true;
        user_count.fetch_sub(1);
        close(sock_fd);
        sock_fd = -1;
        LOG_INFO("Client disconnected");
    }
}

ssize_t http_conn::read(int* save_errno) {
    ssize_t total_len = 0;
    do {
        ssize_t len = read_buf.read_fd(sock_fd, save_errno);
        if (len > 0) {
            total_len += len;
            continue;
        }
        if (len == 0) {
            return total_len > 0 ? total_len : 0;
        }
        if (*save_errno == EAGAIN || *save_errno == EWOULDBLOCK) {
            return total_len > 0 ? total_len : -1;
        }
        return -1;
    } while (is_ET);
    return total_len;
}

ssize_t http_conn::write(int* save_errno) {
    ssize_t total_len = 0;
    do {
        ssize_t len = writev(sock_fd, iov, iov_cnt);
        if (len > 0) {
            total_len += len;

            if (static_cast<size_t>(len) >= iov[0].iov_len) {
                size_t file_sent = static_cast<size_t>(len) - iov[0].iov_len;
                write_buf.retrieve_all();
                iov[0].iov_base = const_cast<char*>(write_buf.peek());
                iov[0].iov_len = 0;
                if (iov_cnt > 1) {
                    iov[1].iov_base = static_cast<char*>(iov[1].iov_base) + file_sent;
                    iov[1].iov_len -= file_sent;
                }
            } else {
                write_buf.retrieve(static_cast<size_t>(len));
                iov[0].iov_base = const_cast<char*>(write_buf.peek());
                iov[0].iov_len = write_buf.readable_bytes();
            }

            if (to_write_bytes() == 0) {
                break;
            }
            continue;
        }
        if (len == 0) {
            break;
        }
        if (*save_errno == EAGAIN || *save_errno == EWOULDBLOCK) {
            return total_len > 0 ? total_len : -1;
        }
        return -1;
    } while (is_ET);
    return total_len;
}

bool http_conn::process() {
    LOG_DEBUG("Begin processing connection.");

    if (read_buf.readable_bytes() <= 0) {
        LOG_ERROR("No readable bytes.");
        return false;
    }

    if (!request.parse(read_buf)) {
        LOG_ERROR("Failed to parse request.");
        response.init(src_dir, request.get_path(), false, 400);
        request.init();
    }
    else if (!request.is_finish()) {
        LOG_ERROR("Request not complete yet.");
        return false;
    }
    else {
        LOG_INFO("Request parsed successfully.");
        response.init(src_dir, request.get_path(), request.is_keep_alive(), 200);
        request.init();
    }
    
    response.make_response(write_buf);
    LOG_DEBUG("Response made");

    if (write_buf.readable_bytes() > 0) {
        LOG_DEBUG("Prepare to send response.");
        iov[0].iov_base = const_cast<char*>(write_buf.peek());
        iov[0].iov_len = write_buf.readable_bytes();
        iov[1].iov_base = nullptr;
        iov[1].iov_len = 0;
        iov_cnt = 1;
    } else {
        LOG_ERROR("Write buffer is empty.");
        return false;
    }

    LOG_INFO("Response buffer prepared.");

    if (response.file_len() > 0 && response.file()) {
        LOG_DEBUG("File to send.");
        iov[1].iov_base = response.file();
        iov[1].iov_len = response.file_len();
        iov_cnt = 2;
    }
    
    LOG_INFO("Process completed.");
    return true;
}