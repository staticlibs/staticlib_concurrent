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
 * File:   cancelable_timer->hpp
 * Author: alex
 *
 * Created on May 18, 2017, 5:27 PM
 */

#ifndef STATICLIB_CONCURRENT_CANCELABLE_TIMER_HPP
#define	STATICLIB_CONCURRENT_CANCELABLE_TIMER_HPP

#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <system_error>

namespace staticlib {
namespace concurrent {

/**
 * Helper class used to time-out operations.
 */
template <typename Timer>
class cancelable_timer : public std::enable_shared_from_this<cancelable_timer<Timer>> {
    /**
     * Mutex used to synchronize all operations
     */
    std::mutex mx;

    /**
     * Timer implementation with api compatible with asio
     */
    std::unique_ptr<Timer> timer;
    
    /**
     * True if the timer is active
     */
    bool active = false;

    /**
     * True if the timer was canceled
     */
    bool canceled = false;

public:

    /**
     * Constructor for inheritors
     */
    explicit cancelable_timer(std::unique_ptr<Timer>&& timer) :
    timer(std::move(timer)) { }
    
    /**
     * Deleted copy constructor
     */
    cancelable_timer(const cancelable_timer&) = delete;

    /**
     * Deleted copy assignment operator
     */
    cancelable_timer& operator=(const cancelable_timer&) = delete;

    /**
     * Starts a timer
     *
     * @param milliseconds number of seconds before the timeout triggers
     */
    void start(std::chrono::milliseconds timeout, std::function<void(const std::error_code&)> timeout_handler) {
        std::lock_guard<std::mutex> guard{mx};
        timer->expires_from_now(timeout);
        auto self = this->shared_from_this();
        timer->async_wait([self, timeout_handler](const std::error_code& ec) {
            std::lock_guard<std::mutex> guard{self->mx};
            self->active = false;
            if (!self->canceled) {
                timeout_handler(ec);
            }
        });
    }

    /**
     * Cancel the timer (operation completed)
     */
    void cancel() {
        std::lock_guard<std::mutex> guard{mx};
        this->canceled = true;
        if (active) {
            timer->cancel();
        }
    }

    /**
     * Mutex accessor
     * 
     * @return mutex reference
     */
    std::mutex& mutex() {
        return mx;
    }
};

} // namespace
}

#endif	/* STATICLIB_CONCURRENT_CANCELABLE_TIMER_HPP */

