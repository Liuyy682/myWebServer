#pragma once

#include <unordered_map>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include "buffer/buffer.h"
#include "log/log.h"

class http_response {
public:
    http_response();
    ~http_response();

    void init(const std::string &src_dir, std::string &path, bool is_keep_alive = false, int code = -1);
    void make_response(buffer &buff);
    void unmap_file();
    size_t file_len() const { return file_stat.st_size; }
    char* file() const { return mm_file; }

private:
    void add_state_line(buffer &buff);
    void add_headers(buffer &buff);
    void add_content(buffer &buff);
    void error_html();
    std::string get_file_type();

    int code;
    bool is_keep_alive;
    std::string path;
    std::string src_dir;
    char *mm_file;
    struct stat file_stat;

    static const std::unordered_map<std::string, std::string> SUFFIX_TYPE;
    static const std::unordered_map<int, std::string> CODE_STATUS;
    static const std::unordered_map<int, std::string> CODE_PATH;
};