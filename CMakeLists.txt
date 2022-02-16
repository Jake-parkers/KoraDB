cmake_minimum_required(VERSION 3.16.3)
project(kv_store)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

file(GLOB SOURCES "src/*.cpp")

add_executable(kv_store ${SOURCES})