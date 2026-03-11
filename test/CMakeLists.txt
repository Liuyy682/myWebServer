cmake_minimum_required(VERSION 3.10)

# 设置项目名称
project(TestModules)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 包含头文件目录
include_directories(../include)

# 添加可执行文件
add_executable(test_epoller test_epoller.cpp ../src/epoller.cpp)
add_executable(test_http_conn test_http_conn.cpp ../src/http_conn.cpp ../src/httprequest.cpp ../src/httpresponse.cpp ../src/buffer.cpp ../src/log.cpp)
add_executable(test_httprequest test_httprequest.cpp ../src/httprequest.cpp ../src/buffer.cpp ../src/log.cpp)
add_executable(test_httpresponse test_httpresponse.cpp ../src/httpresponse.cpp ../src/buffer.cpp ../src/log.cpp)
add_executable(test_buffer test_buffer.cpp ../src/buffer.cpp)