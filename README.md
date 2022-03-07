# KoraDB
Kora is a persistent key-value database that provides ordered mapping between key-value pairs. 

## Features

- Data stored is sorted by key

- Keys and values are stored as bytes

- The supported ops are Get(key), Set(key, value), Delete(key)

- The compaction strategy used is size-tiered compaction (compaction is done in the background periodically)

## Cloning the project

```
git clone https://github.com/Jake-parkers/KoraDB.git
```
## Building

This project uses cmake and can only be built and used on *POSIX* systems for now.

### Quick Start

```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
sudo make install
```

## Usage
This project was built as a library that can be embedded into other projects that need a persistent key-value store. Hence building the project will install the library into `/usr/local/include/koradb` and `/usr/local/lib`. 

### Linking to another project

Assuming that our project's name is `untitled`, we can use the library like so:

```
cmake_minimum_required(VERSION 3.21)
project(untitled)

add_executable(untitled main.cpp)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -pthread")

include_directories(/usr/local/lib)

find_library(KORATEST libkora PATHS /usr/local/lib)

target_link_libraries(untitled ${KORATEST})
target_include_directories(untitled PUBLIC  $<BUILD_INTERFACE:/usr/local/include>)

```
See the embedded example folder for a sample project and usage

## Project Files

Brief description of the source files and the header files

### kdb.h & kdb.cpp

This is the main interface to the db. It contains all DB constructor and the  `Get`, `Set` and `Delete` methods.

### storage_engine.h & storage_engine.cpp

This is the heart of the project. It contains the interface and implementation for writing to, reading from, deleting from the db as well as merging and compaction.

### status.h & status.cpp

This class is used as a return type by other classes to indicate sucess or failure.

### data.h

This header file is a simple implementation of a class that holds a byte array of and its length

### result.h

This class is used as a return type just like Status. The difference between the two is that if a method needs to return additional data as an `std::string`, the Result class can handle it

### options.h

This class contains different options that control how the database behaves.