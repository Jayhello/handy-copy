//
// Created by DW on 2018/11/25.
//

#ifndef C_BASIC_INTERRUPTSLEEP_H
#define C_BASIC_INTERRUPTSLEEP_H

#include "noncopyable.h"

#include <chrono>
#include <condition_variable>
#include <mutex>

namespace xy{
    using Clock = std::chrono::system_clock;

    class InterruptSleep: public noncopyable{
        public:
            InterruptSleep(bool inter=false):interrupt_(inter){}

            void sleep();
            void sleep_for(const Clock::duration dura);
            void sleep_until(const Clock::time_point timePoint);

            void interrupt();

        private:
            std::condition_variable cv_;
            std::mutex mtx_;

            bool interrupt_;
    };

    void InterruptSleep::sleep() {
        std::unique_lock<std::mutex> lock(mtx_);

        cv_.wait(lock, [this](){ return interrupt_;});
        interrupt_ = false;
    }

    void InterruptSleep::interrupt(){
        std::lock_guard<std::mutex> lock(mtx_);
        interrupt_ = true;

        cv_.notify_one();// TODO note this
    }

    void InterruptSleep::sleep_for(const Clock::duration dura) {
        std::unique_lock<std::mutex> lock(mtx_);

        cv_.wait_for(lock, dura, [this](){return interrupt_;});
        interrupt_ = false;
    }

    void InterruptSleep::sleep_until(const Clock::time_point timePoint){
        std::unique_lock<std::mutex> lock(mtx_);
        cv_.wait_until(lock, timePoint, [this](){return interrupt_;});
        interrupt_ = false;
    }

}

#endif //C_BASIC_INTERRUPTSLEEP_H
