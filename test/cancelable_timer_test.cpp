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
 * File:   cancelable_timer_test.cpp
 * Author: alex
 *
 * Created on May 18, 2017, 5:31 PM
 */

#include "staticlib/concurrent/cancelable_timer.hpp"

#include <chrono>
#include <iostream>
#include <thread>

#include "staticlib/config/assert.hpp"
#include "staticlib/support.hpp"

class test_timer {
    std::chrono::microseconds millis = std::chrono::microseconds(0);
    bool cancel_called = false;
    
public:
    void expires_from_now(std::chrono::microseconds ms) {
        this->millis = ms;
    }
    
    void async_wait(std::function<void(const std::error_code&)> timeout_handler) {
        auto th = std::thread([this, timeout_handler] {
            std::this_thread::sleep_for(this->millis);
            timeout_handler(std::error_code(0, std::system_category()));
        });
        th.detach();
    }
    
    void cancel() {
        this->cancel_called = true;
    }
};

void test_cancel() {
    bool handler_called = false;
    auto ct = std::make_shared<sl::concurrent::cancelable_timer<test_timer>>(sl::support::make_unique<test_timer>());
    ct->start(std::chrono::milliseconds(100), [&handler_called] (const std::error_code&) {
        handler_called = true;
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ct->cancel();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    slassert(!handler_called)
}

void test_timeout() {
    bool handler_called = false;
    auto ct = std::make_shared<sl::concurrent::cancelable_timer<test_timer>>(sl::support::make_unique<test_timer>());
    ct->start(std::chrono::milliseconds(50), [&handler_called] (const std::error_code&) {
        handler_called = true;
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    slassert(handler_called)
}

int main() {
    try {
        test_cancel();
        test_timeout();
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        return 1;
    }
    return 0;
}

