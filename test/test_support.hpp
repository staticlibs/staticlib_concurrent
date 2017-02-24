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
 * File:   test_support.hpp
 * Author: alex
 *
 * Created on February 21, 2017, 2:13 PM
 */

#ifndef STATICLIB_CONCURRENT_TEST_SUPPORT_HPP
#define	STATICLIB_CONCURRENT_TEST_SUPPORT_HPP

// source: https://github.com/facebook/folly/blob/b75ef0a0af48766298ebcc946dd31fe0da5161e3/folly/test/ProducerConsumerQueueTest.cpp

#include <cstdlib>
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
        return std::rand() % 26;
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

template<typename QueueMaker>
struct perf_test_type {
    typedef typename QueueMaker::queue_type QueueType;
    typedef typename QueueType::value_type T;

    std::shared_ptr<QueueType> queue_;
    std::atomic<bool> done_;
    test_traits<T> traits_;
    
    explicit perf_test_type() : 
    queue_(QueueMaker().make_queue()), 
    done_(false) { }

    void run() {
        using namespace std::chrono;
//        auto const startTime = system_clock::now();
        std::thread producer([this] {
            this->producer();
        });
        std::thread consumer([this] {
            this->consumer();
        });
        producer.join();
        done_ = true;
        consumer.join();
//        auto duration = duration_cast<milliseconds>(system_clock::now() - startTime);
//        std::cout << " done: " << duration.count() << "ms" << std::endl;
    }

    void producer() {
        // This is written differently than you might expect so that
        // it does not run afoul of -Wsign-compare, regardless of the
        // signedness of this loop's upper bound.
        for (auto i = traits_.limit(); i > 0; --i) {
            while (!queue_->emplace(traits_.generate())) {
            }
        }
    }

    void consumer() {
        while (!done_) {
            T data;
            queue_->poll(data);
        }
    }
};

template<typename QueueMaker>
static void test_perf() {
    perf_test_type<QueueMaker>().run();
}

//template<class TestType> void do_test(const char* name) {
//    std::cout << " testing: " << name << std::endl;
//    std::unique_ptr<TestType> const t(new TestType());
//    (*t)();
//}

//template<class Queue, size_t Size, bool Pop = false >
//void perf_test_type(const char* type) {
//    const size_t size = Size;
//    std::cout << "Type: " << type << std::endl;
//    do_test<perf_test<Queue, size> >("ProducerConsumerQueue");
//}

template<typename QueueMaker>
struct correctness_test_type {
    typedef typename QueueMaker::queue_type QueueType;
    typedef typename QueueType::value_type T;

    std::vector<T> testData_;
    std::shared_ptr<QueueType> queue_;
    test_traits<T> traits_;
    std::atomic<bool> done_;

    explicit correctness_test_type() :
    queue_(QueueMaker().make_queue()),
    done_(false) {
        const size_t testSize = static_cast<size_t> (traits_.limit());
        testData_.reserve(testSize);
        for (size_t i = 0; i < testSize; ++i) {
            testData_.push_back(traits_.generate());
        }
    }

    void run() {
        std::thread producer([this] {
            this->producer(); 
        });
        std::thread consumer([this] {
            this->consumer(); 
        });
        producer.join();
        done_ = true;
        consumer.join();
    }

    void producer() {
        for (auto& data : testData_) {
            while (!queue_->emplace(data)) {
            }
        }
    }

    void consumer() {
        for (auto expect : testData_) {
        again:
            T data;
            if (!queue_->poll(data)) {
                if (done_) {
                    // Try one more read; unless there's a bug in the queue class
                    // there should still be more data sitting in the queue even
                    // though the producer thread exited.
                    if (!queue_->poll(data)) {
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

template<typename QueueMaker>
static void test_correctness() {
    correctness_test_type<QueueMaker>().run();
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

template<typename QueueMaker>
void test_destructor() {
    // Test that orphaned elements in a ProducerConsumerQueue are
    // destroyed.
    {        
        auto queue = QueueMaker().make_queue();
        for (int i = 0; i < 10; ++i) {
            slassert(queue->emplace(dtor_checker()));
        }
        slassert(dtor_checker::numInstances == 10);
        {
            dtor_checker ignore;
            slassert(queue->poll(ignore));
            slassert(queue->poll(ignore));
        }
        slassert(dtor_checker::numInstances == 8);
    }
    slassert(dtor_checker::numInstances == 0);

}

template<typename QueueMaker>
void test_destructor_wrapped() {
    // Test the same thing in the case that the queue write pointer has
    // wrapped, but the read one hasn't.
    {
        auto queue = QueueMaker().make_queue();
        for (int i = 0; i < 3; ++i) {
            slassert(queue->emplace(dtor_checker()));
        }
        slassert(dtor_checker::numInstances == 3);
        {
            dtor_checker ignore;
            slassert(queue->poll(ignore));
        }
        slassert(dtor_checker::numInstances == 2);
        slassert(queue->emplace(dtor_checker()));
        slassert(dtor_checker::numInstances == 3);
    }
    slassert(dtor_checker::numInstances == 0);
}

template<typename QueueMaker>
void test_empty_full() {
    auto queue_ptr = QueueMaker().make_queue();
    auto& queue = *queue_ptr;
    slassert(queue.empty());
    slassert(!queue.full());
    slassert(queue.emplace(1));
    slassert(!queue.empty());
    slassert(!queue.full());
    slassert(queue.emplace(2));
    slassert(!queue.empty());
    slassert(queue.emplace(3));
    slassert(queue.full());
    slassert(queue.size() == 3);
}

template<typename QueueMaker>
void test_wait() {
    std::atomic<bool> flag{false};
    auto queue = QueueMaker().make_queue();
    auto th = std::thread([queue, &flag] {
        std::string dest;
        bool taken = queue->take(dest);
                slassert(taken);
                slassert("foo" == dest);
                flag.store(true, std::memory_order_release);
    });
    slassert(!flag.load(std::memory_order_acquire));
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    slassert(!flag.load(std::memory_order_acquire));
    bool emplaced = queue->emplace("foo");
    slassert(emplaced);
    th.join();
    slassert(flag.load(std::memory_order_acquire));
}

template<typename QueueMaker>
void test_poll_get_free_slots() {
    auto queue = QueueMaker().make_queue();
    std::string dest;
    slassert(0 == queue->poll_get_free_slots(dest));
    slassert(queue->emplace("foo"));
    slassert(queue->emplace("bar"));
    slassert(2 == queue->size());
    slassert(2 == queue->poll_get_free_slots(dest));
    slassert(1 == queue->size());
    slassert(1 == queue->poll_get_free_slots(dest));
    slassert(queue->empty());
    slassert(0 == queue->poll_get_free_slots(dest));
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

template<typename Queue>
void test_speed_wait(Queue& queue) {
    auto start = std::chrono::system_clock::now();
    auto th = std::thread([&queue] {
        size_t i = 0;
        size_t sum = 0;
        while (i < 10000000) {
            queue.take(i);
            sum += i;
        }
        std::cout << "worker exit, sum: [" << sum << "]" << std::endl;
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

