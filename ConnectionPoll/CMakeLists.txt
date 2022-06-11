cmake_minimum_required(VERSION 3.2)
project(testjson)
set(CMAKE_BUILD_TYPE Release)
add_compile_options(-std=c++11)
include_directories ("${PROJECT_SOURCE_DIR}/include")
include_directories ("/usr/local/mysql-8.0.26-macos11-arm64/include")
link_directories("/usr/local/mysql-8.0.26-macos11-arm64/lib")

add_library(jsoncpp SHARED IMPORTED)
set_target_properties(jsoncpp PROPERTIES
    IMPORTED_LOCATION "/opt/homebrew/Cellar/jsoncpp/1.9.4_1/lib/libjsoncpp.dylib"
    INTERFACE_INCLUDE_DIRECTORIES "/opt/homebrew/Cellar/jsoncpp/1.9.4_1/include"
)

add_executable(mysqlConnectionPool "test.cpp" ConnectionPoll.cpp ConnectionPoll.h MysqlConn.cpp MysqlConn.h)
target_link_libraries(mysqlConnectionPool jsoncpp mysqlclient)
