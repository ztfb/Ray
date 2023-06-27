cmake_minimum_required(VERSION 3.2)

project(sqllib)

add_library(sqllib_static STATIC sqllib.cpp)

set_target_properties(sqllib_static PROPERTIES  OUTPUT_NAME "sqllib")

add_library(sqllib SHARED sqllib.cpp)

include_directories(../Python/include ../MySQL/include)

link_directories(../MySQL/lib)

target_link_libraries(${CMAKE_PROJECT_NAME} mysqlclient boost_python38)
