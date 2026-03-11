#include "httprequest.h"

const std::unordered_set<std::string> http_request::DEFAULT_HTML {
            "/index", "/register", "/login",
             "/welcome", "/video", "/picture", };

const std::unordered_map<std::string, int> http_request::DEFAULT_HTML_TAG {
            {"/register.html", 0}, {"/login.html", 1},  };

void http_request::init() {
    method = path = version = body = "";
    header.clear();
    state = REQUEST_LINE;
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
    while (buff.readable_bytes() && state != FINISH) {
        const char* line_end = std::search(buff.peek(), buff.peek() + buff.readable_bytes(), CRLF, CRLF + 2);
        std::string line(buff.peek(), line_end);
        switch (state) {
            case REQUEST_LINE:
                if (!parse_request_line(line)) {
                    return false;
                }
                break;
            case HEADERS:
                parse_headers(line);
                break;
            case BODY:
                if (!parse_body(line)) {
                    return false;
                }
                break;
            default:
                break;
        }
        if (line_end == buff.begin_write()) {
            break;
        }
        buff.retrieve_until(line_end + 2);
    }
    LOG_DEBUG("Parse HTTP request finished");
    return true;
}

bool http_request::parse_request_line(const std::string& line) {
    std::regex patten("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
    std::smatch sub_match;
    if (std::regex_match(line, sub_match, patten)) {
        method = sub_match[1];
        path = sub_match[2];
        version = sub_match[3];
        state = HEADERS;
        return true;
    }
    LOG_ERROR("Parse request line failed");
    return false;
}

void http_request::parse_headers(const std::string& line) {
    std::regex patten("^([^:]*): ?(.*)$");
    std::smatch sub_match;
    if (std::regex_match(line, sub_match, patten)) {
        header[sub_match[1]] = sub_match[2];
    }
    else {
        state = BODY;
    }
}

bool http_request::parse_body(const std::string& line) {
    return false;
}