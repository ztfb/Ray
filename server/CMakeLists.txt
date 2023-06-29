cmake_minimum_required(VERSION 3.2)

project(server)

add_executable(${CMAKE_PROJECT_NAME} main.cpp Server.cpp Log.cpp ThreadPool.cpp Buffer.cpp MySQLPool.cpp Epoll.cpp Timer.cpp RunPython.cpp Connection.cpp HttpParser.cpp)

include_directories(../Python/include ../MySQL/include)

link_directories(../Python/lib ../MySQL/lib)

target_link_libraries(${CMAKE_PROJECT_NAME} mysqlclient pthread python3.8)
