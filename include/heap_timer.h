#pragma once

#include <chrono>
#include <functional>
#include <unordered_map>
#include <vector>
#include <assert.h>
#include <algorithm>

struct time_node {
    int id;
    std::chrono::steady_clock::time_point expires;
    std::function<void()> timer_cb;
    bool operator<(const time_node& t) {
        return expires < t.expires;
    }
};

class heap_timer {
public:
    heap_timer() { heap.reserve(64); }
    ~heap_timer() { clear(); }

    void add(int id, int timeout, const std::function<void()>& timer_cb);
    void adjust(int id, int timeout);
    void do_work(int id);
    void tick();
    void clear();
    void pop();
    int get_next_tick();

private:
    void del(size_t i);
    void sift_up(size_t i);
    bool sift_down(size_t index, size_t n);
    void swap_node(size_t i, size_t j);

    std::vector<time_node> heap;
    std::unordered_map<int, size_t> ref;
};