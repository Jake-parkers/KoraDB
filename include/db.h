//
// Created by Joshua Kwaku on 13/02/2022.
//

#ifndef KV_STORE_DB_H
#define KV_STORE_DB_H

#include "status.h"
#include "result.h"
#include "storage_engine.h"
#include "helper.h"
#include "options.h"

#include <string>
#include <map>
#include <utility>
#include <filesystem>

namespace fs = std::filesystem;
namespace Kora {
    class DB {
    public:
        DB(): _filename{"/tmp/data.db"} {
            createDBDirectory();
        }

        explicit DB(std::string db_filename): _filename{std::move(db_filename)} {
            createDBDirectory();
        }

        DB(Options options, std::string db_filename): _filename{std::move(db_filename)}, _dbOptions{options} {
            createDBDirectory();
        }

        Result Get(const std::string& key);

        Status Set(const std::string& key, const std::string& value);

        Status Delete(std::string key);

        void Write() {}

    private:
        std::string _filename;
        Options _dbOptions{};
        StorageEngine _storage_engine;
        std::thread t;
    };
}


#endif //KV_STORE_DB_H
