#include "log.h"

#ifndef PROJECT_ROOT_DIR
#define PROJECT_ROOT_DIR "."
#endif

Log& Log::instance() {
    static Log inst;
    return inst;
}

void Log::set_enabled(bool enabled_flag) {
    enabled.store(enabled_flag, std::memory_order_relaxed);
}

bool Log::is_enabled() {
    return enabled.load(std::memory_order_relaxed);
}

Log::Log() : running(true) {
    const std::string log_path = std::string(PROJECT_ROOT_DIR) + "/webserver.log";
    log_file.open(log_path, std::ios::out | std::ios::app);
    if (!log_file.is_open()) {
        throw std::runtime_error("Failed to open log file: " + log_path);
    }
    writer_thread = std::thread(&Log::writer_loop, this);
}

Log::~Log() {
    running.store(false);
    cv.notify_all();

    if (writer_thread.joinable()) {
        writer_thread.join();
    }

    if (log_file.is_open()) {
        log_file.flush();
        log_file.close();
    }
}

void Log::log(Level level, const std::string &msg) {
    if (!is_enabled()) {
        return;
    }

    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::tm tm_buf{};
    localtime_r(&time_t, &tm_buf);

    std::stringstream ss;
    ss << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S");

    const char* level_str;
    switch (level) {
        case Level::Info:  level_str = "INFO";  break;
        case Level::Warn:  level_str = "WARN";  break;
        case Level::Error: level_str = "ERROR"; break;
        case Level::Debug: level_str = "DEBUG"; break;
    }

    std::string line = "[" + ss.str() + "] [" + level_str + "] " + msg;
    {
        std::lock_guard<std::mutex> lock(log_mtx);
        msg_queue.push(std::move(line));
    }
    cv.notify_one();
}

void Log::logf(Level level, const char* fmt, ...) {
    if (!is_enabled()) {
        return;
    }

    if (fmt == nullptr) {
        log(level, "");
        return;
    }

    va_list args;
    va_start(args, fmt);

    va_list args_copy;
    va_copy(args_copy, args);
    const int needed = std::vsnprintf(nullptr, 0, fmt, args_copy);
    va_end(args_copy);

    if (needed < 0) {
        va_end(args);
        log(level, fmt);
        return;
    }
    
    std::vector<char> local_buffer(static_cast<size_t>(needed) + 1);
    std::vsnprintf(local_buffer.data(), local_buffer.size(), fmt, args);
    va_end(args);

    log(level, std::string(local_buffer.data(), static_cast<size_t>(needed)));
}

void Log::writer_loop() {
    while (running.load() || !msg_queue.empty()) {
        std::unique_lock<std::mutex> lock(log_mtx);
        cv.wait(lock, [this] { return !running.load() || !msg_queue.empty(); });

        while (!msg_queue.empty()) {
            log_file << msg_queue.front() << std::endl;
            msg_queue.pop();
        }
        log_file.flush();
    }
}