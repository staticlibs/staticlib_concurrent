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
#define STATICLIB_CONCURRENT_CONDITION_LATCH_HPP

#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>

namespace staticlib {
namespace concurrent {

/**
 * Spurious-wakeup-free lock, uses arbitrary "condition" functor to check locked/unlocked state
 */
class condition_latch : public std::enable_shared_from_this<condition_latch> {
    mutable std::mutex mutex;
    std::condition_variable cv;
    std::function<bool()> condition;

public:
    /**
     * Constructor
     * 
     * @param condition locked/unlocked state functor
     */
    explicit condition_latch(std::function<bool()> condition) :
    condition(std::move(condition)) { }

    /**
     * Deleted copy constructor
     */
    condition_latch(const condition_latch&) = delete;

    /**
     * Deleted copy assignment operator
     */
    condition_latch& operator=(const condition_latch&) = delete;

    /**
     * Deleted move constructor
     */
    condition_latch(condition_latch&&) = delete;

    /**
     * Deleted move assignment operator
     */
    condition_latch& operator=(condition_latch&&) = delete;

    /**
     * Wait on this latch until specified condition won't
     * become positive and latch will be notified about that
     */
    void await() {
        std::unique_lock<std::mutex> guard{mutex};
        cv.wait(guard, [this] {
            return condition();
        });
    }

    /**
     * Wait on this latch until specified condition won't
     * become positive and latch will be notified about that or
     * specified timeout will be expired
     * 
     * @param timeout max time period to wait
     * @return false if exit on timeout expiry, true otherwise
     */
    bool await(std::chrono::milliseconds timeout) {
        std::unique_lock<std::mutex> guard{mutex};
        return cv.wait_for(guard, timeout, [this] {
            return condition();
        });
    }

    /**
     * Notifies one of the threads, waiting on this lock,
     * to awake and re-check the condition
     */
    void notify_one() {
        cv.notify_one();
    }

    /**
     * Notifies all the threads, waiting on this lock,
     * to awake and re-check the condition
     */
    void notify_all() {
        cv.notify_all();
    }
};

} // namespace
}


#endif /* STATICLIB_CONCURRENT_CONDITION_LATCH_HPP */

