# 设置CMake最低版本要求
cmake_minimum_required(VERSION 3.10)

# 定义项目名称和使用语言
project(WebServerProject LANGUAGES CXX)

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
    main.cpp
)

# 添加可执行文件
add_executable(server ${SOURCE_FILES})

