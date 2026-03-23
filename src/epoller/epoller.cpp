#include "epoller/epoller.h"

epoller::epoller(int max_events) : epoll_fd(epoll_create(5)), events(max_events) {
    assert(epoll_fd != -1 && events.size() > 0);
}

epoller::~epoller() {
    close(epoll_fd);
}

bool epoller::add_fd(int fd, uint32_t events) {
    if (fd < 0) return false;
    epoll_event ev = {0};
    ev.data.fd = fd;
    ev.events = events;
    return epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev) == 0;
}

bool epoller::mod_fd(int fd, uint32_t events) {
    if (fd < 0) return false;
    epoll_event ev = {0};
    ev.data.fd = fd;
    ev.events = events;
    return epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev) == 0;
}

bool epoller::del_fd(int fd) {
    if (fd < 0) return false;
    epoll_event ev = {0};
    return epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, &ev) == 0;
}

int epoller::get_event_fd(size_t i) const {
    assert(i < events.size() && i >= 0);
    return events[i].data.fd;
}

uint32_t epoller::get_events(size_t i) const {
    assert(i < events.size() && i >= 0);
    return events[i].events;
}

int epoller::wait(int timeout) {
    return epoll_wait(epoll_fd, events.data(), static_cast<int>(events.size()), timeout);
}