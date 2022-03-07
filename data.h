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
            _data = (char *) malloc(other._size + 1);
            strcpy(_data, other._data);
            _size = other._size;
        }
        // copy assignment
        Data& operator=(const Data&) = default;

        // move constructor
        Data(Data&& that) noexcept: _data{nullptr}, _size{0} {
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
        size_t _size = 0;
    };

    class Comparator {
    public:
        bool operator()(const Data& d1, const Data& d2) const {
            const size_t min_len = (d1.size() < d2.size()) ? d1.size() : d2.size();
            int result = memcmp(d1.data(), d2.data(), min_len);
            if (result < 0) {
                // first differing byte in d1 is less than that of d2, hence  d1 precedes d2
                return true;
            }
            if (result == 0) {
                if (d1.size() < d2.size()) {
                    // they are not the same length and d1 is the smaller one, then it precedes d2
                    return true;
                } else if (d1.size() > d2.size()) {
                    return false;
                } else {
                    // they are same so bool doesn't really matter here
                    return false;
                }
            }

            if (result > 0) {
                // first differing byte in d1 is greater than that of d2, hence d2 precedes d1
                return false;
            }
        }
    };
}

#endif //KV_STORE_DATA_H
