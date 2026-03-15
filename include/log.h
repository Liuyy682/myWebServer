#pragma once

#include <atomic>
#include <cstdarg>
#include <condition_variable>
#include <fstream>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

class Log {
public:
    enum class Level { Info, Warn, Error, Debug };

    static Log& instance();

    void log(Level level, const std::string &msg);
    void logf(Level level, const char* fmt, ...);

    ~Log();

private:
    Log();

    Log(const Log&) = delete;
    Log& operator=(const Log&) = delete;

    void writer_loop();

    std::mutex log_mtx;
    std::condition_variable cv;
    std::queue<std::string> msg_queue;
    std::thread writer_thread;
    std::ofstream log_file;
    std::atomic<bool> running;
};

#define LOG_INFO(fmt, ...) Log::instance().logf(Log::Level::Info, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...) Log::instance().logf(Log::Level::Warn, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) Log::instance().logf(Log::Level::Error, fmt, ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...) Log::instance().logf(Log::Level::Debug, fmt, ##__VA_ARGS__)