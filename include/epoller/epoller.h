#pragma once

#include <sys/epoll.h>
#include <vector>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>

class epoller {
public:
    explicit epoller(int max_events = 10000);
    ~epoller();

    bool add_fd(int fd, uint32_t events);
    bool mod_fd(int fd, uint32_t events);
    bool del_fd(int fd);

    int get_event_fd(size_t i) const;
    uint32_t get_events(size_t i) const;
    int wait(int timeout = -1);
    
private:
    int epoll_fd;
    std::vector<struct epoll_event> events;
};