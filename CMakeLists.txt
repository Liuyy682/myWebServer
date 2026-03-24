# 设置CMake最低版本要求
cmake_minimum_required(VERSION 3.10)

# 定义项目名称和使用语言
project(WebServerProject LANGUAGES CXX)

find_package(Threads REQUIRED)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 包含头文件目录
include_directories(
    apps/web_server/include
)

# 指定源文件
set(SOURCE_FILES
    apps/web_server/src/log/log.cpp
    apps/web_server/src/server/webserver.cpp
    apps/web_server/src/epoller/epoller.cpp
    apps/web_server/src/http/http_conn.cpp
    apps/web_server/src/http/httprequest.cpp
    apps/web_server/src/http/httpresponse.cpp
    apps/web_server/src/buffer/buffer.cpp
    apps/web_server/src/timer/heap_timer.cpp
    apps/web_server/src/util/sql_conn_pool.cpp
    main.cpp
)

# 添加可执行文件
add_executable(server ${SOURCE_FILES})
target_compile_definitions(server PRIVATE PROJECT_ROOT_DIR="${CMAKE_SOURCE_DIR}")
target_link_libraries(server PRIVATE Threads::Threads mysqlclient)

add_custom_target(webbench_install
    COMMAND bash ${CMAKE_SOURCE_DIR}/scripts/install_webbench.sh
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Install wrk into tools/wrk/bin"
)

add_custom_target(webbench_run
    COMMAND ${CMAKE_COMMAND} -E env
            WRK_THREADS=4
            WRK_CONNECTIONS=400
            WRK_DURATION=10s
            WRK_URL=http://127.0.0.1:8080/
            bash ${CMAKE_SOURCE_DIR}/scripts/run_webbench.sh
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Run wrk with default pressure settings"
)

