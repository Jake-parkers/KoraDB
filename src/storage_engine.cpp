//
// Created by Joshua Kwaku on 13/02/2022.
//

#include "storage_engine.h"
#include <cstring>
#include <fstream>

Kora::Status Kora::StorageEngine::Set(const char* key, const char* value) {
    /**
     * Convert key and value to char array
     * insert them into the memtable
     * update the memtable approx size
     * add the new data to the log file
     */
    _memtable.insert(std::make_pair(key, value));
    _memtableSize += strlen(key) + strlen(value);
}

Kora::Status Kora::StorageEngine::Get(std::string key) {
    /**
     * Convert key to char array
     * check the memtable first
     * if not there then recursively check the sstables
     */
    char *k = &key[0];
    if (_memtable.find(k) != _memtable.end())  {

    } else {
        return Kora::Status::NotFound("Key does not exist");
    }
}