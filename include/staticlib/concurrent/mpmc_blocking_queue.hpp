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
 * File:   mpmc_blocking_queue.hpp
 * Author: alex
 *
 * Created on February 20, 2017, 8:44 PM
 */

#ifndef STATICLIB_CONCURRENT_MPMC_BLOCKING_QUEUE_HPP
#define	STATICLIB_CONCURRENT_MPMC_BLOCKING_QUEUE_HPP

#include <cstdint>
#include <chrono>
#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>

namespace staticlib {
namespace concurrent {

template<typename T>
class mpmc_blocking_queue : public std::enable_shared_from_this<mpmc_blocking_queue<T>> {
    mutable std::mutex mutex;
    std::condition_variable empty_cv;
    std::deque<T> queue;
    const size_t max_queue_size;
    bool unblocked = false;
    
public:
    /**
     * Type of elements
     */
    using value_type = T;
    
    explicit mpmc_blocking_queue(size_t max_queue_size = 0) :
    max_queue_size(max_queue_size) { }

    mpmc_blocking_queue(const mpmc_blocking_queue&) = delete;

    mpmc_blocking_queue& operator=(const mpmc_blocking_queue&) = delete;

    mpmc_blocking_queue(mpmc_blocking_queue&&) = delete;

    mpmc_blocking_queue& operator=(mpmc_blocking_queue&&) = delete;

    /**
     * Emplace a value at the end of the queue
     * 
     * @param recordArgs constructor arguments for queue element
     * @return false if the queue was full, true otherwise
     */
    template<typename ...Args>
    bool emplace(Args&&... record_args) {
        std::lock_guard<std::mutex> guard{mutex};
        auto size = queue.size();
        if (0 == max_queue_size || size < max_queue_size) {
            queue.emplace_back(std::forward<Args>(record_args)...);
            if (0 == size) {
                empty_cv.notify_all();
            }
            return true;
        } else {
            return false;
        }
    }

    /**
     * Emplace the values from specified range into
     * this queue
     * 
     * @param range source range
     * @return number of elements emplaced
     */
    template<typename Range,
            class = typename std::enable_if<!std::is_lvalue_reference<Range>::value>::type>
    size_t emplace_range(Range&& range) {
        std::lock_guard<std::mutex> guard{mutex};
        auto origin_size = queue.size();
        for (auto&& el : range) {
            if (0 == max_queue_size || queue.size() < max_queue_size) {
                queue.emplace_back(std::move(el));
            } else {
                break;
            }
        }
        return queue.size() - origin_size;
    }

    /**
     * Emplace the values from specified range into
     * this queue
     * 
     * @param range source range
     * @return number of elements emplaced
     */
    template<typename Range>
    size_t emplace_range(Range& range) {
        std::lock_guard<std::mutex> guard{mutex};
        auto origin_size = queue.size();
        for (auto& el : range) {
            if (0 == max_queue_size || queue.size() < max_queue_size) {
                queue.emplace_back(el);
            } else {
                break;
            }
        }
        return queue.size() - origin_size;
    }

    /**
     * Attempt to read the value at the front to the queue into a variable.
     * This method returns immediately.
     * 
     * @param record move (or copy) the value at the front of the queue to given variable
     * @return returns false if queue was empty, true otherwise
     */
    bool poll(T& record) {
        std::lock_guard<std::mutex> guard{mutex};
        if (!queue.empty()) {
            record = std::move(queue.front());
            queue.pop_front();
            return true;
        } else {
            return false;
        }
    }

    /**
     * Consume all the immediately-available 
     * contents of this queue into specified functor
     * 
     * @param func functor to consume contents
     * @return number of elements consumed
     */
    template<typename Func>
    size_t poll(Func&& func) {
        std::lock_guard<std::mutex> guard{mutex};
        auto origin_size = queue.size();
        while (!queue.empty()) {
            T record = std::move(queue.front());
            queue.pop_front();
            func(std::move(record));
        }
        return origin_size - queue.size();
    }

    /**
     * Attempt to read the value at the front of the queue into a variable.
     * This method will wait on empty queue infinitely (by default), 
     * or up to specified amount of milliseconds
     * 
     * @param record move (or copy) the value at the front of the queue to given variable
     * @param timeout max amount of milliseconds to wait on empty queue,
     *        negative value (supplied by default) will cause infinite wait
     * @return returns false if queue was empty after timeout, true otherwise
     */
    bool take(T& record, std::chrono::milliseconds timeout = std::chrono::milliseconds(0)) {
        std::unique_lock<std::mutex> lock{mutex};
        if (!queue.empty()) {
            record = std::move(queue.front());
            queue.pop_front();
            return true;
        } else {
            auto predicate = [this] {
                return this->unblocked || !this->queue.empty();
            };
            if (timeout > std::chrono::milliseconds(0)) {
                empty_cv.wait_for(lock, timeout, predicate);
            } else {
                empty_cv.wait(lock, predicate);
            }
            if (!queue.empty()) {
                record = std::move(queue.front());
                queue.pop_front();
                return true;
            } else {
                return false;
            }
        }
    }

    /**
     * Unblocks the queue allowing consumers to
     * exit 'take' calls. Queue cannot be used
     * for waiting on it after this call.
     */
    void unblock() {
        std::lock_guard<std::mutex> guard{mutex};
        this->unblocked = true;
        if (queue.empty()) {
            empty_cv.notify_all();
        }
    }

    /**
     * Checks whether this queue was unblocked
     * 
     * @return whether this queue was unblocked
     */
    bool is_unblocked() const {
        std::lock_guard<std::mutex> guard{mutex};
        return unblocked;
    }

    /**
     * Check if the queue is empty
     * 
     * @return whether queue is empty
     */
    bool empty() const {
        std::lock_guard<std::mutex> guard{mutex};
        return queue.empty();
    }

    /**
     * Check if the queue is full, always false for unbounded queue
     * 
     * @return whether queue is full
     */
    bool full() const {
        std::lock_guard<std::mutex> guard{mutex};
        if (0 == max_queue_size) {
            return false;
        }
        return queue.size() >= max_queue_size;
    }

    /**
     * Returns the number of entries in the queue
     * 
     * @return number of entries in the queue
     */
    size_t size() const {
        std::lock_guard<std::mutex> guard{mutex};
        return queue.size();
    }

    /**
     * Accessor for max queue size specified at creation
     * 
     * @return max queue size
     */
    size_t max_size() const {
        return max_queue_size;
    }

};

} // namespace
}

#endif	/* STATICLIB_CONCURRENT_MPMC_BLOCKING_QUEUE_HPP */

