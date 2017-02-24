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
 * File:   spsc_inobject_waiting_queue.hpp
 * Author: alex
 *
 * Created on February 21, 2017, 9:24 AM
 */

#ifndef STATICLIB_CONCURRENT_SPSC_INOBJECT_WAITING_QUEUE_HPP
#define	STATICLIB_CONCURRENT_SPSC_INOBJECT_WAITING_QUEUE_HPP

#include <cstdint>
#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>

#include "staticlib/concurrent/spsc_inobject_concurrent_queue.hpp"

namespace staticlib {
namespace concurrent {

template<typename T, size_t Size>
class spsc_inobject_waiting_queue : public std::enable_shared_from_this<spsc_inobject_waiting_queue<T, Size>> {
    mutable std::mutex mutex;
    std::condition_variable empty_cv;
    spsc_inobject_concurrent_queue<T, Size> queue;
    bool unblocked = false;

public:
    /**
     * Type of elements
     */
    using value_type = T;

    spsc_inobject_waiting_queue() :
    queue(Size) { }

    explicit spsc_inobject_waiting_queue(size_t) :
    queue(Size) { }

    /**
     * Deleted copy constructor
     * 
     * @param other instance
     */
    spsc_inobject_waiting_queue(const spsc_inobject_waiting_queue&) = delete;

    /**
     * Deleted copy assignment operator
     * 
     * @param other instance
     * @return reference to self
     */
    spsc_inobject_waiting_queue& operator=(const spsc_inobject_waiting_queue&) = delete;

    spsc_inobject_waiting_queue(spsc_inobject_waiting_queue&&) = delete;

    spsc_inobject_waiting_queue& operator=(spsc_inobject_waiting_queue&&) = delete;

    /**
     * Emplace a value at the end of the queue
     * 
     * @param recordArgs constructor arguments for queue element
     * @return false if the queue was full, true otherwise
     */
    template<class ...Args>
    bool emplace(Args&&... record_args) {
        bool res = queue.emplace(std::forward<Args>(record_args)...);
        empty_cv.notify_one();
        return res;
    }

    /**
     * Attempt to read the value at the front to the queue into a variable
     * 
     * @param record move (or copy) the value at the front of the queue to given variable
     * @return  returns false if queue was empty, true otherwise
     */
    bool poll(T& record) {
        return queue.poll(record);
    }

    /**
     * Retrieve a pointer to the item at the front of the queue
     * 
     * @return a pointer to the item, nullptr if it is empty
     */
    T* front_ptr() {
        return queue.front_ptr();
    }

    bool take(T& record) {
        bool res = queue.poll(record);
        if (res) {
            return res;
        }
        std::unique_lock<std::mutex> guard{mutex};
        empty_cv.wait(guard, [this] {
            return unblocked || !queue.empty();
        });
        return queue.poll(record);
    }

    void unblock() {
        std::lock_guard<std::mutex> guard{mutex};
        this->unblocked = true;
        if (queue.empty()) {
            empty_cv.notify_one();
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
        return queue.empty();
    }

    /**
     * Check if the queue is full
     * 
     * @return whether queue is full
     */
    bool full() const {
        return queue.full();
    }

    /**
     * Returns the number of entries in the queue.
     * If called by consumer, then true size may be more (because producer may
     * be adding items concurrently).
     * If called by producer, then true size may be less (because consumer may
     * be removing items concurrently).
     * It is undefined to call this from any other thread.
     * 
     * @return number of entries in the queue
     */
    size_t size() const {
        return queue.size();
    }

    /**
     * Accessor for max queue size specified at creation
     * 
     * @return max queue size
     */
    size_t max_size() const {
        return queue.max_size();
    }
};    
    
} // namespace
}

#endif	/* STATICLIB_CONCURRENT_SPSC_INOBJECT_WAITING_QUEUE_HPP */

