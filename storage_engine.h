//
// Created by Joshua Kwaku on 13/02/2022.
//

#ifndef KV_STORE_STORAGE_ENGINE_H
#define KV_STORE_STORAGE_ENGINE_H

#include <map>
#include <unordered_map>
#include "status.h"
#include "result.h"
#include "data.h"
#include "timer.h"
#include <thread>
#include <condition_variable>
#include <mutex>
#include "helper.h"
#include <limits.h>
namespace Kora {
    class StorageEngine {
    public:
        StorageEngine() {
            // build the _sstable map allover once the storage engine starts
            BuildSSTableMap();

            // setup sparese hash index
            //BuildIndexes();

            if(!_writerThread.joinable())
                _writerThread = std::thread(&StorageEngine::Write, this);

            UpdateSSTablesFromLogFile(this);

            _timer.start(10000, Compact);

        }
        Kora::Status Set(Data&& key, Data&& value, bool from_log=false) noexcept;
        Kora::Result Get(Data&& key);
        Kora::Status Delete(const Data&& key);
        static void LogData(const char* data, size_t key_size, size_t value_size);


        ~StorageEngine(){
            _writerThread.join();
        }

    private:
        std::map<Data, Data, Kora::Comparator> _memtable;
        std::map<Data, Data, Kora::Comparator> _temp_memtable;
        static std::map<long, std::string, std::greater<>> _sstables; // filename -> fullpath
        size_t _memtableSize = 0;
        std::thread _writerThread;
        static std::string _TOMBSTONE_RECORD;
        std::condition_variable _cond;
        std::mutex _mutex;
        bool _memtable_is_full = false;
        bool _update_is_from_logfile = false;
        bool _done_writing = false;
        static bool _done_updating_sstables;
        const static long long _MAX_SST_SIZE = 1024;
        Timer _timer;
        static std::vector<CompactibleObject> L1CompactibleFiles(); // ssts between 50bytes and 100bytes
        static std::vector<CompactibleObject> L2CompactibleFiles(); // ssts between 101bytes and 300bytes
        static std::vector<CompactibleObject> L3CompactibleFiles(); // ssts between 301bytes and 500bytes
        static std::vector<CompactibleObject> L4CompactibleFiles(); // ssts between 501bytes and 1000bytes

        // write out memtable to sstable
        [[noreturn]] void Write();

        // compact memtable
        static void Compact();

        /**
         * This functions sets the log file to zero bytes. Called only when a memtable has been written successfully to an sstable on disk
         */
        static void ClearLogFile();

        static void CreateIndexFromCompactedSegment(std::string);

        static void StoreSegmentpath(long filename, std::string filepath) {
            Kora::StorageEngine::_sstables.insert(std::make_pair(filename, filepath));
        }

        static void DeleteSegmentpath(long filename) {
            Kora::StorageEngine::_sstables.erase(filename);
        }

        static void RemoveIndex(std::string filepath) {
            Kora::StorageEngine::_hash_indexes.erase(filepath);
        }

        // keep in-memory index of all segments. filepath->index
        static std::unordered_map<std::string, std::map<std::string, size_t>> _hash_indexes;

        /**
         *
         * @param key - the key we're searching for
         * @param filepath
         * @param start_offset
         * @param end_offset
         * @return
         */
        static Result Search(const char* key, std::string filepath, size_t start_offset, size_t end_offset = SIZE_MAX);

        /**
         * When DB is started build an in-memory cache of all sstables from most-recent to least-recent. The cache helps speed up the process of looking for a key as we already know where to start looking from and where to end
         */
        static void BuildSSTableMap();

        /**
         * When DB is started, create a sparse hash index for all sstables to speed up search.. TBD
         */
        static void BuildIndexes();

        /***
         * WHen DB restarts, load all non-persisted data to the memtable for them to eventually be written to disk
         * @param SE - Pointer to the Storage Engine instance
         */
        static void UpdateSSTablesFromLogFile(StorageEngine *SE);

        static void DiscardDeletedKey(std::string, long);
    };

}

#endif //KV_STORE_STORAGE_ENGINE_H

//#include "../src/storage_engine.cpp";