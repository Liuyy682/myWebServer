#pragma once

#include <vector>
#include <string>
#include <cstring>
#include <unistd.h>
#include <atomic>
#include <assert.h>
#include <sys/uio.h>

class buffer {
public:
    buffer(int buffer_size = 1024);
    ~buffer() = default;

    void retrieve(size_t len);
    void retrieve_until(const char* end);
    void retrieve_all();

    ssize_t read_fd(int fd, int* save_errno);
    ssize_t write_fd(int fd, int* save_errno);
    void append(const char* str, size_t len);
    void append(const std::string& str);
    void append(const void* data, size_t len);
    void ensure_writable(size_t len);
    
    size_t readable_bytes() const { return write_pos.load() - read_pos.load(); }
    size_t writable_bytes() const { return buffer_.size() - write_pos.load(); }
    size_t prependable_bytes() const { return read_pos.load(); }
    const char* peek() const { return begin_ptr() + read_pos.load(); }
    const char* begin_write() const { return begin_ptr() + write_pos.load(); }

private:
    const char* begin_ptr() const { return &*buffer_.begin(); }
    char* begin_ptr() { return &*buffer_.begin(); }
    void make_space(size_t len);
    

    std::vector<char> buffer_;
    std::atomic<size_t> read_pos{0};
    std::atomic<size_t> write_pos{0};
};