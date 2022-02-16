//
// Created by Joshua Kwaku on 13/02/2022.
//

#ifndef KV_STORE_OPTIONS_H
#define KV_STORE_OPTIONS_H

namespace Kora {
    struct Options {
        Options(): create_if_missing{false} {}

        // If true, the database will be created if it is missing.
        bool create_if_missing = false;

        // If true, an error is raised if
    };
    struct WriteOptions {
        bool sync = false;
    };
}


#endif //KV_STORE_OPTIONS_H
