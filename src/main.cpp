#include <iostream>
#include <fstream>
#include <map>
#include "../include/db.h"

int main() {
//    std::cout << "Hello, World!" << std::endl;
//
//    std::multimap<std::string, std::string> test;
//    test.insert(std::make_pair("joshua", "Boateng"));
//    test.insert(std::make_pair("joba", "Oluwabori"));
//    test.insert(std::make_pair("joba", "Adesanya"));
//    test.insert(std::make_pair("damilare", "Ibiyemi"));
//
//    for (auto &pair: test) {
//        std::cout << pair.first << '\n';
//    }
//
//    std::string name = "Jake";
//    char *t = &name[0];
//    std::ofstream file("test.bin", std::ios::binary);
//    file.write(reinterpret_cast<char*>(&t), name.size());
//    file.close();
    std::cout << "Hello from: " << std::this_thread::get_id() << '\n';
    Kora::DB db;
    Kora::Status status = db.Set("kamsy", "04:03:01");
    db.Set("james", "08:09:10");
    if (status.isOk()) std::cout << "Success!\n";
    else std::cout << status.message() << '\n';
    Kora::Result result = db.Get("kamsy");
    if (result.status().code() == Kora::Code::_OK) {
        std::cout << result.data() << '\n';
    } else {
        std::cout << result.status().message() << '\n';
    }
}
