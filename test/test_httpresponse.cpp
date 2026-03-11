// 简单的 httpresponse 模块测试代码
#include "../include/httpresponse.h"
#include <iostream>

void test_httpresponse() {
    buffer buff;
    http_response response;

    // 使用虚拟路径以确保环境独立
    std::string path = "/";
    std::string src_dir = "/"; 

    response.init(src_dir, path, true, 200);

    response.make_response(buff);
    
    if (buff.readable_bytes() > 0) {
        std::cout << "HTTP response generated successfully." << std::endl;
    } else {
        std::cout << "Failed to generate HTTP response." << std::endl;
    }

    // 继续其他功能的测试
}

int main() {
    test_httpresponse();
    return 0;
}