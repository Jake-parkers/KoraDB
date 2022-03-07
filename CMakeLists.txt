cmake_minimum_required(VERSION 3.16.3)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -pthread")

project(koradb VERSION 1.0.1 DESCRIPTION "A persistent key-value store")

include(GNUInstallDirs)

add_library(koradb SHARED src/kdb.cpp src/options.cpp src/status.cpp src/storage_engine.cpp)

set_target_properties(koradb PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION 1 PUBLIC_HEADER "include/data.h;include/helper.h;include/kdb.h;include/options.h;include/result.h;include/status.h;include/storage_engine.h;include/timer.h")

configure_file(koradb.pc.in koradb.pc @ONLY)

target_include_directories(koradb PRIVATE .)

install(TARGETS koradb LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/koradb)

install(FILES ${CMAKE_BINARY_DIR}/koradb.pc DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)