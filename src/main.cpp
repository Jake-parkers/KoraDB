#include <iostream>
#include <fstream>
#include <map>

int main() {
    std::cout << "Hello, World!" << std::endl;

    std::multimap<std::string, std::string> test;
    test.insert(std::make_pair("joshua", "Boateng"));
    test.insert(std::make_pair("joba", "Oluwabori"));
    test.insert(std::make_pair("joba", "Adesanya"));
    test.insert(std::make_pair("damilare", "Ibiyemi"));

    for (auto &pair: test) {
        std::cout << pair.first << '\n';
    }

    std::string name = "Jake";
    char *t = &name[0];
    std::ofstream file("test.bin", std::ios::binary);
    file.write(reinterpret_cast<char*>(&t), name.size());
    file.close();
}
