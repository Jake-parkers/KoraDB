//
// Created by Joshua Kwaku on 13/02/2022.
//

#include "storage_engine.h"
#include "helper.h"
#include <cstring>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <vector>
#include <sstream>
namespace fs = std::filesystem;

Kora::Status Kora::StorageEngine::Set(Data&& key, Data&& value) noexcept {
    /**
     * insert key and value into the memtable
     * update the memtable approx size
     * add the new data to the log file
     */
     std::cout << "Key Address = " << &key << " Value Address = " << &value << "\n";
    size_t key_size = key.size();
    size_t value_size = value.size();
    char data[key_size + value_size + 1]; //extra bytes for new line character and the new line characters of key and value
    // strcpy copies the string pointed to by source including the null character
    strcpy(data, key.data());
    strcpy(&data[key_size], value.data());
    data[strlen(data)] = '\n';
    std::unique_lock<std::mutex> ulock(_mutex);
    _memtable.insert(std::make_pair(key, value));
    _memtableSize += sizeof(key) + sizeof(value);
    ulock.unlock();
    LogData(data, key_size, value_size);
    ulock.lock();
    if (_memtableSize >= 1024) {
        _temp_memtable = std::move(_memtable);
        _memtable = std::map<Data, Data, std::less<>>();
        _memtable_is_full = true;
        _cond.notify_one();
        _compaction_cond.notify_one();
    }
    return {};
}

Kora::Result Kora::StorageEngine::Get(const Kora::Data& key) {
    /**
     * Convert key to char array
     * check the memtable first
     * if not there then recursively check the sstables
     */
    std::lock_guard<std::mutex> lg(_mutex);
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

Kora::Status Kora::StorageEngine::Write() {
    std::cout << "Hello from " << std::this_thread::get_id() << "\n";
    std::unique_lock<std::mutex> ulock(_mutex);
    _cond.wait(ulock, [this]{ return _memtable_is_full; });
    std::cout << "memtable size = " << memtableSize() << '\n';
    _memtable_is_full = false;
    auto path = Kora::getDBPath();
    path /= today() + ".sst";
    std::ofstream logfile(path.string(), std::ios::binary | std::ios_base::app);
    if (logfile.is_open()) {
        for(const auto& [key, value]: _temp_memtable) {
            std::cout << "Key: " << key.data() << " Value: " << value.data() << '\n';
            size_t key_size = key.size();
            size_t value_size = value.size();
            size_t total_size = key.size() + value.size();
            logfile.write(reinterpret_cast<char*>(&key_size), sizeof(key.size()));
            logfile.write(reinterpret_cast<char*>(&value_size), sizeof(value.size()));
            logfile.write(reinterpret_cast<char*>(&total_size), sizeof(total_size));
            logfile.write(key.data(), sizeof(key.data()));
            logfile.write(value.data(), sizeof(value.data()));
        }
        logfile.close();
    }
    _temp_memtable.erase(_temp_memtable.begin(), _temp_memtable.end());
    return {};
}

Kora::Status Kora::StorageEngine::Compact() {
    std::cout << "Hello from the compaction thread!! ID = " << std::this_thread::get_id() << '\n';
    std::unique_lock<std::mutex> ulock(_mutex);
    _compaction_cond.wait(ulock, [this]{ return _memtable_is_full; });
    ulock.unlock();
    // loop through db folder
    // check if no of ssts is at least 2
    // pick two ssts of the same size and merge them into one
    // repeatedly do this until we have just one sst or we reach a filesize that we have decided not to merge
    auto db_path = getDBPath();
    uintmax_t size = 0;
    int file_count = 0;
    std::vector<CompactibleObject> compactible_files;
    struct CompactibleObject c_file;
    std::ifstream f;
    for (auto const& dir_entry: fs::directory_iterator{db_path}) {
        if(dir_entry.exists() && dir_entry.is_regular_file()) {
            if(dir_entry.path().has_extension()) {
                auto ext = dir_entry.path().extension().string();
                if (ext == ".sst") {
                    if (file_count == 2) {
                        auto new_segment_path = Kora::getDBPath();
                        new_segment_path /= today() + ".sst";
                        std::ifstream file1(compactible_files[0].filepath.string());
                        std::ifstream file2(compactible_files[1].filepath.string());
                        std::ofstream new_segment (new_segment_path, std::ios::binary | std::ios::app);
                        std::string file1_bin_string;
                        std::string file2_bin_string;
                        size_t key_size, value_size, total_size;
                        size_t key_size2, value_size2, total_size2;
                        char *key = nullptr; char * value = nullptr;
                        char *key2 = nullptr; char * value2 = nullptr;
                        while(!file1.eof() && !file2.eof()) {
                            getline(file1, file1_bin_string);
                            getline(file2, file2_bin_string);
                            std::stringstream linestream1(file1_bin_string);
                            std::stringstream linestream2(file2_bin_string);
                            linestream1 >> key_size >> value_size >> total_size >> key >> value;
                            linestream2 >> key_size2 >> value_size2 >> total_size2 >> key2 >> value2;
                            if (strcmp(key, key2) > 0) {
                                // key is greater than key2
                                new_segment.write(reinterpret_cast<char*>(&key_size), sizeof(key_size));
                                new_segment.write(reinterpret_cast<char*>(&value_size), sizeof(value_size));
                                new_segment.write(reinterpret_cast<char*>(&total_size), sizeof(total_size));
                                new_segment.write(key, sizeof(key));
                                new_segment.write(value, sizeof(value));
                            }
                            if (strcmp(key, key2) < 0) {
                                // key is less than key2
                                new_segment.write(reinterpret_cast<char*>(&key_size2), sizeof(key_size2));
                                new_segment.write(reinterpret_cast<char*>(&value_size2), sizeof(value_size2));
                                new_segment.write(reinterpret_cast<char*>(&total_size2), sizeof(total_size2));
                                new_segment.write(key2, sizeof(key2));
                                new_segment.write(value2, sizeof(value2));
                            }
                            if (strcmp(key, key2) == 0) {
                                // they are equal
                                // pick the most recent
                            }
                        }
                    } else {
                        if (size == 0) {
                            c_file = {dir_entry.path().string(), dir_entry.file_size()};
                            compactible_files.push_back(c_file);
                            size = dir_entry.file_size();
                            file_count += 1;
                        } else {
                            uintmax_t size_plus_512 = size + 512;
                            uintmax_t size_minus_512 = dir_entry.file_size() - 512;
                            if (dir_entry.file_size() >= size_minus_512 && dir_entry.file_size() <= size_plus_512) {
                                c_file = {dir_entry.path().string(), dir_entry.file_size()};
                                compactible_files.push_back(c_file);
                                size += dir_entry.file_size();
                                file_count += 1;
                            }
                        }
                    }
                }
            }
        }
    }
    return {};
}