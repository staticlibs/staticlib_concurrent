/* 
 * File:   countdown_latch_test.cpp
 * Author: alex
 *
 * Created on February 21, 2017, 1:26 PM
 */

#include "staticlib/concurrent/countdown_latch.hpp"

#include <atomic>
#include <iostream>
#include <thread>
#include <bits/atomic_base.h>

#include "staticlib/config/assert.hpp"

namespace sc = staticlib::concurrent;

void test_latch() {
    sc::countdown_latch latch{2};
    std::atomic<int> shared{0};
    auto th1 = std::thread([&shared, &latch]{
        shared.fetch_add(1, std::memory_order_relaxed);
        latch.count_down();
    });
    auto th2 = std::thread([&shared, &latch] {
        shared.fetch_add(1, std::memory_order_relaxed);
        latch.count_down();
    });
    latch.await();
    slassert(2 == shared.load(std::memory_order_relaxed));
    th1.join();
    th2.join();
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
