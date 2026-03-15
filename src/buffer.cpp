#include "buffer.h"

 //用指定大小初始化缓冲区
buffer::buffer(int buffer_size) : buffer_(buffer_size) {
    read_pos.store(0);
    write_pos.store(0);
}

 //前进读取位置 len 字节
void buffer::retrieve(size_t len) {
    assert(len <= readable_bytes());
    read_pos.fetch_add(len);
}

 //清除缓冲区中的所有数据
void buffer::retrieve_all() {
    if (!buffer_.empty()) {
        bzero(begin_ptr(), buffer_.size());
    }
    read_pos.store(0);
    write_pos.store(0);
}

 //检索直到指定终点的数据
void buffer::retrieve_until(const char* end) {
    assert(peek() <= end);
    retrieve(end - peek());
}

 //将数据追加到缓冲区
void buffer::append(const char* str, size_t len) {
    assert(str);
    ensure_writable(len);
    std::copy(str, str + len, begin_ptr() + write_pos.load());
    write_pos.fetch_add(len);
}

void buffer::append(const std::string& str) {
    append(str.data(), str.length());
}

void buffer::append(const void* data, size_t len) {
    assert(data);
    append(static_cast<const char*>(data), len);
}

 //确保缓冲区有足够的空间
void buffer::ensure_writable(size_t len) {
    if (writable_bytes() < len) {
        make_space(len);
    }
    assert(writable_bytes() >= len);
}

 //如有需要，分配更多空间
void buffer::make_space(size_t len) {
    if (writable_bytes() + prependable_bytes() < len) {
        buffer_.resize(write_pos.load() + len + 1);
    }
    else {
        size_t readable = readable_bytes();
        std::copy(begin_ptr() + read_pos.load(), begin_ptr() + write_pos.load(), begin_ptr());
        read_pos.store(0);
        write_pos.store(readable);
        assert(readable == readable_bytes());
    }
}

 //从文件描述符读取数据
ssize_t buffer::read_fd(int fd, int* save_errno) {
    char buff[65536];
    struct iovec vec[2];
    const size_t writable = writable_bytes();
    vec[0].iov_base = begin_ptr() + write_pos.load();
    vec[0].iov_len = writable;
    vec[1].iov_base = buff;
    vec[1].iov_len = sizeof(buff);
    const ssize_t len = readv(fd, vec, 2);
    if (len < 0) {
        *save_errno = errno;
    }
    else if (static_cast<size_t>(len) <= writable) {
        write_pos.fetch_add(len);
    }
    else {
        append(buff, len - writable);
    }
    return len;
}

 //将缓冲区数据写入文件描述符
ssize_t buffer::write_fd(int fd, int* save_errno) {
    size_t read_size = readable_bytes();
    const ssize_t len = write(fd, peek(), read_size);
    if (len < 0) {
        *save_errno = errno;
        return len;
    }
    read_pos.fetch_add(len);
    return len;
}