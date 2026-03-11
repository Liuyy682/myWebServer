// 简单的 buffer 模块测试代码
#include "../include/buffer.h"
#include <iostream>

void test_buffer() {
    buffer buff;

    const char* test_data = "Hello buffer!";
    buff.append(test_data, strlen(test_data));
    
    if (buff.readable_bytes() == strlen(test_data)) {
        std::cout << "Buffer append and size check successful." << std::endl;
    } else {
        std::cout << "Buffer append or size check failed." << std::endl;
    }

    // 继续其他功能的测试
}

int main() {
    test_buffer();
    return 0;
}