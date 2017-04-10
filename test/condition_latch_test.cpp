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

void test_latch() {
    std::atomic<bool> flag{false};
    std::shared_ptr<sl::concurrent::condition_latch> latch = std::make_shared<sl::concurrent::condition_latch>([&flag] {
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
