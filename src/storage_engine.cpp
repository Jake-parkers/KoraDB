//
// Created by Joshua Kwaku on 13/02/2022.
//

#include "../include/storage_engine.h"
#include "../include/helper.h"
#include <cstring>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <vector>
#include <sstream>
#include <cstdlib>

namespace fs = std::filesystem;

// initialize static variables
std::map<long, std::string, std::greater<>> Kora::StorageEngine::_sstables = std::map<long, std::string, std::greater<>>();
std::unordered_map<std::string, std::map<std::string, size_t>> Kora::StorageEngine::_hash_indexes = std::unordered_map<std::string, std::map<std::string, size_t>>();
std::string Kora::StorageEngine::_TOMBSTONE_RECORD = "koraDYtombstoneDX";
bool Kora::StorageEngine::_done_updating_sstables = false;


Kora::Status Kora::StorageEngine::Set(Data&& key, Data&& value, bool from_log) noexcept {
    /**
     * insert key and value into the memtable
     * update the memtable approx size
     * add the new data to the log file
     */
    while (true) {
        size_t key_size = key.size();
        size_t value_size = value.size();
        char data[key_size + value_size]; //extra bytes for new line character and the new line characters of key and value
        // strcpy copies the string pointed to by source including the null character
        strcpy(data, key.data());
        strcpy(&data[key_size], value.data());
        std::unique_lock<std::mutex> ulock(_mutex);
        _memtable.insert(std::make_pair(key, value));
        _memtableSize += sizeof(key) + sizeof(value);
        ulock.unlock();
        // only write to the log file when the Set method is called by a client and not when we're updating the sstables from the log fileΩ
        if(!from_log) LogData(data, key_size, value_size);
        ulock.lock();
        if (_memtableSize >= _MAX_MEMTABLE_SIZE) {
            _done_writing = false;
            _temp_memtable = std::move(_memtable);
            _memtableSize = 0;
            _memtable = std::map<Data, Data, Kora::Comparator>();
            _memtable_is_full = true;
            _update_is_from_logfile = from_log;
            ulock.unlock();
            _cond.notify_one();
            ulock.lock();
            _cond.wait(ulock, [this] { return _done_writing; });
            // delete log file since memtable has been successfully written to disk
            ClearLogFile();
        }
        return {};
    }
}

Kora::Result Kora::StorageEngine::Get(Kora::Data&& input_key) {
    /**
     * Convert key to char array
     * check the memtable first
     * start from the most recent segment, check for they key, continue until we run out of segments to check
     */
    std::lock_guard<std::mutex> lg(_mutex);
    Result r(Kora::Status::NotFound("key not found"));
    auto entry = _memtable.find(input_key);

    if (entry != _memtable.end())  {
        // record exists but has been deleted. Return not found status
        if (std::string(entry->second.data(), entry->second.size()).compare(Kora::StorageEngine::_TOMBSTONE_RECORD) == 0) return r;

        return Result(Kora::Status(), std::string(_memtable[input_key].data(), _memtable[input_key].size()));
    } else {
        for (auto& [key, value]: _sstables) {
            r = Search(input_key.data(), value, 0);
            if (r.status().isOk()) {
                // check if it has been deleted
                if (r.data().compare(Kora::StorageEngine::_TOMBSTONE_RECORD) == 0) {
                    return Result{Kora::Status::NotFound("Key not found")};
                }
                return r; // we have found the key
            }
        }
        return r;
    }
}

Kora::Result Kora::StorageEngine::Search(const char* key, std::string filepath, size_t start_offset, size_t end_offset) {
    std::ifstream segment {filepath, std::ios::binary};
    size_t key_size = 0, value_size = 0, total_size = 0, prev_total_size = 0, copy_range = 0, file_length = 0;
    std::string k, value;
    if (segment.good()) {
        segment.seekg(start_offset);
        while (!segment.eof()) {
            segment.seekg(total_size);
            segment.read(reinterpret_cast<char*>(&key_size), sizeof key_size);
            total_size += sizeof key_size;

            segment.read(reinterpret_cast<char*>(&value_size), sizeof value_size);
            total_size += sizeof value_size;

            k.resize(key_size);
            segment.read(&k[0],key_size);
            total_size += key_size;
            total_size += value_size;

            // don't bother comparing keys that are not of the same length
            if (k.size() != strlen(key)) {
                segment.get(); // let's know if we are at eof quick enough to avoid errors
                continue;
            }

            // continue as long as we have not found the key
            if (memcmp(k.data(), key, k.size()) != 0) {
                segment.get(); // let's know if we are at eof quick enough to avoid errors
                continue;
            }

            value.resize(value_size);
            segment.read(&value[0],value_size);

            return Result( Kora::Status::OK(), std::move(value));
        }
        Result r(Kora::Status::NotFound("Key not found"));
        return r;
    } else {
        // TODO: How should I handle when file no good
    }
}

Kora::Status Kora::StorageEngine::Delete(const Data&& key) {
    /**
     * Add a tombstone to the memtable and the logfile. During compaction, this will be used to delete the key-value entry
     */
    size_t key_size = key.size(), value_size = Kora::StorageEngine::_TOMBSTONE_RECORD.size();
    char data[key_size + value_size];
    {
        std::lock_guard<std::mutex> lg(_mutex);
        _memtable.insert(std::make_pair(key, Data(Kora::StorageEngine::_TOMBSTONE_RECORD.data())));
        strcpy(data, key.data());
        strcpy(&data[key_size], Kora::StorageEngine::_TOMBSTONE_RECORD.data());
    }
    LogData(data, key_size, value_size);
    return {};
}

void Kora::StorageEngine::LogData(const char* data, size_t key_size, size_t value_size) {
    auto path = Kora::getDBPath();
    path /= "log.kdb";
    std::ofstream logfile(path.string(), std::ios::binary | std::ios_base::app);
    if (logfile.is_open()) {
        logfile.write(reinterpret_cast<char*>(&key_size), sizeof(key_size));
        logfile.write(reinterpret_cast<char*>(&value_size), sizeof(value_size));
        logfile.write(data, strlen(data));
        logfile.close();
    }
}

[[noreturn]] void Kora::StorageEngine::Write() {
    while (true) {
        std::unique_lock<std::mutex> ulock(_mutex);
        _cond.wait(ulock, [this]{ return _memtable_is_full; });
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        auto path = Kora::getDBPath();
        path /= now() + ".sst";
        size_t total_size = 0;
        std::map<std::string, size_t> hash_index;
        // insert first entry into has index. We're starting from there
        hash_index.insert(std::make_pair(std::string(_temp_memtable.begin()->first.data()), 0));

        size_t starting_byte = 0;
        int count = 0;
        size_t key_size = _temp_memtable.begin()->first.size();
        size_t value_size = _temp_memtable.begin()->second.size();
        std::ofstream segment(path.string(), std::ios::binary);

        // if first record has not been deleted and is not from the log file, write first entry to sstable since we will be moving the iterator by 1
        size_t min_len = _temp_memtable.begin()->second.size() < Kora::StorageEngine::_TOMBSTONE_RECORD.size() ? _temp_memtable.begin()->second.size() : Kora::StorageEngine::_TOMBSTONE_RECORD.size();
        if (memcmp(_temp_memtable.begin()->second.data(), Kora::StorageEngine::_TOMBSTONE_RECORD.data(),min_len) == 0) {
            if (_update_is_from_logfile) { // even if entry is deleted as seen from log file, still write it so that the value in the SStable can be updated accordingly.
                segment.write(reinterpret_cast<char*>(&key_size), sizeof(_temp_memtable.begin()->first.size()));
                segment.write(reinterpret_cast<char*>(&value_size), sizeof(_temp_memtable.begin()->second.size()));
                segment.write(_temp_memtable.begin()->first.data(), key_size);
                segment.write(_temp_memtable.begin()->second.data(), value_size);
                // set starting byte to beginning of second entry since we will insert the first entry beforehand. The reason for inserting the first entry before hand is because we are making it an index, hence to made it easier to index the other necessary keys, we are excluding it from the iteration.
                starting_byte =  sizeof(size_t) + sizeof(size_t) + _temp_memtable.begin()->first.size() + _temp_memtable.begin()->second.size();
            }
        } else {
            //write normally if it's not deleted
            segment.write(reinterpret_cast<char*>(&key_size), sizeof(_temp_memtable.begin()->first.size()));
            segment.write(reinterpret_cast<char*>(&value_size), sizeof(_temp_memtable.begin()->second.size()));
            segment.write(_temp_memtable.begin()->first.data(), key_size);
            segment.write(_temp_memtable.begin()->second.data(), value_size);
            // set starting byte to beginning of second entry since we will insert the first entry beforehand. The reason for inserting the first entry before hand is because we are making it an index, hence to made it easier to index the other necessary keys, we are excluding it from the iteration.
            starting_byte =  sizeof(size_t) + sizeof(size_t) + _temp_memtable.begin()->first.size() + _temp_memtable.begin()->second.size();
        }

        if (segment.is_open()) {
            for(auto it = std::next(_temp_memtable.begin(), 1); it != _temp_memtable.end(); it++) {
                auto key = it->first;
                auto value = it->second;

                // skip if record has been deleted and is not from the log file
                min_len = value.size() < Kora::StorageEngine::_TOMBSTONE_RECORD.size() ? value.size() : Kora::StorageEngine::_TOMBSTONE_RECORD.size();
                if (!_update_is_from_logfile && memcmp(value.data(), Kora::StorageEngine::_TOMBSTONE_RECORD.data(), min_len) == 0) continue;

                total_size = sizeof key.data() + sizeof value.data() + key.size() + value.size();
                count += total_size;
                if (count >= _HASH_INDEX_INTERVAL) {
                    hash_index.insert(std::make_pair(std::string(key.data()), (count - total_size) + starting_byte));
                    starting_byte += count;
                    count = 0;
                }
                key_size = key.size();
                value_size = value.size();
                segment.write(reinterpret_cast<char*>(&key_size), sizeof(key.size()));
                segment.write(reinterpret_cast<char*>(&value_size), sizeof(value.size()));
                segment.write(key.data(), key_size);
                segment.write(value.data(), value_size);
            }
            segment.close();
            Kora::StorageEngine::StoreSegmentpath(getSegmentFileAsLong(path.filename()), path);
        }
        _temp_memtable.erase(_temp_memtable.begin(), _temp_memtable.end());
        _hash_indexes.insert(std::make_pair(path, hash_index));
        _done_writing = true;
        _memtable_is_full = false;
        _update_is_from_logfile = false;
        ulock.unlock();
        _cond.notify_one();
    }
}

void Kora::StorageEngine::Compact() {
    if (Kora::sstableCount() <= 1) return;
    auto db_path = Kora::getDBPath();

    while (Kora::sstableCount() >= 2) {

        long int f1_time_created = 0; // time_t is alias for long int
        long int f2_time_created = 0;
        size_t key_size = 0, value_size = 0, total_size = 0, prev_total_size = 0, copy_range = 0, file_length = 0;
        size_t key_size2 = 0, value_size2 = 0, total_size2 = 0, prev_total_size2 = 0, copy_range2 = 0, file_length2= 0;


        auto compactible_files = L1CompactibleFiles();
        if (compactible_files.size() < 2) {
            compactible_files = L2CompactibleFiles();
            if (compactible_files.size() < 2) compactible_files = L3CompactibleFiles();
            if (compactible_files.size() < 2) compactible_files = L4CompactibleFiles();
            if (compactible_files.size() < 2) break;
        }

        // initialize variables
        std::string key, value, key2, value2;

        // create new segment file
        auto new_segment_path = Kora::getDBPath();
        new_segment_path /= now() + ".sst";
        std::ofstream new_segment{ new_segment_path, std::ios::binary};

        // open the compactible files for reading
        std::ifstream file1 {compactible_files[0].filepath.string(), std::ios::binary};
        file_length = compactible_files[0].size;
        std::ifstream file2 {compactible_files[1].filepath.string(), std::ios::binary};
        file_length2 = compactible_files[1].size;

        std::string f1_filename = compactible_files[0].filepath.filename();
        f1_filename = f1_filename.substr(0, f1_filename.find_last_of('.'));
        std::string f2_filename = compactible_files[1].filepath.filename();
        f2_filename = f2_filename.substr(0, f2_filename.find_last_of('.'));
        f1_time_created = std::stol(f1_filename, nullptr, 10); // nullptr because the entire string will consist of digits
        f2_time_created = std::stol(f2_filename, nullptr, 10);

        while(!file1.eof() && !file2.eof()) {
            // prev_total_size will hold the next position to start reading the next record from
            size_t f1_pos = file1.tellg();
            size_t f2_pos = file2.tellg();

            prev_total_size = total_size;
            prev_total_size2 = total_size2;
            file1.seekg(prev_total_size); file2.seekg(prev_total_size2);
            file1.read(reinterpret_cast<char*>(&key_size), sizeof key_size);
            total_size += sizeof key_size;

            file2.read(reinterpret_cast<char*>(&key_size2), sizeof key_size2);
            total_size2 += sizeof key_size2;

            file1.read(reinterpret_cast<char*>(&value_size), sizeof value_size);
            total_size += sizeof value_size;

            file2.read(reinterpret_cast<char*>(&value_size2), sizeof value_size2);
            total_size2 += sizeof value_size2;

            key.resize(key_size);
            file1.read(&key[0],key_size);
            total_size += key_size;

            key2.resize(key_size2);
            file2.read(&key2[0],key_size2);
            total_size2 += key_size2;

            value.resize(value_size);
            file1.read(&value[0],value_size);
            total_size += value_size;

            value2.resize(value_size2);
            file2.read(&value2[0],value_size2);
            total_size2 += value_size2;

            int pos1 = file1.tellg();
            int pos2 = file2.tellg();

            auto diff = key.compare(key2);
            file1.seekg(prev_total_size); // go to beginning of current record
            file2.seekg(prev_total_size2);
            copy_range2 = total_size2 - prev_total_size2;
            copy_range = total_size - prev_total_size;
            if (diff > 0) {
                // key is greater than key2
                if (value2.compare(Kora::StorageEngine::_TOMBSTONE_RECORD) == 0) {
                    file1.close();
                    file2.close();
                    DiscardDeletedKey(key2, f2_time_created);
                    file1.seekg(0);
                    file2.seekg(0);
                    total_size = 0, total_size2 = 0, prev_total_size2 = 0, prev_total_size = 0;
                    file1 = std::ifstream {compactible_files[0].filepath.string(), std::ios::binary};
                    file2 = std::ifstream  {compactible_files[1].filepath.string(), std::ios::binary};
                    continue;
                }
                std::copy_n(std::istreambuf_iterator<char>(file2), copy_range2, std::ostreambuf_iterator<char>(new_segment));
                size_t n_pos = new_segment.tellp();
                file2.seekg(pos2);
                total_size = prev_total_size;
                file1.seekg(total_size);
            }
            else if (diff < 0) {
                // key is less than key2
                if (value.compare(Kora::StorageEngine::_TOMBSTONE_RECORD) == 0) {
                    file1.close();
                    file2.close();
                    DiscardDeletedKey(key, f1_time_created);
                    file1.seekg(0);
                    file2.seekg(0);
                    total_size = 0, total_size2 = 0, prev_total_size2 = 0, prev_total_size = 0;
                    file1 = std::ifstream {compactible_files[0].filepath.string(), std::ios::binary};
                    file2 = std::ifstream  {compactible_files[1].filepath.string(), std::ios::binary};
                    continue;
                }
                std::copy_n(std::istreambuf_iterator<char>(file1), copy_range, std::ostreambuf_iterator<char>(new_segment));
                file1.seekg(pos1);
                total_size2 = prev_total_size2;
                file2.seekg(total_size2);
            }
            else {
                // they are equal
                // pick the most recent
                std::string f1_filename = compactible_files[0].filepath.filename();
                f1_filename = f1_filename.substr(0, f1_filename.find_last_of('.'));
                std::string f2_filename = compactible_files[1].filepath.filename();
                f2_filename = f2_filename.substr(0, f2_filename.find_last_of('.'));
                f1_time_created = std::stol(f1_filename, nullptr, 10); // nullptr because the entire string will consist of digits
                f2_time_created = std::stol(f2_filename, nullptr, 10);
                if (f1_time_created > f2_time_created) {
                    if (value.compare(Kora::StorageEngine::_TOMBSTONE_RECORD) == 0) {
                        file1.close();
                        file2.close();
                        DiscardDeletedKey(key, f1_time_created);
                        file1.seekg(0);
                        file2.seekg(0);
                        total_size = 0, total_size2 = 0, prev_total_size2 = 0, prev_total_size = 0;
                        file1 = std::ifstream {compactible_files[0].filepath.string(), std::ios::binary};
                        file2 = std::ifstream  {compactible_files[1].filepath.string(), std::ios::binary};
                        continue;
                    }
                    std::copy_n(std::istreambuf_iterator<char>(file1), copy_range, std::ostreambuf_iterator<char>(new_segment));
                    file1.seekg(pos1);
                    file2.seekg(total_size2);
                }
                else {
                    if (value2.compare(Kora::StorageEngine::_TOMBSTONE_RECORD) == 0) {
                        file1.close();
                        file2.close();
                        DiscardDeletedKey(key2, f2_time_created);
                        file1.seekg(0);
                        file2.seekg(0);
                        total_size = 0, total_size2 = 0, prev_total_size2 = 0, prev_total_size = 0;
                        file1 = std::ifstream {compactible_files[0].filepath.string(), std::ios::binary};
                        file2 = std::ifstream  {compactible_files[1].filepath.string(), std::ios::binary};
                        continue;
                    }
                    std::copy_n(std::istreambuf_iterator<char>(file2), copy_range2, std::ostreambuf_iterator<char>(new_segment));
                    file2.seekg(pos2);
//                    total_size = prev_total_size;
                    file1.seekg(total_size);
                }
            }
            // go to position total_size + 1
            file1.get(); // do this so that eof can be set appropriately. see https://en.cppreference.com/w/cpp/io/basic_istream/get
            file2.get();
        }
        // copy over any of the leftovers. This may copy over some deleted entries. it's no problem as deleted records are checked on Get() and such records would eventually be removed by compaction in the future.
        if (!file1.eof()) {
            file1.seekg(total_size); // because .get() has moved it one byte already
            std::copy_n(std::istreambuf_iterator<char>(file1), file_length - total_size, std::ostreambuf_iterator<char>(new_segment));
        }
        if (!file2.eof()) {
            file2.seekg(total_size2);  // because .get() has moved it one byte already
            std::copy_n(std::istreambuf_iterator<char>(file2), file_length2 - total_size2, std::ostreambuf_iterator<char>(new_segment));
        }
        new_segment.seekp(0);
        new_segment.close();
        // store new segment for easy retrieval
        Kora::StorageEngine::StoreSegmentpath(getSegmentFileAsLong(new_segment_path.filename()), new_segment_path);

        // delete all references to already compacted files
        Kora::StorageEngine::DeleteSegmentpath(getSegmentFileAsLong(compactible_files[0].filepath.filename()));
        fs::remove(compactible_files[0].filepath);
        RemoveIndex(compactible_files[0].filepath);
        Kora::StorageEngine::DeleteSegmentpath(getSegmentFileAsLong(compactible_files[1].filepath.filename()));
        RemoveIndex(compactible_files[1].filepath);
        fs::remove(compactible_files[1].filepath);

        // create index
        CreateIndexFromCompactedSegment(new_segment_path.string());
    }
}

void Kora::StorageEngine::CreateIndexFromCompactedSegment(std::string filepath) {
    std::ifstream file1 {filepath, std::ios::binary};

    size_t key_size = 0, value_size = 0, count = 0, starting_byte = 0, prev_total_size = 0, first = 0;

    std::string key, value, key2, value2;

    std::map<std::string, size_t> hash_index;

    while(!file1.eof()) {
        file1.seekg(prev_total_size);
        size_t total_size = 0;
        size_t f1_pos = file1.tellg();

        file1.read(reinterpret_cast<char*>(&key_size), sizeof key_size);
        total_size += sizeof key_size;

        file1.read(reinterpret_cast<char*>(&value_size), sizeof value_size);
        total_size += sizeof value_size;

        key.resize(key_size);
        file1.read(&key[0],key_size);
        total_size += key_size;

        value.resize(value_size);
        file1.read(&value[0],value_size);
        total_size += value_size;
        count += total_size;

        // index first entry
        if (first == 0) {
            hash_index.insert(std::make_pair(key,
                                             starting_byte));
            starting_byte += count;
            prev_total_size = count;
            count = 0;
            ++first;
            file1.get(); // do this so that eof can be set appropriately. see https://en.cppreference.com/w/cpp/io/basic_istream/get
            continue;
        }

        // index next x byte/kb
        if (count >= 50) {
            hash_index.insert(std::make_pair(key, starting_byte));
            prev_total_size += total_size;
            starting_byte += total_size;
            count = 0;
            file1.get(); // do this so that eof can be set appropriately. see https://en.cppreference.com/w/cpp/io/basic_istream/get
            continue;
        }
        // go to position total_size + 1
        file1.get(); // do this so that eof can be set appropriately. see https://en.cppreference.com/w/cpp/io/basic_istream/get
        starting_byte += count;
        prev_total_size += count;
    }
    _hash_indexes.insert(std::make_pair(filepath, hash_index));
}

std::vector<Kora::CompactibleObject> Kora::StorageEngine::L1CompactibleFiles() {
    auto db_path = Kora::getDBPath();
    std::vector<Kora::CompactibleObject> result;
    short int file_count = 0;
    uintmax_t total_size = 0;
    for (auto const& dir_entry: fs::directory_iterator{db_path}) {
        if (dir_entry.exists() && dir_entry.is_regular_file()) {
            auto ext = dir_entry.path().extension().string();
            if (ext == ".sst") {
                if (dir_entry.file_size() >= _MAX_MEMTABLE_SIZE && dir_entry.file_size() <= _MAX_LEVEL1_SIZE) {
                    if (file_count == 0) {
                        CompactibleObject cobj = {dir_entry.path().string(), dir_entry.file_size()};
                        total_size = cobj.size;
                        result.push_back(cobj);
                        ++file_count;
                    } else {
                        CompactibleObject cobj = {dir_entry.path().string(), dir_entry.file_size()};
                        if (std::find(result.begin(), result.end(), cobj) != result.end()) continue;
                        result.push_back(cobj);
                        ++file_count;
                        break;
                    }
                }
            } else continue;
        }
    }
    return result;
}

std::vector<Kora::CompactibleObject> Kora::StorageEngine::L2CompactibleFiles() {
    auto db_path = Kora::getDBPath();
    std::vector<Kora::CompactibleObject> result;
    short int file_count = 0;
    for (auto const& dir_entry: fs::directory_iterator{db_path}) {
        if (dir_entry.exists() && dir_entry.is_regular_file()) {
            auto ext = dir_entry.path().extension().string();
            if (ext == ".sst") {
                if (dir_entry.file_size() >= _MAX_LEVEL1_SIZE + 1 && dir_entry.file_size() <= _MAX_LEVEL2_SIZE) {
                    if (file_count == 0) {
                        CompactibleObject cobj = {dir_entry.path().string(), dir_entry.file_size()};
                        result.push_back(cobj);
                        ++file_count;
                    } else {
                        CompactibleObject cobj = {dir_entry.path().string(), dir_entry.file_size()};
                        if (std::find(result.begin(), result.end(), cobj) != result.end()) continue;
                        result.push_back(cobj);
                        ++file_count;
                        break;
                    }
                }
            } else continue;
        }
    }
    return result;
}

std::vector<Kora::CompactibleObject> Kora::StorageEngine::L3CompactibleFiles() {
    auto db_path = Kora::getDBPath();
    std::vector<Kora::CompactibleObject> result;
    short int file_count = 0;
    for (auto const& dir_entry: fs::directory_iterator{db_path}) {
        if (dir_entry.exists() && dir_entry.is_regular_file()) {
            auto ext = dir_entry.path().extension().string();
            if (ext == ".sst") {
                if (dir_entry.file_size() >= _MAX_LEVEL2_SIZE + 1 && dir_entry.file_size() <= _MAX_LEVEL3_SIZE) {
                    if (file_count == 0) {
                        CompactibleObject cobj = {dir_entry.path().string(), dir_entry.file_size()};
                        result.push_back(cobj);
                        ++file_count;
                    } else {
                        CompactibleObject cobj = {dir_entry.path().string(), dir_entry.file_size()};
                        if (std::find(result.begin(), result.end(), cobj) != result.end()) continue;
                        result.push_back(cobj);
                        ++file_count;
                        break;
                    }
                }
            } else continue;
        }
    }
    return result;
}

std::vector<Kora::CompactibleObject> Kora::StorageEngine::L4CompactibleFiles() {
    auto db_path = Kora::getDBPath();
    std::vector<Kora::CompactibleObject> result;
    short int file_count = 0;
    for (auto const& dir_entry: fs::directory_iterator{db_path}) {
        if (dir_entry.exists() && dir_entry.is_regular_file()) {
            auto ext = dir_entry.path().extension().string();
            if (ext == ".sst") {
                if (dir_entry.file_size() >= _MIN_LEVEL4_SIZE) {
                    if (file_count == 0) {
                        CompactibleObject cobj = {dir_entry.path().string(), dir_entry.file_size()};
                        result.push_back(cobj);
                        ++file_count;
                    } else {
                        CompactibleObject cobj = {dir_entry.path().string(), dir_entry.file_size()};
                        if (std::find(result.begin(), result.end(), cobj) != result.end()) continue;
                        result.push_back(cobj);
                        ++file_count;
                        break;
                    }
                }
            } else continue;
        }
    }
    return result;
}

void Kora::StorageEngine::BuildSSTableMap() {
    auto db_path = Kora::getDBPath();
    if (!fs::exists(db_path)) return;
    for (auto const& dir_entry: fs::directory_iterator{db_path}) {
        if (dir_entry.exists() && dir_entry.is_regular_file()) {
            auto ext = dir_entry.path().extension().string();
            if (ext == ".sst") {
                long filename = Kora::getSegmentFileAsLong(dir_entry.path().filename());
                _sstables.insert(std::make_pair(filename, dir_entry.path().string()));
            } else continue;
        }
    }
}


void Kora::StorageEngine::BuildIndexes() {
    auto db_path = Kora::getDBPath();
    if (!fs::exists(db_path)) return;
    for (auto const& dir_entry: fs::directory_iterator{db_path}) {
        if (dir_entry.exists() && dir_entry.is_regular_file()) {
            auto ext = dir_entry.path().extension().string();
            if (ext == ".sst") {
                CreateIndexFromCompactedSegment(dir_entry.path().string());
            } else continue;
        }
    }
}

/**
 * This method reads the log file, writes each entry to a memtable which would eventually be written out to disk and compacted, hence updating the records.
 */
void Kora::StorageEngine::UpdateSSTablesFromLogFile(StorageEngine *SE) {
    auto path = Kora::getDBPath();
    path /= "log.kdb";
    if(!fs::exists(path)) return;
    std::ifstream file {path, std::ios::binary};

    size_t key_size = 0, value_size = 0, count = 0, starting_byte = 0, total_size = 0, first = 0;

    std::string key, value, key2, value2;

    std::map<std::string, size_t> hash_index;

    if (file.good()) {
        while(!file.eof()) {
            file.seekg(total_size);
            size_t f1_pos = file.tellg();

            file.read(reinterpret_cast<char*>(&key_size), sizeof key_size);
            total_size += sizeof key_size;

            file.read(reinterpret_cast<char*>(&value_size), sizeof value_size);
            total_size += sizeof value_size;

            key.resize(key_size);
            file.read(&key[0],key_size);
            total_size += key_size;

            value.resize(value_size);
            file.read(&value[0],value_size);
            total_size += value_size;
            SE->Set(Data(std::move(key)), Data(std::move(value)), true);
            file.get();
        }
    }
    file.close();
    Kora::StorageEngine::_done_updating_sstables = true;
    // Clear Log file after updating
    // ClearLogFile();
}

void Kora::StorageEngine::DiscardDeletedKey(std::string input_key, long most_recent_filename) {
    try {
        for (const auto& [filename, filepath]: Kora::StorageEngine::_sstables) {
            if (filename > most_recent_filename) continue;

            if(!fs::exists(filepath)) return;

            std::ifstream file {filepath, std::ios::binary};

            // create temp file
            fs::path temp_file_path { filepath.substr(0, filepath.find_last_of(".")) + "_temp.sst"};
            std::ofstream temp_file {temp_file_path, std::ios::binary};
            if (file.good() && temp_file.good()) {
                // store contents of file here temporarily
                std::copy_n(std::istreambuf_iterator<char>(file), fs::file_size(filepath), std::ostreambuf_iterator<char>(temp_file));
                temp_file.close();
            }

            size_t key_size = 0, value_size = 0, count = 0, starting_byte = 0, total_size = 0, prev_total_size = 0, first = 0;

            std::string key, value, key2, value2;

            std::map<std::string, size_t> hash_index;

            while(!file.eof()) {
                file.seekg(prev_total_size);
                prev_total_size = total_size;
                file.seekg(prev_total_size);
                size_t f1_pos = file.tellg();

                file.read(reinterpret_cast<char*>(&key_size), sizeof key_size);
                total_size += sizeof key_size;

                file.read(reinterpret_cast<char*>(&value_size), sizeof value_size);
                total_size += sizeof value_size;

                key.resize(key_size);
                file.read(&key[0],key_size);
                total_size += key_size;

                value.resize(value_size);
                file.read(&value[0],value_size);
                total_size += value_size;
                // if deleted, remove from file
                if (key.compare(input_key) == 0) {
                    // resize file from 0 to prev_total_size
                    file.close();
                    {
                        std::ofstream f{filepath, std::ios::binary}; // open main file in output mode
                        std::ifstream tempf{temp_file_path, std::ios::binary}; // open temp file in input mode
                        fs::resize_file(filepath, prev_total_size); // resize file to the total size before the deleted entry
                        std::copy_n(std::istreambuf_iterator<char>(tempf), prev_total_size, std::ostreambuf_iterator<char>(f));
                        tempf.seekg(total_size); // seek to the next entry after the deleted entry
                        if (total_size < fs::file_size(temp_file_path)) std::copy_n(std::istreambuf_iterator<char>(tempf), fs::file_size(temp_file_path) - total_size, std::ostreambuf_iterator<char>(f));
                        tempf.close();
                        break;
                    }
                    file = std::ifstream {filepath, std::ios::binary};
                    file.seekg(prev_total_size);
                }
                file.get();
            }
            file.seekg(0);
            file.close();
            fs::remove(temp_file_path);
        }
    } catch (...) {
        std::cout << " Error ocurred with deleting tombstones\n";
    }
}

void Kora::StorageEngine::ClearLogFile() {
    static int count;
    auto path = getDBPath();
    path /= "log.kdb";
    try {
        fs::resize_file(path, 0);
    } catch (fs::filesystem_error const& ex) {
        // TODO: FIgure out how to handle this better
        std::cout << "Error clearing log file\n";
    }
}
