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

Ensure you have at least g++-9 and gcc-9 installed. You can reference this article https://linuxhint.com/install-and-use-g-on-ubuntu/ for guidelines on how to go this.

```
mkdir build
cd build
cmake -D CMAKE_BUILD_TYPE=Release -D CMAKE_C_COMPILER=gcc-9 -D CMAKE_CXX_COMPILER=g++-9 ..
sudo make install
```

## Usage
This project was built as a library that can be embedded into other projects that need a persistent key-value store. Hence building the project will install the library into `/usr/local/include/koradb` and `/usr/local/lib`. 

### Linking to another project

Assuming that our project's name is `example`, we can use the library like so:

```
cmake_minimum_required(VERSION 3.16.3)
project(example)

add_executable(example main.cpp)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -pthread")

include_directories(/usr/local/lib)

find_library(KORADB libkoradb.so PATHS /usr/local/lib)

target_link_libraries(example ${KORADB})
target_include_directories(example PUBLIC  $<BUILD_INTERFACE:/usr/local/include>)

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

## Implementation Details

See [impl.md](https://github.com/Jake-parkers/KoraDB/blob/main/impl.md) for more details on how the database was implemented


## Rubric Points

The following rubric points were addressed in this project

### Loops, Functions, I/O

Several control structures and functions are used in this project

- The project reads data from a file and processes it: StorageEngine::Search on line 91 in `storage_engine.cpp`
- The project writes data to a file: StorageEngine::LogData on line 150 in `storage_engine.cpp`
- The project writes data to a file: StorageEngine::Write on line 162 in `storage_engine.cpp`
- The project reads data from and writes data to a file: StorageEngine::Compact on line 237 in `storage_engine.cpp`
- The projects reads data and processes it: StorageEngine::CreateIndexFromCompactedSegment on line 433 in `storage_engine.cpp`
- The project defines several helper functions: `include/helper.h`
- The project uses while loops, for loops, if statements and switch statements in `storage_engine.cpp` on lines 29, 69, 77, 95, 97, 154, 163, 182, 201, 241, 250, 278, 442, 463, 496 etc.
- The project uses a switch statement: Status::toString on line 9 in `status.cpp`
- The project uses if/else statements in `data.h` on lines 52, 56, 60, 62, 68

### Object Oriented Programming

This project applies several OOP techniques

-The project is organized into several classes with appropriate specifiers in `data.h` (line 13), `kdb.h` (line 21), `result.h` (line 13) `status.h` (line 20) `storage_engine.h` (line 20) `timer.h` (line 12)
- This project uses member initializer lists in `status.h` on lines 22, 23, 24, 
- This project uses member initializer lists in `timer.h` on line 14
- This project uses member initializer lists in `kdb.h` on lines 27, 31
- This project uses member initializer lists in `data.h` on lines 16, 17, 18, 19, 22, 31
- Classes in this project abstract implementation from their interface

### Memory Management

- Destructors are used in `timer.h` and `storage_engine.h` on lines 16 and 43 respectively
- Move semantics is used in this project in `storage_engine.h` on lines 37, 38, 39
- Move semantics is used in this project in `result.h` on lines 17, 18
- This project passed data by reference in `result.h` on line 60

### Concurrency

- This process spins up threads in `timer.h` on line 17 and `storage_engine.h` on line 30
- std::lock_guard is used in `storage_engine.cpp` on lines 68, 142
- std::condition_variable is used in `storage_engine.h` on line 60 and in `storage_engine.cpp` on lines 51, 53, 165, 233
