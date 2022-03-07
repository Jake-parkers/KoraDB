//
// Created by Joshua Kwaku on 13/02/2022.
//
#include <iostream>
#include <utility>

#include "../include/kdb.h"
Kora::Status Kora::DB::Set(std::string key, std::string value) {
    return _storage_engine.Set(Data(std::move(key)), Data(std::move(value)));
}

Kora::Result Kora::DB::Get(std::string key) {
    return _storage_engine.Get(Data(key));
}

Kora::Status Kora::DB::Delete(std::string key) {
    return _storage_engine.Delete(Data(key));
}