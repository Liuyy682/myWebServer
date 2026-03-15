#include "heap_timer.h"

void heap_timer::sift_up(size_t i) {
    assert(i < heap.size());
    size_t j = (i - 1) / 2;
    while (j >= 0) {
        if (heap[j] < heap[i]) break;
        swap_node(i, j);
        i = j;
        j = (i - 1) / 2;
    }
}

bool heap_timer::sift_down(size_t index, size_t n) {
    assert(index < heap.size());
    size_t i = index;
    size_t j = index * 2 + 1;
    while (j < n) {
        if (j + 1 < n && heap[j + 1] < heap[j]) j++;
        if (heap[index] < heap[j]) break;
        swap_node(i, j);
        i = j;
        j = i * 2 + 1;
    }
    return i > index;
}

void heap_timer::swap_node(size_t i, size_t j) {
    assert(i < heap.size() && j < heap.size());
    std::swap(heap[i], heap[j]);
    ref[heap[i].id] = i;
    ref[heap[j].id] = j;
}

void heap_timer::del(size_t i) {
    assert(i < heap.size());
    size_t n = heap.size() - 1;
    if (i < n) {
        swap_node(i, n);
        if (!sift_down(i, n)) sift_up(i);
    }
    ref.erase(heap.back().id);
    heap.pop_back();
}

void heap_timer::add(int id, int timeout, const std::function<void()>& timer_cb) {
    assert(id >= 0);
    size_t i;
    if (ref.count(id) == 0) {
        i = heap.size();
        heap.push_back({id, std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout), timer_cb});
        ref[id] = i;
        sift_up(i);
    }
    else {
        i = ref[id];
        heap[i].expires = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout);
        heap[i].timer_cb = timer_cb;
        if (!sift_down(i, heap.size())) sift_up(i);
    }
}

void heap_timer::adjust(int id, int timeout) {
    assert(!heap.empty() && ref.count(id) > 0);
    size_t i = ref[id];
    heap[i].expires = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout);
    if (!sift_down(i, heap.size())) sift_up(i);
}

void heap_timer::do_work(int id) {
    if (heap.empty() || ref.count(id) == 0) return;
    size_t i = ref[id];
    time_node node = heap[i];
    node.timer_cb();
    del(i);
}

void heap_timer::tick() {
    if (heap.empty()) return;
    while (!heap.empty()) {
        time_node node = heap.front();
        if (std::chrono::steady_clock::now() < node.expires) break;
        node.timer_cb();
        pop();
    }
}

void heap_timer::pop() {
    assert(!heap.empty());
    del(0);
}

void heap_timer::clear() {
    ref.clear();
    heap.clear();
}

int heap_timer::get_next_tick() {
    tick();
    size_t res = -1;
    if (!heap.empty()) {
        auto now = std::chrono::steady_clock::now();
        auto expires = heap.front().expires;
        res = std::chrono::duration_cast<std::chrono::milliseconds>(expires - now).count();
        if (res < 0) res = 0;
    }
    return res;
}