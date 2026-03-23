#include "server/webserver.h"

#include <algorithm>
#include <cctype>
#include <climits>
#include <cstdlib>
#include <string>

namespace {

std::string env_str(const char* key, const std::string& default_val) {
    const char* value = std::getenv(key);
    if (!value || *value == '\0') {
        return default_val;
    }
    return std::string(value);
}

int env_int(const char* key, int default_val) {
    const char* value = std::getenv(key);
    if (!value || *value == '\0') {
        return default_val;
    }
    char* end = nullptr;
    long parsed = std::strtol(value, &end, 10);
    if (end == value || *end != '\0' || parsed < INT_MIN || parsed > INT_MAX) {
        return default_val;
    }
    return static_cast<int>(parsed);
}

bool env_bool(const char* key, bool default_val) {
    std::string value = env_str(key, "");
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });

    if (value.empty()) {
        return default_val;
    }
    if (value == "1" || value == "true" || value == "yes" || value == "on") {
        return true;
    }
    if (value == "0" || value == "false" || value == "no" || value == "off") {
        return false;
    }
    return default_val;
}

size_t env_size_t(const char* key, size_t default_val) {
    int parsed = env_int(key, static_cast<int>(default_val));
    if (parsed < 0) {
        return default_val;
    }
    return static_cast<size_t>(parsed);
}

} // namespace

int main() {
    int server_port = env_int("SERVER_PORT", 1234);
    bool server_open_linger = env_bool("SERVER_OPEN_LINGER", false);
    size_t server_threads = env_size_t("SERVER_THREADS", 8);
    size_t server_max_task_queue = env_size_t("SERVER_MAX_TASK_QUEUE", 10000);
    int server_trig_mode = env_int("SERVER_TRIG_MODE", 3);
    int server_log_write = env_bool("SERVER_LOG_ENABLED", false) ? 1 : 0;

    std::string db_host = env_str("DB_HOST", "localhost");
    int db_port = env_int("DB_PORT", 3306);
    std::string db_user = env_str("DB_USER", "root");
    std::string db_password = env_str("DB_PASSWORD", "root");
    std::string db_name = env_str("DB_NAME", "webserver");
    int db_pool_size = env_int("DB_POOL_SIZE", 10);

    webserver server(server_port,
                     server_open_linger,
                     server_threads,
                     server_max_task_queue,
                     server_trig_mode,
                     server_log_write,
                     db_host,
                     db_port,
                     db_user,
                     db_password,
                     db_name,
                     db_pool_size);
    
    server.start();

    return 0;
}