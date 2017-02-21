/* 
 * File:   condition_latch_test.cpp
 * Author: alex
 *
 * Created on February 21, 2017, 9:43 PM
 */

#include "staticlib/concurrent/condition_latch.hpp"

#include <atomic>
#include <iostream>
#include <memory>
#include <thread>

#include "staticlib/config/assert.hpp"

namespace sc = staticlib::concurrent;

void test_latch() {
    std::atomic<bool> flag{false};
    std::shared_ptr<sc::condition_latch> latch = std::make_shared<sc::condition_latch>([&flag] {
       return flag.load(std::memory_order_acquire);
    });
    std::atomic<int> shared{0};
    auto th = std::thread([&shared, &latch, &flag] {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        shared.fetch_add(1, std::memory_order_relaxed);
        flag.store(true, std::memory_order_release);
        latch->notify_one();
    });
    latch->await();
    slassert(1 == shared.load(std::memory_order_relaxed));
    th.join();
}

int main() {
    try {
        test_latch();
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        return 1;
    }
    return 0;
}
