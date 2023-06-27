cmake_minimum_required(VERSION 3.2)

project(mysqllib)

add_library(mysqllib_static STATIC mysqllib.cpp)

set_target_properties(mysqllib_static PROPERTIES  OUTPUT_NAME "mysqllib")

add_library(mysqllib SHARED mysqllib.cpp)

include_directories(../Python/include ../MySQL/include)

link_directories(../MySQL/lib)

target_link_libraries(${CMAKE_PROJECT_NAME} mysqlclient boost_python38)
