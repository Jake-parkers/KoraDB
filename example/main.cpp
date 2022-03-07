#include <iostream>
#include "koradb/kdb.h"

int main() {
    std::cout << "Hello, World!" << std::endl;
    Kora::DB db;
    Kora::Status result = db.Set("name", "kwaku");
    if (result.isOk()) {
        std::cout << "Key inserted successfully\n";
    }
    auto key = db.Get("name");
    if (key.status().isOk()) {
        std::cout << key.data() << '\n';
    } else {
        std::cout << key.status().toString() << '\n';
    }
    return 0;
}
