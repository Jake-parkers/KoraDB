//
// Created by kwaku on 15/02/2022.
//

#ifndef KV_STORE_HELPER_H
#define KV_STORE_HELPER_H

#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

namespace Kora {
    inline void createDBDirectory() {
        fs::path p = fs::current_path();
        auto parent_path = p.parent_path();
        auto db_path = parent_path /= "db";
        std::cout << db_path << '\n';
        if (!fs::exists(db_path)) {
            if (!fs::create_directory(db_path)) {
                std::cout << "Error creating database directory\n";
                exit(1);
            }
        }
    }

    inline fs::path getDBPath() {
        fs::path p = fs::current_path();
        auto parent_path = p.parent_path();
        return parent_path /= "db";
    }
}

#endif //KV_STORE_HELPER_H
