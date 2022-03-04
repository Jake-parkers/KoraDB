//
// Created by Joshua Kwaku on 23/02/2022.
//

#ifndef KV_STORE_TIMER_H
#define KV_STORE_TIMER_H

#include <functional>
#include <thread>

namespace Kora {
    class Timer {
    public:
        Timer() : _execute{false} {}

        ~Timer() {
            _thread.join();
        }

        void start(int interval, std::function<void(void)> func) {
            _execute = true;
            _thread = std::thread([this, interval, func] {
                while (_execute) {
                    func();
                    std::this_thread::sleep_for(std::chrono::milliseconds(interval));
                }
            });
        }
    private:
        bool _execute = false;
        std::thread _thread;
    };
}

#endif //KV_STORE_TIMER_H
