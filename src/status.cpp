//
// Created by Joshua Kwaku on 13/02/2022.
//

#include "../include/status.h"
std::string Kora::Status::toString() const {
    if (_message.empty()) return "OK";
    else {
        switch (code()) {
            case Kora::Code::_IOERROR:
                return "IO Error";
            case Kora::Code::_NOTFOUND:
                return "Data not found";
            case Kora::Code::_DONE:
                return "Done";
            default:
                return "Unknown code.";
        }
    }
}
