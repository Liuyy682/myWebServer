#pragma once

#include <atomic>
#include <cstdarg>
#include <condition_variable>
#include <fstream>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <chrono>
#include <cstdio>
#include <cstdarg>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <vector>

class Log {
public:
    enum class Level { Info, Warn, Error, Debug };

    static Log& instance();
    static void set_enabled(bool enabled_flag);
    static bool is_enabled();

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
    inline static std::atomic<bool> enabled{true};
};

#ifndef OPEN_LOGGING
#define LOG_INFO(fmt, ...) do { if (Log::is_enabled()) Log::instance().logf(Log::Level::Info, fmt, ##__VA_ARGS__); } while(0)
#define LOG_WARN(fmt, ...) do { if (Log::is_enabled()) Log::instance().logf(Log::Level::Warn, fmt, ##__VA_ARGS__); } while(0)
#define LOG_ERROR(fmt, ...) do { if (Log::is_enabled()) Log::instance().logf(Log::Level::Error, fmt, ##__VA_ARGS__); } while(0)
#define LOG_DEBUG(fmt, ...) do { if (Log::is_enabled()) Log::instance().logf(Log::Level::Debug, fmt, ##__VA_ARGS__); } while(0)
#else
#define LOG_INFO(fmt, ...) do {} while(0)
#define LOG_WARN(fmt, ...) do {} while(0)
#define LOG_ERROR(fmt, ...) do {} while(0)
#define LOG_DEBUG(fmt, ...) do {} while(0)
#endif