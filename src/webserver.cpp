#include "webserver.h"

webserver::webserver(int port, bool open_linger, size_t core_poolsize, int trig_mode, int is_log_write)
        : port(port), open_linger(open_linger), is_close(false), trig_mode(trig_mode), is_log_write(is_log_write),
      pool(std::make_unique<threadpool>(core_poolsize)), epollers(std::make_unique<epoller>()),
      timer(std::make_unique<heap_timer>()) {
    Log::set_enabled(is_log_write != 0);

    src_dir = getcwd(nullptr, 256);
    assert(src_dir);

    std::string cwd(src_dir);
    std::string resource_path = cwd + "/resources/";
    if (access(resource_path.c_str(), F_OK) != 0) {
        std::string parent_resource_path = cwd + "/../resources/";
        if (access(parent_resource_path.c_str(), F_OK) == 0) {
            resource_path = parent_resource_path;
        }
    }

    char* resolved_path = realpath(resource_path.c_str(), nullptr);
    if (resolved_path) {
        resource_path = resolved_path;
        free(resolved_path);
        if (resource_path.empty() || resource_path.back() != '/') {
            resource_path.push_back('/');
        }
    }

    free(src_dir);
    src_dir = static_cast<char*>(malloc(resource_path.size() + 1));
    assert(src_dir);
    memcpy(src_dir, resource_path.c_str(), resource_path.size() + 1);

    http_conn::src_dir = src_dir;
    http_conn::user_count.store(0);

    const char* db_host = "localhost";
    const char* db_user = "root";
    const char* db_password = "root";
    const char* db_name = "webserver";
    const char* db_port_env = "1234";
    const char* db_conn_size_env = "10";
    if (db_host && db_user && db_password && db_name) {
        int db_port = db_port_env ? std::atoi(db_port_env) : 3306;
        int db_conn_size = db_conn_size_env ? std::atoi(db_conn_size_env) : 10;
        if (db_port <= 0) {
            db_port = 3306;
        }
        if (db_conn_size <= 0) {
            db_conn_size = 10;
        }
        sql_conn_pool::get_instance()->init(db_host, db_port, db_user, db_password, db_name, db_conn_size);
        LOG_INFO("MySQL pool initialized");
    } else {
        LOG_WARN("MySQL env not fully configured (need DB_HOST/DB_USER/DB_PASSWORD/DB_NAME), auth will return error page");
    }

    init_event_mode(trig_mode);
    init_socket();
}

webserver::~webserver() {
    close(listen_fd);
    is_close = true;
    free(src_dir);
}

void webserver::init_socket() {
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    assert(listen_fd >= 0);

    int ret = 0;
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(port);

    struct linger linger = {0};
    if (open_linger) {
        linger.l_linger = 1;
        linger.l_onoff = 1;
    }

    int flag = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
    ret = bind(listen_fd, (struct sockaddr *)&address, sizeof(address));
    assert(ret >= 0);
    ret = listen(listen_fd, 5);
    assert(ret >= 0);
    set_nonblock(listen_fd);
    LOG_INFO("Server initialized");
}

void webserver::init_event_mode(int trig_mode) {
    conn_event = EPOLLRDHUP | EPOLLONESHOT;
    listen_event = EPOLLRDHUP;
    switch(trig_mode) {
        case 0:
            break;
        case 1:
            conn_event |= EPOLLET;
            break;
        case 2:
            listen_event |= EPOLLET;
            break;
        case 3:
            listen_event |= EPOLLET;
            conn_event |= EPOLLET;
            break;
        default:
            LOG_WARN("Invalid trig_mode, defaulting to EPOLLET for both conn_event and listen_event");
            listen_event |= EPOLLET;
            conn_event |= EPOLLET;
    }
    http_conn::is_ET = conn_event & EPOLLET;
}

void webserver::start() {
    int time_ms = -1;
    epollers->add_fd(listen_fd, EPOLLIN | EPOLLET);
    while (!is_close) {
        if (timeout_ms > 0) {
                time_ms = timer->get_next_tick();
            }
        int event_count = epollers->wait(time_ms);
        for (int i = 0; i < event_count; ++i) {
            int sock_fd = epollers->get_event_fd(i);
            uint32_t events = epollers->get_events(i);
            if (sock_fd == listen_fd) {
                deal_client();
            }
            else if (events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                auto it = users.find(sock_fd);
                if (it != users.end()) {
                    if (it->second.get_fd() >= 0) {
                        close_conn(&it->second);
                    }
                }
            }
            else if (events & EPOLLIN) {
                auto it = users.find(sock_fd);
                if (it != users.end()) {
                    if (it->second.get_fd() >= 0) {
                        deal_read(&it->second);
                    }
                }
            }
            else if (events & EPOLLOUT) {
                auto it = users.find(sock_fd);
                if (it != users.end()) {
                    if (it->second.get_fd() >= 0) {
                        deal_write(&it->second);
                    }
                }
            }
        }
    }
}

void webserver::deal_client() {
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    do {
        client_addr_len = sizeof(client_addr);
        int conn_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &client_addr_len);
        if (conn_fd < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            }
            LOG_ERROR("Accept failed");
            break;
        }

        if (http_conn::user_count.load() >= 10000) {
            LOG_ERROR("Too many clients");
            close(conn_fd);
            continue;
        }

        set_nonblock(conn_fd);
        users[conn_fd].init(conn_fd, client_addr);
        if (timeout_ms > 0) {
            timer->add(conn_fd, timeout_ms, std::bind(&webserver::close_conn, this, &users[conn_fd]));
        }
        epollers->add_fd(conn_fd, EPOLLIN | conn_event);
        LOG_INFO("New client connected");
    } while (listen_event & EPOLLET);
}

void webserver::deal_read(http_conn* client) {
    extent_time(client);
    pool->submit(std::bind(&webserver::on_read, this, client));
}

void webserver::deal_write(http_conn* client) {
    extent_time(client);
    pool->submit(std::bind(&webserver::on_write, this, client));
}

void webserver::close_conn(http_conn* client) {
    epollers->del_fd(client->get_fd());
    client->close_conn();
    LOG_DEBUG("Client connection closed");
}

void webserver::on_read(http_conn* client) {
    if (client->get_fd() < 0) {
        return;
    }
    int read_errno = 0;
    ssize_t ret = client->read(&read_errno);
    if (ret <= 0) {
        if (ret < 0 && (read_errno == EAGAIN || read_errno == EWOULDBLOCK)) {
            epollers->mod_fd(client->get_fd(), conn_event | EPOLLIN);
            return;
        }
        close_conn(client);
        return;
    }
    LOG_DEBUG("Read data from client");
    on_process(client);
}

void webserver::on_write(http_conn* client) {
    if (client->get_fd() < 0) {
        return;
    }
    int write_errno = 0;
    ssize_t ret = client->write(&write_errno);
    if (client->to_write_bytes() == 0) {
        if (client->is_keep_alive()) {
            epollers->mod_fd(client->get_fd(), conn_event | EPOLLIN);
            return;
        }
        close_conn(client);
        return;
    }
    if (ret <= 0) {
        if (write_errno == EAGAIN) {
            epollers->mod_fd(client->get_fd(), conn_event | EPOLLOUT);
            return;
        }
        close_conn(client);
        return;
    }
    LOG_DEBUG("Write data to client");
}

void webserver::on_process(http_conn* client) {
    if (client->get_fd() < 0) {
        return;
    }
    if (client->process()) {
        epollers->mod_fd(client->get_fd(), conn_event | EPOLLOUT);
    } else {
        epollers->mod_fd(client->get_fd(), conn_event | EPOLLIN);
    }
}

int webserver::set_nonblock(int fd) {
    assert(fd > 0);
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
}

void webserver::extent_time(http_conn* client) {
    if (timeout_ms > 0) {
        int fd = client->get_fd();
        if (fd >= 0) {
            timer->adjust(fd, timeout_ms);
        }
    }
}