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
 * File:   countdown_latch.hpp
 * Author: alex
 *
 * Created on February 20, 2017, 9:37 PM
 */

#ifndef STATICLIB_CONCURRENT_COUNTDOWN_LATCH_HPP
#define	STATICLIB_CONCURRENT_COUNTDOWN_LATCH_HPP

#include <cstdint>
#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>

namespace staticlib {
namespace concurrent {

/**
 * Synchronization aid that allows one or more threads to wait until a set
of operations being performed in other threads completes
 */
class countdown_latch : public std::enable_shared_from_this<countdown_latch> {
    mutable std::mutex mutex;
    std::condition_variable cv;
    size_t count;
    
public:
    /**
     * Constructor
     * 
     * @param count initial value of the counter
     */
    explicit countdown_latch(size_t count) :
    count(count) { }
    
    /**
     * Deleted copy constructor
     */
    countdown_latch(const countdown_latch&) = delete;

    /**
     * Deleted copy assignment operator
     */
    countdown_latch& operator=(const countdown_latch&) = delete;

    /**
     * Deleted move constructor
     */
    countdown_latch(countdown_latch&&) = delete;

    /**
     * Deleted move assignment operator
     */
    countdown_latch& operator=(countdown_latch&&) = delete;
    
    void await() {
        std::unique_lock<std::mutex> guard{mutex};
        cv.wait(guard, [this] {
            return 0 == count;
        });
    }

    bool await(std::chrono::milliseconds timeout) {
        std::unique_lock<std::mutex> guard{mutex};
        return cv.wait_for(guard, timeout, [this] {
            return 0 == count;
        });
    }
    
    size_t count_down() {
        size_t updated;
        {
            std::lock_guard<std::mutex> guard{mutex};
            if (count > 0) {
                count -= 1;            
            }
            updated = count;
        }
        if (0 == updated) {
            cv.notify_all();
        }
        return updated;
    }
    
    size_t get_count() const {
        std::lock_guard<std::mutex> guard{mutex};
        return count;
    }
    
    size_t reset(size_t count_value) {
        size_t prev = 0;
        {
            std::lock_guard<std::mutex> guard{mutex};
            prev = count;
            count = count_value;
        }
        if (0 == count_value) {
            cv.notify_all();
        }
        return prev;
    }
    
};

} // namespace
}

#endif	/* STATICLIB_CONCURRENT_COUNTDOWN_LATCH_HPP */

