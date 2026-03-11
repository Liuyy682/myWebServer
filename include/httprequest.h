#pragma once

#include <unordered_map>
#include <unordered_set>
#include <string>
#include <regex>
#include <errno.h>
#include "buffer.h"
#include "log.h"

class http_request {
public:
    enum PARSE_STATE {
        REQUEST_LINE,
        HEADERS,
        BODY,
        FINISH
    };
    enum HTTP_CODE {
        NO_REQUEST,
        GET_REQUEST,
        BAD_REQUEST,
        NO_RESOURCE,
        FORBIDDEN_REQUEST,
        FILE_REQUEST,
        INTERNAL_ERROR,
        CLOSED_CONNECTION
    };

    http_request() { init(); }
    ~http_request() = default;

    void init();
    bool parse(buffer& buff);
    bool is_keep_alive() const;
    std::string get_path() const { return path; }
    std::string& get_path() { return path; }

private:
    bool parse_request_line(const std::string& line);
    void parse_headers(const std::string& line);
    bool parse_body(const std::string& line);

    PARSE_STATE state{REQUEST_LINE};
    std::string method, path, version, body;
    std::unordered_map<std::string, std::string> header;

    static const std::unordered_set<std::string> DEFAULT_HTML;
    static const std::unordered_map<std::string, int> DEFAULT_HTML_TAG;
};