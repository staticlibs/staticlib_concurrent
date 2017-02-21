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

