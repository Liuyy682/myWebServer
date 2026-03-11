#pragma once

#include <mutex>
#include <string>

class Log {
public:
    enum Level { Info, Warn, Error, Debug };

    static Log& instance();

    void log(Level level, const std::string &msg);

private:
    Log() = default;

    std::mutex log_mtx;
};

#define LOG_INFO(msg) Log::instance().log(Log::Level::Info, msg)
#define LOG_WARN(msg) Log::instance().log(Log::Level::Warn, msg)
#define LOG_ERROR(msg) Log::instance().log(Log::Level::Error, msg)
#define LOG_DEBUG(msg) Log::instance().log(Log::Level::Debug, msg)