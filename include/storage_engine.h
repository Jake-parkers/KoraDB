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
            if(!_writerThread.joinable())
                _writerThread = std::thread(&StorageEngine::Write, this);

            if(!_compactionThread.joinable())
                _compactionThread = std::thread(&StorageEngine::Compact, this);
        }
        Kora::Status Set(Data&& key, Data&& value) noexcept;
        Kora::Result Get(const Data& key);
        Kora::Status Delete(const char* key);
        static void LogData(const char* data, size_t key_size, size_t value_size);

        size_t memtableSize() { return _memtableSize; }

        ~StorageEngine(){
            _writerThread.join();
            _compactionThread.join();
        }

        // write out memtable to sstable
        Kora::Status Write();

        // compact memtable
        Kora::Status Compact();
    private:
        std::map<Data, Data, std::less<>> _memtable;
        std::map<Data, Data, std::less<>> _temp_memtable;
        size_t _memtableSize = 0;
        std::thread _writerThread;
        std::thread _compactionThread;
        std::condition_variable _cond;
        std::condition_variable _compaction_cond;
        std::mutex _mutex;
        bool _memtable_is_full = false;
    };

}

#endif //KV_STORE_STORAGE_ENGINE_H

//#include "../src/storage_engine.cpp";