// 简单的 epoller 模块测试代码
#include "../include/epoller.h"
#include <iostream>

void test_epoller() {
    epoller ep;
    int fd = 0; // 假设一个有效的文件描述符
    if (ep.add_fd(fd, EPOLLIN)) {
        std::cout << "Added FD successfully." << std::endl;
    } else {
        std::cout << "Failed to add FD." << std::endl;
    }
    // 继续其他测试...
}

int main() {
    test_epoller();
    return 0;
}