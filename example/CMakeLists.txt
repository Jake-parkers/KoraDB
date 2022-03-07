cmake_minimum_required(VERSION 3.16.3)
project(example)

add_executable(example main.cpp)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -pthread")

include_directories(/usr/local/lib)

find_library(KORADB libkoradb.dylib PATHS /usr/local/lib)

target_link_libraries(example ${KORADB})
target_include_directories(example PUBLIC  $<BUILD_INTERFACE:/usr/local/include>)