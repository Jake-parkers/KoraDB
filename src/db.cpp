//
// Created by Joshua Kwaku on 13/02/2022.
//
#include <iostream>

#include "../include/db.h"
#include "../include/data.h"

Kora::Status Kora::DB::Set(const std::string& key, const std::string& value) {
//    const char* k = &key[0];
//    const char* v = &value[0];
    return _storage_engine.Set(Data(key), Data(value));
}

Kora::Result Kora::DB::Get(const std::string& key) {
    return _storage_engine.Get(Data(key));
}
