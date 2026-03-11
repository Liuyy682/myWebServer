#pragma once

#include <queue>
#include <vector>
#include <thread>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <future>
#include "log.h"

class threadpool {
public:
    threadpool(size_t core_poolsize) : 
    core_poolsize(core_poolsize){
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
    void submit(F&& task) {
        {
            std::unique_lock<std::mutex> lock(pool_mtx);
            tasks.emplace(std::forward<F>(task));
        }
        pool_cv.notify_one();
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
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex pool_mtx;
    std::condition_variable pool_cv;
    std::atomic<bool> pool_stop{false};
};