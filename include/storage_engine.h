//
// Created by Joshua Kwaku on 13/02/2022.
//

#ifndef KV_STORE_STORAGE_ENGINE_H
#define KV_STORE_STORAGE_ENGINE_H

#include <map>
#include "status.h"

namespace Kora {
    class StorageEngine {
    public:
        Kora::Status Set(const char* key, const char* value);
        Kora::Status Get(const char* key);
        Kora::Status Delete(const char* key);
    private:
        std::map<const char*, const char*> _memtable;
        size_t _memtableSize;
    };

}

#endif //KV_STORE_STORAGE_ENGINE_H

//#include "../src/storage_engine.cpp";