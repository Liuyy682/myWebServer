#pragma once

#include <queue>
#include <vector>
#include <thread>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <future>
#include "log/log.h"

class threadpool {
public:
    threadpool(size_t core_poolsize, size_t max_task_queue = 0) : 
    core_poolsize(core_poolsize), max_task_queue(max_task_queue) {
        for (size_t i = 0; i < core_poolsize; ++i) {
            workers.emplace_back([this]() { this->worker_func(); });
        }
    }

    ~threadpool() {
        pool_stop.store(true);
        pool_cv.notify_all();
        for (std::thread &worker : workers) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }
    
    template<typename F>
    bool submit(F&& task) {
        {
            std::unique_lock<std::mutex> lock(pool_mtx);
            if (pool_stop.load()) {
                return false;
            }
            if (max_task_queue > 0 && tasks.size() >= max_task_queue) {
                return false;
            }
            tasks.emplace(std::forward<F>(task));
        }
        pool_cv.notify_one();
        return true;
    }

    size_t pending_tasks() const {
        std::lock_guard<std::mutex> lock(pool_mtx);
        return tasks.size();
    }

    size_t task_queue_limit() const {
        return max_task_queue;
    }

    bool is_stopping() const {
        return pool_stop.load();
    }

private:
    void worker_func() {
        while (true) {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(pool_mtx);
                pool_cv.wait(lock, [this]() { return pool_stop.load() || !tasks.empty(); });
                if (pool_stop.load() && tasks.empty()) {
                    return;
                }
                task = std::move(tasks.front());
                tasks.pop();
            }
            task();
        }
    }

    size_t core_poolsize{4};
    size_t max_task_queue{0};
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    mutable std::mutex pool_mtx;
    std::condition_variable pool_cv;
    std::atomic<bool> pool_stop{false};
};