#include "httpresponse.h"

const std::unordered_map<std::string, std::string> http_response::SUFFIX_TYPE = {
    { ".html",  "text/html" },
    { ".xml",   "text/xml" },
    { ".xhtml", "application/xhtml+xml" },
    { ".txt",   "text/plain" },
    { ".rtf",   "application/rtf" },
    { ".pdf",   "application/pdf" },
    { ".word",  "application/nsword" },
    { ".png",   "image/png" },
    { ".gif",   "image/gif" },
    { ".jpg",   "image/jpg" },
    { ".jpeg",  "image/jpeg" },
    { ".au",    "audio/basic" },
    { ".mpeg",  "video/mpeg" },
    { ".mpg",   "video/mpeg" },
    { ".avi",   "video/x-msvideo" },
    { ".mp4",   "video/mp4" },
    { ".gz",    "application/x-gzip" },
    { ".tar",   "application/x-tar" },
    { ".css",   "text/css "},
    { ".js",    "text/javascript "},
};

const std::unordered_map<int, std::string> http_response::CODE_STATUS = {
    { 200, "OK" },
    { 400, "Bad Request" },
    { 403, "Forbidden" },
    { 404, "Not Found" }
};

const std::unordered_map<int, std::string> http_response::CODE_PATH = {
    { 400, "/400.html" },
    { 403, "/403.html" },
    { 404, "/404.html" }
};

 // 构造函数，初始化HTTP响应对象的成员变量
http_response::http_response() 
    : code(-1), is_keep_alive(false), path(""), src_dir(""), mm_file(nullptr), file_stat({0}) {}

 // 析构函数，释放映射到内存的文件
http_response::~http_response() {
    unmap_file();
}

 // 初始化HTTP响应对象的状态，包括文件路径、连接保持状态、HTTP状态码等
void http_response::init(const std::string &src_dir, std::string &path, bool is_keep_alive, int code) {
    if (mm_file) {
        unmap_file();
    }
    this->code = code;
    this->is_keep_alive = is_keep_alive;
    this->path = path;
    this->src_dir = src_dir;
    mm_file = nullptr;
    file_stat = {0};
}

 // 生成HTTP响应，根据请求的资源路径生成相应的响应头和内容
void http_response::make_response(buffer &buff) {
    if (path == "/") {
        path = "/index.html";
    }

    if (stat((src_dir + path).c_str(), &file_stat) < 0 || S_ISDIR(file_stat.st_mode)) {
        code = 404;
    }
    else if (!(file_stat.st_mode & S_IROTH)) {
        code = 403;
    }
    else if (code == -1) {
        code = 200;
    }
    error_html();
    add_state_line(buff);
    add_headers(buff);
    add_content(buff);
}

 // 添加状态行到响应缓冲区
void http_response::add_state_line(buffer &buff) {
    std::string status = CODE_STATUS.at(code);
    buff.append("HTTP/1.1 " + std::to_string(code) + " " + status + "\r\n");
}

 // 添加响应头部信息到缓冲区
void http_response::add_headers(buffer &buff) {
    buff.append("Connection: ");
    if (is_keep_alive) {
        buff.append("keep-alive\r\n");
        buff.append("Keep-Alive: max=6, timeout=120\r\n");
    }
    else {
        buff.append("close\r\n");
    }
    buff.append("Content-type: " + get_file_type() + "\r\n");
}

 // 根据请求的文件路径添加内容到缓冲区
void http_response::add_content(buffer &buff) {
    int src_fd = open((src_dir + path).data(), O_RDONLY);
    if (src_fd < 0) {
        LOG_ERROR("Open file failed.");
        error_html();
        return;
    }

    mm_file = static_cast<char*>(mmap(0, file_stat.st_size, PROT_READ, MAP_PRIVATE, src_fd, 0));
    close(src_fd);
    if (mm_file == MAP_FAILED) {
        LOG_ERROR("MMap file failed.");
        error_html();
        return;
    }
    buff.append("Content-length: " + std::to_string(file_stat.st_size) + "\r\n\r\n");
}

 // 根据错误状态码，设置相应的错误页面路径
void http_response::error_html() {
    if (CODE_PATH.count(code) == 0) {
        return;
    }
    path = CODE_PATH.at(code);
    stat((src_dir + path).data(), &file_stat);
}

 //解除文件映射
void http_response::unmap_file() {
    if (mm_file) {
        munmap(mm_file, file_stat.st_size);
        mm_file = nullptr;
    }
}

 // 获取请求文件的MIME类型
std::string http_response::get_file_type() {
    size_t idx = path.find_last_of('.');
    if (idx == std::string::npos) {
        return "text/plain";
    }
    std::string suffix = path.substr(idx);
    if (SUFFIX_TYPE.count(suffix) == 0) {
        return "text/plain";
    }
    return SUFFIX_TYPE.at(suffix);
}