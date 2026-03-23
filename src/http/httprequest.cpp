#include "http/httprequest.h"

 // 初始化默认HTML页面路径集合
const std::unordered_set<std::string> http_request::DEFAULT_HTML {
            "/index", "/register", "/login",
             "/welcome", "/video", "/picture", };

 // 默认HTML标签及其对应的整数值
const std::unordered_map<std::string, int> http_request::DEFAULT_HTML_TAG { 
            {"/register.html", 0}, {"/login.html", 1},  };

 // 初始化HTTP请求对象的状态，包括方法、路径、版本、请求体和头信息
void http_request::init() {
    method = path = version = body = "";
    header.clear();
    state = PARSE_STATE::REQUEST_LINE;
}

 // 判断是否保持连接
bool http_request::is_keep_alive() const {
    auto it = header.find("Connection");
    if (it != header.end()) {
        return it->second == "keep-alive" && version == "1.1";
    }
    return false;
}

 // 解析HTTP请求
bool http_request::parse(buffer& buff) {
    const char CRLF[] = "\r\n";
    if (buff.readable_bytes() <= 0) {
        return false;
    }

    while (buff.readable_bytes() && state != PARSE_STATE::FINISH) {
        if (state == PARSE_STATE::BODY) {
            auto it = header.find("Content-Length");
            size_t content_len = 0;
            if (it != header.end()) {
                content_len = std::strtoul(it->second.c_str(), nullptr, 10);
            }

            if (content_len == 0) {
                state = PARSE_STATE::FINISH;
                break;
            }

            if (buff.readable_bytes() < content_len) {
                return true;
            }

            parse_body(std::string(buff.peek(), buff.peek() + content_len));
            buff.retrieve(content_len);
            break;
        }

        const char* line_start = buff.peek();
        const char* line_end = std::search(line_start, buff.peek() + buff.readable_bytes(), CRLF, CRLF + 2);
        if (line_end == buff.peek() + buff.readable_bytes()) {
            break;
        }

        std::string line(line_start, line_end);
        // 解析不同的请求状态
        switch (state) {
            case PARSE_STATE::REQUEST_LINE:
                if (!parse_request_line(line)) {
                    return false;
                }
                break;
            case PARSE_STATE::HEADERS:
                LOG_DEBUG("Parsing header line: %s", line.c_str());
                parse_headers(line);
                break;
            case PARSE_STATE::BODY:
                LOG_DEBUG("Parsing body line: %s", line.c_str());
                parse_body(line);
                break;
            default:
                break;
        }
        buff.retrieve_until(line_end + 2);
    }

    if (state != PARSE_STATE::FINISH) {
        return true;
    }

    LOG_INFO("Parse HTTP request finished"); // 记录解析完成的日志
    return true;
}

 // 解析请求行
bool http_request::parse_request_line(const std::string& line) {
    std::regex patten("^([^ ]+) ([^ ]+) HTTP/([0-9.]+)$");
    std::smatch sub_match;
    if (std::regex_match(line, sub_match, patten)) {
        method = sub_match[1];
        path = sub_match[2];
        size_t query_pos = path.find('?'); // 检查并剥离路径中的查询参数
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
    LOG_ERROR("Parse request line failed, line='%s'", line.c_str()); // 记录解析失败的错误日志
    return false;
}

 // 解析请求头
void http_request::parse_headers(const std::string& line) {
    std::regex patten("^([^:]*): ?(.*)$"); // 检查并解析头字段
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

 // 解析请求体
void http_request::parse_body(const std::string& line) {
    body = line; // 将请求体内容存储到body中
    parse_post();
    state = PARSE_STATE::FINISH;
}

void http_request::parse_from_urlencoded() {
    // 先对整个 body 进行 URL 解码
    std::string decoded_body = url_decode(body);

    // 再用 regex 分割（或更简单的 string split）
    std::regex pattern("([^&=]*)=([^&]*)");
    auto begin = std::sregex_iterator(decoded_body.begin(), decoded_body.end(), pattern);
    auto end = std::sregex_iterator();

    for (auto it = begin; it != end; ++it) {
        std::string key = (*it)[1].str();
        std::string val = (*it)[2].str();
        post[key] = val;
    }
}

std::string http_request::url_decode(const std::string& str) {
    std::string result;
    for (size_t i = 0; i < str.size(); ++i) {
        if (str[i] == '%') {
            if (i + 2 < str.size()) {
                int high = convert_hex(str[i + 1]);
                int low = convert_hex(str[i + 2]);
                if (high != -1 && low != -1) {
                    result += static_cast<char>((high << 4) | low);
                    i += 2;
                }
            }
        }
        else if (str[i] == '+') {
            result += ' ';
        }
        else {
            result += str[i];
        }
    }
    return result;
}

int http_request::convert_hex(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    else if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }
    else if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    return -1;
}

void http_request::parse_post() {
    if (method == "POST" && header["Content-Type"] == "application/x-www-form-urlencoded") {
        parse_from_urlencoded();
        if(DEFAULT_HTML_TAG.count(path)) {
            int tag = DEFAULT_HTML_TAG.find(path)->second;
            LOG_DEBUG("Tag:%d", tag);
            if(tag == 0 || tag == 1) {
                bool isRegister = (tag == 0);
                if(user_verify(post["username"], post["password"], isRegister)) {
                    path = "/welcome.html";
                } 
                else {
                    path = "/error.html";
                }
            }
        }
    }
}

bool http_request::user_verify(const std::string& name, const std::string& pwd, bool is_register) {
    MYSQL* conn;
    sql_conn_RAII(&conn, sql_conn_pool::get_instance());
    if (conn == nullptr) {
        return false;
    }

    std::string esc_name(name.size() * 2 + 1, '\0');
    std::string esc_pwd(pwd.size() * 2 + 1, '\0');
    unsigned long esc_name_len = mysql_real_escape_string(conn, &esc_name[0], name.c_str(), name.size());
    unsigned long esc_pwd_len = mysql_real_escape_string(conn, &esc_pwd[0], pwd.c_str(), pwd.size());
    esc_name.resize(esc_name_len);
    esc_pwd.resize(esc_pwd_len);

    char select_sql[512] = {0};
    snprintf(select_sql, sizeof(select_sql),
             "SELECT username, password FROM user WHERE username='%s' LIMIT 1", esc_name.c_str());
    if (mysql_query(conn, select_sql)) {
        LOG_ERROR("MySQL query error!");
        return false;
    }

    MYSQL_RES* res = mysql_store_result(conn);
    if (res == nullptr) {
        LOG_ERROR("MySQL store result error!");
        return false;
    }

    MYSQL_ROW row = mysql_fetch_row(res);
    bool user_exists = (row != nullptr);
    bool verify_result = false;

    if (is_register) {
        if (!user_exists) {
            char insert_sql[512] = {0};
            snprintf(insert_sql, sizeof(insert_sql),
                     "INSERT INTO user(username, password) VALUES('%s', '%s')", esc_name.c_str(), esc_pwd.c_str());
            if (mysql_query(conn, insert_sql) == 0) {
                verify_result = true;
            } else {
                LOG_ERROR("MySQL insert error!");
            }
        }
    } else {
        if (user_exists && row[1] != nullptr && pwd == row[1]) {
            verify_result = true;
        }
    }

    mysql_free_result(res);
    LOG_INFO("User verify result: %s", verify_result ? "success" : "failure");
    return verify_result;
}