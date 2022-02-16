//
// Created by Joshua Kwaku on 13/02/2022.
//

#include "storage_engine.h"
#include "helper.h"
#include <cstring>
#include <fstream>
#include <iostream>

Kora::Status Kora::StorageEngine::Set(Data&& key, Data&& value) noexcept {
    /**
     * Convert key and value to char array
     * insert them into the memtable
     * update the memtable approx size
     * add the new data to the log file
     */
     std::cout << "Got here\n";
    size_t key_size = key.size();
    size_t value_size = value.size();
    char data[key_size + value_size + 1]; //extra bytes for new line character and the new line characters of key and value
    // strcpy copies the string pointed to by source including the null character
    strcpy(data, key.data());
    strcpy(&data[key_size], value.data());
    data[strlen(data)] = '\n';
    _memtable.insert(std::make_pair(std::move(key), std::move(value)));
    _memtableSize += sizeof(key) + sizeof(value);
    LogData(data, key_size, value_size);
    return {};
}

Kora::Result Kora::StorageEngine::Get(const Kora::Data& key) {
    /**
     * Convert key to char array
     * check the memtable first
     * if not there then recursively check the sstables
     */
    if (_memtable.find(key) != _memtable.end())  {
        return Result {Kora::Status::OK(), std::string(_memtable[key].data(), _memtable[key].size()) };
    } else {
        return Result (Kora::Status(Kora::Code::_NOTFOUND, "Key does not exist"));
    }
}

// TODO: Consider making the log file path a static variable, create it once an instance of db is created
void Kora::StorageEngine::LogData(const char* data, size_t key_size, size_t value_size) {
    auto path = Kora::getDBPath();
    size_t total_size = key_size + value_size;
    path /= "log.kdb";
    std::ofstream logfile(path.string(), std::ios::binary | std::ios_base::app);
    if (logfile.is_open()) {
        logfile.write(reinterpret_cast<char*>(&key_size), sizeof(key_size));
        logfile.write(reinterpret_cast<char*>(&value_size), sizeof(value_size));
        logfile.write(reinterpret_cast<char*>(&total_size), sizeof(total_size));
        logfile.write(data, sizeof(data));
        logfile.close();
    }
}