//
// Created by Joshua Kwaku on 13/02/2022.
//

#ifndef KV_STORE_STATUS_H
#define KV_STORE_STATUS_H


#include <string>

namespace Kora {
    enum class Code {
        _OK = 1,
        _NOTFOUND = 2,
        _IOERROR = 3,
        _DONE = 4
    };

    class Status {
    public:
        Status(): _code(Code::_OK), _message{""} {};
        Status(Code code, std::string message): _code{code}, _message{message} {}
        Status(Code code): _code{code} {}
        ~Status();

        static Status OK() { return Status(); }
        static Status NotFound(std::string message) { return Status(Code::_NOTFOUND, message); }
        static Status Done() { return Status(Code::_DONE); }
        static Status IoError(std::string message) { return Status(Code::_IOERROR, message); }
        Code code() const { return _code; }
        std::string toString() const;

        bool isOk() { return _code == Code::_OK; }
        bool isNotFound() { return _code == Code::_NOTFOUND; }
        bool isIoError() { return _code == Code::_IOERROR; }
        bool isDone() { return _code == Code::_DONE; }

    private:
        Code _code;
        std::string _message;
    };

}

#endif //KV_STORE_STATUS_H
