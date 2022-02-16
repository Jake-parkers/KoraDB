//
// Created by Joshua Kwaku on 13/02/2022.
//

#ifndef KV_STORE_STORAGE_ENGINE_H
#define KV_STORE_STORAGE_ENGINE_H

#include <map>
#include "status.h"
#include "result.h"
#include "data.h"

namespace Kora {
    class StorageEngine {
    public:
        Kora::Status Set(Data&& key, Data&& value) noexcept;
        Kora::Result Get(const Data& key);
        Kora::Status Delete(const char* key);
        static void LogData(const char* data, size_t key_size, size_t value_size);
    private:
        std::map<Data, Data, std::less<>> _memtable;
        size_t _memtableSize;
    };

}

#endif //KV_STORE_STORAGE_ENGINE_H

//#include "../src/storage_engine.cpp";