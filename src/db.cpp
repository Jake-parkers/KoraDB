//
// Created by Joshua Kwaku on 13/02/2022.
//
#include <iostream>
#include <utility>

#include "../include/db.h"
#include "../include/data.h"

Kora::Status Kora::DB::Set(std::string key, std::string value) {
//    const char* k = &key[0];
//    const char* v = &value[0];
    return _storage_engine.Set(Data(std::move(key)), Data(std::move(value)));
}

Kora::Result Kora::DB::Get(const std::string& key) {
    return _storage_engine.Get(Data(key));
}
