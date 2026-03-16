#include "httprequest.h"

const std::unordered_set<std::string> http_request::DEFAULT_HTML {
            "/index", "/register", "/login",
             "/welcome", "/video", "/picture", };

const std::unordered_map<std::string, int> http_request::DEFAULT_HTML_TAG {
            {"/register.html", 0}, {"/login.html", 1},  };

void http_request::init() {
    method = path = version = body = "";
    header.clear();
    state = PARSE_STATE::REQUEST_LINE;
}

bool http_request::is_keep_alive() const {
    auto it = header.find("Connection");
    if (it != header.end()) {
        return it->second == "keep-alive" && version == "1.1";
    }
    return false;
}

bool http_request::parse(buffer& buff) {
    const char CRLF[] = "\r\n";
    if (buff.readable_bytes() <= 0) {
        return false;
    }

    while (buff.readable_bytes() && state != PARSE_STATE::FINISH) {
        if (state == PARSE_STATE::BODY) {
            size_t content_length = 0;
            auto it = header.find("Content-Length");
            if (it != header.end()) {
                content_length = static_cast<size_t>(std::strtoul(it->second.c_str(), nullptr, 10));
            }

            if (content_length == 0) {
                state = PARSE_STATE::FINISH;
                break;
            }

            if (buff.readable_bytes() < content_length) {
                break;
            }

            body.assign(buff.peek(), buff.peek() + content_length);
            buff.retrieve(content_length);
            state = PARSE_STATE::FINISH;
            break;
        }

        const char* line_start = buff.peek();
        const char* line_end = std::search(line_start, buff.peek() + buff.readable_bytes(), CRLF, CRLF + 2);
        if (line_end == buff.peek() + buff.readable_bytes()) {
            break;
        }

        std::string line(line_start, line_end);
        switch (state) {
            case PARSE_STATE::REQUEST_LINE:
                if (!parse_request_line(line)) {
                    return false;
                }
                break;
            case PARSE_STATE::HEADERS:
                parse_headers(line);
                break;
            case PARSE_STATE::BODY:
                if (!parse_body(line)) {
                    return false;
                }
                break;
            default:
                break;
        }
        buff.retrieve_until(line_end + 2);
    }

    if (state != PARSE_STATE::FINISH) {
        return true;
    }

    LOG_DEBUG("Parse HTTP request finished");
    return true;
}

bool http_request::parse_request_line(const std::string& line) {
    std::regex patten("^([^ ]+) ([^ ]+) HTTP/([0-9.]+)$");
    std::smatch sub_match;
    if (std::regex_match(line, sub_match, patten)) {
        method = sub_match[1];
        path = sub_match[2];
        size_t query_pos = path.find('?');
        if (query_pos != std::string::npos) {
            path = path.substr(0, query_pos);
        }
        if (path == "/") {
            path = "/index.html";
        }
        else if (DEFAULT_HTML.count(path)) {
            path += ".html";
        }
        version = sub_match[3];
        state = PARSE_STATE::HEADERS;
        return true;
    }
    LOG_ERROR("Parse request line failed, line='%s'", line.c_str());
    return false;
}

void http_request::parse_headers(const std::string& line) {
    std::regex patten("^([^:]*): ?(.*)$");
    std::smatch sub_match;
    if (std::regex_match(line, sub_match, patten)) {
        header[sub_match[1]] = sub_match[2];
    }
    else {
        auto it = header.find("Content-Length");
        if (it != header.end() && std::strtoul(it->second.c_str(), nullptr, 10) > 0) {
            state = PARSE_STATE::BODY;
        }
        else {
            state = PARSE_STATE::FINISH;
        }
    }
}

bool http_request::parse_body(const std::string& line) {
    body = line;
    state = PARSE_STATE::FINISH;
    return true;
}