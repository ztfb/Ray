cmake_minimum_required(VERSION 3.0)

project(server)

add_executable(${CMAKE_PROJECT_NAME} main.cpp Server.cpp Log.cpp ThreadPool.cpp Buffer.cpp MySQLPool.cpp)

# 引入依赖库
target_link_libraries(${CMAKE_PROJECT_NAME} mysqlclient pthread)
