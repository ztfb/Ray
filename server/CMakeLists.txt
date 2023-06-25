cmake_minimum_required(VERSION 3.0)

project(server)

#查找依赖库
find_package(Threads REQUIRED)

add_executable(${CMAKE_PROJECT_NAME} main.cpp Server.cpp Log.cpp)

#引入依赖库
target_link_libraries(${CMAKE_PROJECT_NAME} Threads::Threads)