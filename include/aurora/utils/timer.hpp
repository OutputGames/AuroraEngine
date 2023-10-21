#ifndef TIMER_HPP
#define TIMER_HPP

#include "utils/utils.hpp"

class Timer {
public:
    AURORA_API Timer(size_t time, const std::function<void(void)>& f) : time{std::chrono::milliseconds{time}}, f{f} {}
    AURORA_API ~Timer() { wait_thread.join(); }

private:
    AURORA_API void wait_then_call()
    {
        std::unique_lock<std::mutex> lck{mtx};
        for(int i{10}; i > 0; --i) {
            std::cout << "Thread " << wait_thread.get_id() << " countdown at: " << '\t' << i << '\n';
            cv.wait_for(lck, time / 10);
        }
        f();
    }
    std::mutex mtx;
    std::condition_variable cv{};
    std::chrono::milliseconds time;
    std::function <void(void)> f;
    std::thread wait_thread{[this]() {wait_then_call(); }};
};

#endif