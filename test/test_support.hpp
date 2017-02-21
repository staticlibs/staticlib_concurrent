/* 
 * File:   test_support.hpp
 * Author: alex
 *
 * Created on February 21, 2017, 2:13 PM
 */

#ifndef STATICLIB_CONCURRENT_TEST_SUPPORT_HPP
#define	STATICLIB_CONCURRENT_TEST_SUPPORT_HPP

// source: https://github.com/facebook/folly/blob/b75ef0a0af48766298ebcc946dd31fe0da5161e3/folly/test/ProducerConsumerQueueTest.cpp

#include <atomic>
#include <chrono>
#include <memory>
#include <string>
#include <thread>
#include <vector>

template<class T> struct test_traits {

    T limit() const {
        return 1 << 12;
    }

    T generate() const {
        return rand() % 26;
    }
};

template<> struct test_traits<std::string> {

    unsigned int limit() const {
        return 1 << 12;
    }

    std::string generate() const {
        return std::string(12, '#');
    }
};

template<class QueueType, size_t Size>
struct perf_test {
    typedef typename QueueType::value_type T;

    explicit perf_test() : queue_(Size), done_(false) { }

    void operator()() {
        using namespace std::chrono;
        auto const startTime = system_clock::now();
        std::thread producer([this] {
            this->producer(); });
        std::thread consumer([this] {
            this->consumer(); });
        producer.join();
        done_ = true;
        consumer.join();
        auto duration = duration_cast<milliseconds>(system_clock::now() - startTime);
        std::cout << " done: " << duration.count() << "ms" << std::endl;
    }

    void producer() {
        // This is written differently than you might expect so that
        // it does not run afoul of -Wsign-compare, regardless of the
        // signedness of this loop's upper bound.
        for (auto i = traits_.limit(); i > 0; --i) {
            while (!queue_.emplace(traits_.generate())) {
            }
        }
    }

    void consumer() {
        while (!done_) {
            T data;
            queue_.poll(data);
        }
    }
    QueueType queue_;
    std::atomic<bool> done_;
    test_traits<T> traits_;
};

template<class TestType> void do_test(const char* name) {
    std::cout << " testing: " << name << std::endl;
    std::unique_ptr<TestType> const t(new TestType());
    (*t)();
}

template<class Queue, size_t Size, bool Pop = false >
void perf_test_type(const char* type) {
    const size_t size = Size;
    std::cout << "Type: " << type << std::endl;
    do_test<perf_test<Queue, size> >("ProducerConsumerQueue");
}

template<class QueueType, size_t Size>
struct correctness_test {
    typedef typename QueueType::value_type T;

    std::vector<T> testData_;
    QueueType queue_;
    test_traits<T> traits_;
    std::atomic<bool> done_;

    explicit correctness_test() :
    queue_(Size),
    done_(false) {
        const size_t testSize = static_cast<size_t> (traits_.limit());
        testData_.reserve(testSize);
        for (size_t i = 0; i < testSize; ++i) {
            testData_.push_back(traits_.generate());
        }
    }

    void operator()() {
        std::thread producer([this] {
            this->producer(); });
        std::thread consumer([this] {
            this->consumer(); });
        producer.join();
        done_ = true;
        consumer.join();
    }

    void producer() {
        for (auto& data : testData_) {
            while (!queue_.emplace(data)) {
            }
        }
    }

    void consumer() {
        for (auto expect : testData_) {
        again:
            T data;
            if (!queue_.poll(data)) {
                if (done_) {
                    // Try one more read; unless there's a bug in the queue class
                    // there should still be more data sitting in the queue even
                    // though the producer thread exited.
                    if (!queue_.poll(data)) {
                        throw staticlib::config::assert_exception(TRACEMSG("Finished too early ..."));
                    }
                } else {
                    goto again;
                }
            }
            (void) expect;
            slassert(data == expect);
        }
    }
};

template<class Queue, size_t Size>
void correctness_test_type(const std::string& type) {
    std::cout << "Type: " << type << std::endl;
    do_test<correctness_test<Queue, Size> >("ProducerConsumerQueue");
}

struct dtor_checker {
    static unsigned int numInstances;

    dtor_checker() {
        ++numInstances;
    }

    dtor_checker(const dtor_checker&) {
        ++numInstances;
    }

    ~dtor_checker() {
        --numInstances;
    }
};

unsigned int dtor_checker::numInstances = 0;

template<typename Queue>
void test_destructor() {
    // Test that orphaned elements in a ProducerConsumerQueue are
    // destroyed.
    {
        Queue queue(1024);
        for (int i = 0; i < 10; ++i) {
            slassert(queue.emplace(dtor_checker()));
        }
        slassert(dtor_checker::numInstances == 10);
        {
            dtor_checker ignore;
            slassert(queue.poll(ignore));
            slassert(queue.poll(ignore));
        }
        slassert(dtor_checker::numInstances == 8);
    }
    slassert(dtor_checker::numInstances == 0);
    // Test the same thing in the case that the queue write pointer has
    // wrapped, but the read one hasn't.
//    {
//        Queue queue(4);
//        for (int i = 0; i < 3; ++i) {
//            slassert(queue.emplace(dtor_checker()));
//        }
//        slassert(dtor_checker::numInstances == 3);
//        {
//            dtor_checker ignore;
//            slassert(queue.poll(ignore));
//        }
//        slassert(dtor_checker::numInstances == 2);
//        slassert(queue.emplace(dtor_checker()));
//        slassert(dtor_checker::numInstances == 3);
//    }
//    slassert(dtor_checker::numInstances == 0);
}

template<typename Queue>
void test_empty_full() {
    Queue queue(3);
    slassert(queue.empty());
    slassert(!queue.full());
    slassert(queue.emplace(1));
    slassert(!queue.empty());
    slassert(!queue.full());
    slassert(queue.emplace(2));
    slassert(!queue.empty());
    slassert(queue.emplace(3));
    slassert(queue.full());
    slassert(queue.size_guess() == 3);
}

template<typename Queue>
void test_speed(Queue& queue) {
    auto start = std::chrono::system_clock::now();
    auto th = std::thread([&queue] {
        size_t i = 0;
        size_t sum = 0;
        size_t fail = 0;
        while (i < 10000000) {
            bool success = queue.poll(i);
            if (success) {
                sum += i;
            } else {
                fail += 1;
            }
        }
        std::cout << "worker exit, sum: [" << sum << "], fails: [" << fail << "]" << std::endl;
    });
    size_t fail = 0;
    for (size_t i = 0; i < 10000001; i++) {
        while (!queue.emplace(i)) {
            fail += 1;
        }
    }
    th.join();
    std::cout << "main exit, fails: [" << fail << "]" << std::endl;

    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start);
    std::cout << "millis elapsed: " << elapsed.count() << std::endl;
}

#endif	/* STATICLIB_CONCURRENT_TEST_SUPPORT_HPP */

