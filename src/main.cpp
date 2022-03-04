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
//    Kora::Status status = db.Set("adeleke", "gabriel");
//    db.Set("awomuti", "Kehinde");
//    db.Set("ralf", "ragnick");
//    db.Set("micheal", "owen");
//    db.Delete("awomuti");
//    db.Set("paul", "scholes");
//    db.Set("owen", "hagreaves");
//    db.Set("bruno", "fernandes");
//    db.Set("cristiano", "ronaldo");
//    db.Set("edinson", "cavani");
//    db.Set("david", "degea");
//    db.Set("luke", "shaw");
//    db.Set("diogo", "dalot");
//    db.Set("scott", "mctominay");
//    if (status.isOk()) std::cout << "Success!\n";
//    else std::cout << status.message() << '\n';

//    Kora::Result result = db.Get("aaron");
//    if (result.status().isOk()) {
//        std::cout << result.data() << '\n';
//    } else {
//        std::cout << result.status().message() << '\n';
//    }
    //std::cout << "DATAXX: " << db.Get("awomuti").status().message() << "\n";
//    std::cout << db.Get("mesut").status().message() << "\n";
//
//    db.Set("jimmy", "garner");
//    db.Set("neymar", "dasilva");
//    db.Set("dani", "alves");
//    db.Set("thomas", "muellar");
//   db.Set("manuel", "nueuer");
//    db.Set("kepa", "arrizabalaga");
//    db.Delete("edinson");
//    std::cout << "=====================================================\n";
//    std::cout << "XXXRALF: " << db.Get("edinson").data() << "\n";
//    std::cout << "XXXTHOMAS: " << db.Get("edinson").status().message() << "\n";
//    std::cout << db.Get("fred").data() << "\n";
//    db.Delete("fred");
//    std::cout << db.Get("fred").status().message() << "\n";

}
