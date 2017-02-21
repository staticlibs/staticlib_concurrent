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

class countdown_latch : public std::enable_shared_from_this<countdown_latch> {
    mutable std::mutex mutex;
    std::condition_variable cv;
    size_t count;
    
public:
    explicit countdown_latch(size_t count) :
    count(count) { }
    
    countdown_latch(const countdown_latch&) = delete;
    
    countdown_latch& operator=(const countdown_latch&) = delete;
    
    countdown_latch(countdown_latch&&) = delete;
    
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
        std::lock_guard<std::mutex> guard{mutex};
        if (count > 0) {
            count -= 1;            
        }
        if (0 == count) {
            cv.notify_all();
        }
        return count;
    }
    
    size_t get_count() const {
        std::lock_guard<std::mutex> guard{mutex};
        return count;
    }
    
    size_t reset(size_t count_value) {
        std::lock_guard<std::mutex> guard{mutex};
        size_t prev = count;
        count = count_value;
        if (0 == count) {
            cv.notify_all();
        }
        return prev;
    }
    
};

} // namespace
}

#endif	/* STATICLIB_CONCURRENT_COUNTDOWN_LATCH_HPP */

