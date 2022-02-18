//
// Created by Joshua Kwaku on 13/02/2022.
//

#ifndef KV_STORE_STORAGE_ENGINE_H
#define KV_STORE_STORAGE_ENGINE_H

#include <map>
#include "status.h"
#include "result.h"
#include "data.h"
#include <thread>
#include <condition_variable>
#include <mutex>

namespace Kora {
    class StorageEngine {
    public:
        StorageEngine() {
            if(!_t.joinable())
                _t = std::thread(&StorageEngine::Write, this);
        }
        Kora::Status Set(Data&& key, Data&& value) noexcept;
        Kora::Result Get(const Data& key);
        Kora::Status Delete(const char* key);
        static void LogData(const char* data, size_t key_size, size_t value_size);

        ~StorageEngine(){
            _t.join();
        }

        // write out memtable to sstable
        Kora::Status Write();
    private:
        std::map<Data, Data, std::less<>> _memtable;
        size_t _memtableSize = 0;
        std::thread _t;
        std::condition_variable _cond;
        std::mutex _mutex;
        bool _memtable_is_full = false;
    };

}

#endif //KV_STORE_STORAGE_ENGINE_H

//#include "../src/storage_engine.cpp";