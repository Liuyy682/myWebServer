# CMake generated Testfile for 
# Source directory: /root/myWebServer
# Build directory: /root/myWebServer/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(TestThreadpool "/root/myWebServer/build/test/TestThreadpool")
set_tests_properties(TestThreadpool PROPERTIES  _BACKTRACE_TRIPLES "/root/myWebServer/CMakeLists.txt;34;add_test;/root/myWebServer/CMakeLists.txt;0;")
subdirs("test")
