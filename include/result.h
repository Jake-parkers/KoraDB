//
// Created by kwaku on 14/02/2022.
//

#ifndef KV_STORE_RESULT_H
#define KV_STORE_RESULT_H

#include <utility>

#include "status.h"

namespace Kora {
    class Result {
    public:
        // constructor
        Result() = delete;
        explicit Result(const Status&& status) : _status{status} {}
        Result(const Status&& status, std::string&& data): _status{status}, _data {data} {}

        // setters
        void setData(const char* data) { _data = data; }
        void setStatus(const Status&& status) { _status = status; }

        // getters
        std::string data() { return _data; }
        Status status() { return _status; }
    private:
        Status _status;
        std::string _data;
    };
}

#endif //KV_STORE_RESULT_H
