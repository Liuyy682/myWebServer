# 设置CMake最低版本要求
cmake_minimum_required(VERSION 3.10)

# 定义项目名称和使用语言
project(WebServerProject LANGUAGES CXX)

find_package(Threads REQUIRED)

# 设置C++标准
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 包含头文件目录
include_directories(include)

# 指定源文件
set(SOURCE_FILES
    src/log.cpp
    src/webserver.cpp
    src/epoller.cpp
    src/http_conn.cpp
    src/httprequest.cpp
    src/httpresponse.cpp
    src/buffer.cpp
    src/heap_timer.cpp
    src/sql_conn_pool.cpp
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

