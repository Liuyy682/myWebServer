// 简单的 httprequest 模块测试代码
#include "../include/httprequest.h"
#include "../include/log.h"
#include <iostream>

void test_httprequest() {
    buffer buff;
    http_request request;
    
    buff.append("GET /index.html HTTP/1.1\r\nHost: example.com\r\n\r\n");
    
    if (request.parse(buff)) {
        std::cout << "HTTP request parsed successfully." << std::endl;
    } else {
        std::cout << "Failed to parse HTTP request." << std::endl;
    }

    // 继续其他功能的测试
}

int main() {
    // 调用日志记录以测试其功能
    LOG_DEBUG("Starting test_httprequest");
    test_httprequest();
    LOG_DEBUG("Finished test_httprequest");
    return 0;
}