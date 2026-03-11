#include "log.h"
#include <chrono>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <iostream>

Log& Log::instance() {
    static Log inst;
    return inst;
}

void Log::log(Level level, const std::string &msg) {
    std::unique_lock<std::mutex> lock(log_mtx);

    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");

    const char* level_str;
    switch (level) {
        case Level::Info:  level_str = "INFO";  break;
        case Level::Warn:  level_str = "WARN";  break;
        case Level::Error: level_str = "ERROR"; break;
        case Level::Debug: level_str = "DEBUG"; break;
    }

    std::cout << "[" << ss.str() << "] [" << level_str << "] " << msg << std::endl; 
}