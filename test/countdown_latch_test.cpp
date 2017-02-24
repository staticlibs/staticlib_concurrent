/*
 * Copyright 2017, alex at staticlibs.net
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* 
 * File:   countdown_latch_test.cpp
 * Author: alex
 *
 * Created on February 21, 2017, 1:26 PM
 */

#include "staticlib/concurrent/countdown_latch.hpp"

#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>

#include "staticlib/config/assert.hpp"

namespace sc = staticlib::concurrent;

void test_latch() {
    sc::countdown_latch latch{2};
    std::atomic<int> shared{0};
    auto th1 = std::thread([&shared, &latch]{
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        shared.fetch_add(1, std::memory_order_relaxed);
        latch.count_down();
    });
    auto th2 = std::thread([&shared, &latch] {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
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
