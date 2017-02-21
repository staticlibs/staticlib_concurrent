/* 
 * File:   spsc_onstack_concurrent_queue.hpp
 * Author: alex
 *
 * Created on February 21, 2017, 9:22 AM
 */

#ifndef STATICLIB_CONCURRENT_SPSC_ONSTACK_CONCURRENT_QUEUE_HPP
#define	STATICLIB_CONCURRENT_SPSC_ONSTACK_CONCURRENT_QUEUE_HPP

#include <cstdint>
#include <atomic>
#include <memory>
#include <utility>

namespace staticlib {
namespace concurrent {

template<typename T, size_t Size>
class spsc_onstack_concurrent_queue : public std::enable_shared_from_this<spsc_onstack_concurrent_queue<T, Size>> {
    typename std::aligned_storage<sizeof (T) * (Size + 1), std::alignment_of<T>::value>::type records_storage;
    const size_t size = Size + 1;
    T* const records;    
    std::atomic<size_t> read_idx;
    std::atomic<size_t> write_idx;

public:
    /**
     * Type of elements
     */
    using value_type = T;

    /**
     * Constructor,
     * 
     * @param size queue size, must be >= 1
     */
    spsc_onstack_concurrent_queue() :
    records(reinterpret_cast<T*> (std::addressof(records_storage))),
    read_idx(0),
    write_idx(0) { }

    explicit spsc_onstack_concurrent_queue(size_t) :
    records(reinterpret_cast<T*> (std::addressof(records_storage))),
    read_idx(0),
    write_idx(0) { }
    
    /**
     * Deleted copy constructor
     * 
     * @param other instance
     */
    spsc_onstack_concurrent_queue(const spsc_onstack_concurrent_queue&) = delete;

    /**
     * Deleted copy assignment operator
     * 
     * @param other instance
     * @return reference to self
     */
    spsc_onstack_concurrent_queue& operator=(const spsc_onstack_concurrent_queue&) = delete;

    spsc_onstack_concurrent_queue(spsc_onstack_concurrent_queue&&) = delete;

    spsc_onstack_concurrent_queue& operator=(spsc_onstack_concurrent_queue&&) = delete;

    /**
     * Destructor
     */
    ~spsc_onstack_concurrent_queue() {
        // We need to destruct anything that may still exist in our queue.
        // (No real synchronization needed at destructor time: only one
        // thread can be doing this.)

        // check disabled, still safe, may be slower
        // if (!boost::has_trivial_destructor<T>::value) {
        size_t read = read_idx.load(std::memory_order_acquire);
        size_t end = write_idx.load(std::memory_order_acquire);
        while (read != end) {
            records[read].~T();
            if (++read == size) {
                read = 0;
            }
        }
        // }
    }

    /**
     * Emplace a value at the end of the queue
     * 
     * @param recordArgs constructor arguments for queue element
     * @return false if the queue was full, true otherwise
     */
    template<class ...Args>
    bool emplace(Args&&... record_args) {
        size_t const current_write = write_idx.load(std::memory_order_relaxed);
        size_t next_record = current_write + 1;
        if (next_record == size) {
            next_record = 0;
        }
        if (next_record != read_idx.load(std::memory_order_acquire)) {
            new (std::addressof(records[current_write])) T(std::forward<Args>(record_args)...);
            write_idx.store(next_record, std::memory_order_release);
            return true;
        }

        // queue is full
        return false;
    }

    /**
     * Attempt to read the value at the front to the queue into a variable
     * 
     * @param record move (or copy) the value at the front of the queue to given variable
     * @return  returns false if queue was empty, true otherwise
     */
    bool poll(T& record) {
        size_t const current_read = read_idx.load(std::memory_order_relaxed);
        if (current_read == write_idx.load(std::memory_order_acquire)) {
            // queue is empty
            return false;
        }

        size_t next_record = current_read + 1;
        if (next_record == size) {
            next_record = 0;
        }
        record = std::move(records[current_read]);
        records[current_read].~T();
        read_idx.store(next_record, std::memory_order_release);
        return true;
    }

    /**
     * Retrieve a pointer to the item at the front of the queue
     * 
     * @return a pointer to the item, nullptr if it is empty
     */
    T* front_ptr() {
        size_t const current_read = read_idx.load(std::memory_order_relaxed);
        if (current_read == write_idx.load(std::memory_order_acquire)) {
            // queue is empty
            return nullptr;
        }
        return std::addressof(records[current_read]);
    }

    /**
     * Check if the queue is empty
     * 
     * @return whether queue is empty
     */
    bool empty() const {
        return read_idx.load(std::memory_order_acquire) ==
                write_idx.load(std::memory_order_acquire);
    }

    /**
     * Check if the queue is full
     * 
     * @return whether queue is full
     */
    bool full() const {
        size_t next_record = write_idx.load(std::memory_order_acquire) + 1;
        if (next_record == size) {
            next_record = 0;
        }
        if (next_record != read_idx.load(std::memory_order_acquire)) {
            return false;
        }
        // queue is full
        return true;
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
    size_t size_guess() const {
        int ret = write_idx.load(std::memory_order_acquire) -
                read_idx.load(std::memory_order_acquire);
        if (ret < 0) {
            ret += size;
        }
        return ret;
    }

    /**
     * Accessor for max queue size specified at creation
     * 
     * @return max queue size
     */
    size_t max_size() const {
        return size - 1;
    }
};    
    
} // namespace
}

#endif	/* STATICLIB_CONCURRENT_SPSC_ONSTACK_CONCURRENT_QUEUE_HPP */

