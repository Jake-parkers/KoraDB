//
// Created by Joshua Kwaku on 13/02/2022.
//

#include "status.h"
std::string Kora::Status::toString() const {
    if (_message == "") return "OK";
    else {
        switch (code()) {
            case Kora::Code::_IOERROR:
                return "IO Error";
                break;
            case Kora::Code::_NOTFOUND:
                return "Data not found";
                break;
            case Kora::Code::_DONE:
                return "Done";
                break;
            default:
                return "Unknown code.";
                break;
        }
    }
}
