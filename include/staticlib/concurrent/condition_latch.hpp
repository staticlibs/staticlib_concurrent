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
 * File:   condition_latch.hpp
 * Author: alex
 *
 * Created on February 21, 2017, 9:38 PM
 */

#ifndef STATICLIB_CONCURRENT_CONDITION_LATCH_HPP
#define	STATICLIB_CONCURRENT_CONDITION_LATCH_HPP

#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>

namespace staticlib {
namespace concurrent {

class condition_latch : public std::enable_shared_from_this<condition_latch> {
    mutable std::mutex mutex;
    std::condition_variable cv;
    std::function<bool()> condition;

public:
    explicit condition_latch(std::function<bool()> condition) :
    condition(std::move(condition)) { }

    condition_latch(const condition_latch&) = delete;

    condition_latch& operator=(const condition_latch&) = delete;

    condition_latch(condition_latch&&) = delete;

    condition_latch& operator=(condition_latch&&) = delete;

    void await() {
        std::unique_lock<std::mutex> guard{mutex};
        cv.wait(guard, [this] {
            return condition();
        });
    }

    bool await(std::chrono::milliseconds timeout) {
        std::unique_lock<std::mutex> guard{mutex};
        return cv.wait_for(guard, timeout, [this] {
            return condition();
        });
    }

    void notify_one() {
        cv.notify_one();
    }

    void notify_all() {
        cv.notify_all();
    }
};

} // namespace
}


#endif	/* STATICLIB_CONCURRENT_CONDITION_LATCH_HPP */

