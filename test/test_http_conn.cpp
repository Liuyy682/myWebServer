#include "../include/http_conn.h"
#include <iostream>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>

void test_http_conn() {
    int sock_fd = open("/dev/null", O_RDWR);
    if (sock_fd == -1) {
        std::cerr << "Failed to open /dev/null." << std::endl;
        return;
    }

    http_conn conn;
    sockaddr_in addr{};
    std::cout << "Initializing connection..." << std::endl; 
    conn.init(sock_fd, addr);

    std::cout << "Connection initialized, FD: " << conn.get_fd() << std::endl;

    if (conn.get_fd() == sock_fd) {
        std::cout << "Socket FD initialization successful." << std::endl;
    } else {
        std::cout << "Socket FD initialization failed." << std::endl;
    }

    close(sock_fd);
    std::cout << "Socket closed." << std::endl;
}

int main() {
    std::cout << "Testing http_conn..." << std::endl;
    test_http_conn();
    std::cout << "Test completed." << std::endl;
    return 0;
}