cmake_minimum_required(VERSION 3.16.3)
project(kv_store)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -pthread")

include_directories(include)

file(GLOB SOURCES "src/*.cpp")

add_executable(kv_store ${SOURCES})