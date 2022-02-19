//
// Created by kwaku on 16/02/2022.
//

#ifndef KV_STORE_DATA_H
#define KV_STORE_DATA_H

#include <cstring>
#include <string>
#include <iostream>

namespace Kora {
    class Data {
    public:
        Data() = default;
        explicit Data(char* data) : _data{data}, _size(strlen(data)) {}
        Data(char* data, size_t size) : _data{data}, _size(size) {}
        Data(std::string& str, size_t size) : _data{str.data()}, _size(size) {}
        explicit Data(std::string str) : _data{str.data()}, _size(str.size()) {}

        // copy constructor
        Data(const Data& other): _data {nullptr}, _size {0} {
            std::cout << "Copy constructor called\n";
            _data = (char *) malloc(other._size + 1);
            strcpy(_data, other._data);
            _size = other._size;
        }
        // copy assignment
        Data& operator=(const Data&) = default;

        // move constructor
        Data(Data&& that) noexcept: _data{nullptr}, _size{0} {
            std::cout << "Move constructor called\n";
            std::swap(_data, that._data);
            std::swap(_size, that._size);
        }

        //getters
        // return a pointer to the first character of the data
        [[nodiscard]] const char* data() const { return _data; }

        // return the length of the referenced data in bytes
        [[nodiscard]] size_t size() const { return _size; }
    private:
        char* _data = nullptr;
        size_t _size;
    };

    inline bool operator<(const Data& d1, const Data& d2) {
        if (d1.size() != d2.size()) return true;
        int r = memcmp(d1.data(), d2.data(), d1.size());
        if (r == 0)
            return false;
        else return true;
    }
}

#endif //KV_STORE_DATA_H
