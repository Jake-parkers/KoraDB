//
// Created by Joshua Kwaku on 13/02/2022.
//

#ifndef KV_STORE_DB_H
#define KV_STORE_DB_H

#include "options.h"
#include "status.h"

#include <string>
#include <map>

namespace Kora {
    template <typename T>
    class DB {
    public:
        DB(): _filename{"/tmp/data.db"} {}

        DB(Options options, std::string db_filename): _filename{db_filename}, _dbOptions{options} {}

        Status Get(std::string key);

        Status Set(std::string key, T value);

        Status Delete(std::string key);

        void Write() {}

    private:
        std::string _filename;
        Options _dbOptions;
        std::map<std::string, char*> _memtable;
    };
}


#endif //KV_STORE_DB_H
