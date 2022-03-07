//
// Created by kwaku on 15/02/2022.
//

#ifndef KV_STORE_HELPER_H
#define KV_STORE_HELPER_H

#include <filesystem>
#include <iostream>
#include <chrono>
#include <ctime>
#include <fstream>
#include <sstream>

namespace fs = std::filesystem;

namespace Kora {
    struct CompactibleObject {
        fs::path filepath;
        uintmax_t size;
    };

    inline bool operator==(const CompactibleObject &c1, const CompactibleObject &c2) {
        return c1.filepath.string().compare(c2.filepath.string()) == 0;
    }

    inline void createDBDirectory() {
        fs::path p = fs::current_path();
        auto parent_path = p.parent_path();
        auto db_path = parent_path /= "db";
        if (!fs::exists(db_path)) {
            if (!fs::create_directory(db_path)) {
                exit(1);
            }
        }
    }

    inline void createDir(fs::path&& dir_path) {
        if (!fs::exists(dir_path)) {
            if (!fs::create_directory(dir_path)) {
                exit(1);
            }
        }
    }

    inline fs::path getDBPath() {
        fs::path p = fs::current_path();
        auto parent_path = p.parent_path();
        return parent_path /= "db";
    }

    inline std::string now() {
        static int count = 0;
        auto result = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
        std::stringstream ss;
        ss << result;
        return ss.str();
    }

    inline size_t fileLength(std::ifstream& file) {
        file.seekg(0, file.end);
        size_t length = file.tellg();
        file.seekg(0, file.beg);
        return length;
    }

    inline int sstableCount() {
        int count = 0;
        auto path = getDBPath();
        if (!fs::exists(path)) return count;
        for(auto const& dir_entry: fs::directory_iterator{path}) {
            if (dir_entry.exists() && dir_entry.is_regular_file()) {
                auto ext = dir_entry.path().extension().string();
                if (ext == ".sst") ++count;
            }
        }
        return count;
    }

    inline long getSegmentFileAsLong(fs::path filename) {
        std::string filename_str = filename.string();
        filename_str = filename_str.substr(0, filename_str.find_last_of('.'));
        long result =  std::stol(filename_str, nullptr, 10);
        return result;
    }
}

#endif //KV_STORE_HELPER_H
