/* 
 * File:   mpmc_blocking_queue_test.cpp
 * Author: alex
 *
 * Created on February 21, 2017, 1:03 PM
 */

#include "staticlib/concurrent/mpmc_blocking_queue.hpp"

#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <sstream>

#include "staticlib/config/assert.hpp"
#include "staticlib/config/to_string.hpp"

#include "test_support.hpp"

namespace sc = staticlib::concurrent;

const uint32_t ELEMENTS_COUNT = 1 << 10;

class test_string_generator {
    uint32_t counter = 0;

public:

    std::string generate(uint32_t size) {
        return std::string(size, staticlib::config::to_string(counter++)[0]);
    }
};

class my_movable_str {
    std::string val;
public:

    my_movable_str(std::string val) : val(val) { }

    const std::string& get_val() const {
        return val;
    }

    my_movable_str(const my_movable_str&) = delete;
    my_movable_str& operator=(const my_movable_str&) = delete;

    my_movable_str(my_movable_str&& other) :
    val(other.val) {
        other.val = "";
    };

    my_movable_str& operator=(my_movable_str&& other) {
        this->val = other.val;
        other.val = "";
        return *this;
    }
};

template<typename T, size_t Size>
class Maker {
public:
    using queue_type = sc::mpmc_blocking_queue<T>;

    std::shared_ptr<queue_type> make_queue() {
        return std::make_shared<queue_type>(Size);
    }
};

void test_take() {
    test_string_generator gen{};
    std::vector<std::string> data{};
    sc::mpmc_blocking_queue<my_movable_str> queue{};
    for (size_t i = 0; i < ELEMENTS_COUNT; i++) {
        std::string str = gen.generate(42);
        data.push_back(str);
        queue.emplace(std::move(str));
    }
    std::thread consumer([&] {
        for (size_t i = 0; i < ELEMENTS_COUNT; i++) {
            my_movable_str el{""};
            bool success = queue.take(el);
            slassert(success);
            slassert(el.get_val() == data[i]);
        }
    });
    consumer.join();
}

void test_intermittent() {
    sc::mpmc_blocking_queue<my_movable_str> queue{};
    std::thread producer([&] {
        test_string_generator gen{};
        for (size_t i = 0; i < 10; i++) {
            std::string str = gen.generate(42);
                    queue.emplace(std::move(str));
        }
        std::this_thread::sleep_for(std::chrono::milliseconds{200});
        for (size_t i = 10; i < 20; i++) {
            std::string str = gen.generate(42);
                    queue.emplace(std::move(str));
        }
        std::this_thread::sleep_for(std::chrono::milliseconds{300});
        for (size_t i = 20; i < ELEMENTS_COUNT; i++) {
            std::string str = gen.generate(42);
                    queue.emplace(std::move(str));
        }
    });
    std::thread consumer([&] {
        for (size_t i = 0; i < ELEMENTS_COUNT; i++) {
            my_movable_str el{""};
            bool success = queue.take(el);
            slassert(success);
            slassert(42 == el.get_val().size());
        }
    });
    producer.join();
    consumer.join();
}

void test_multi() {
    sc::mpmc_blocking_queue<my_movable_str> queue{};
    auto take = [&](size_t count) {
        for (size_t i = 0; i < count; i++) {
            my_movable_str el{""};
            bool success = queue.take(el);
            slassert(success);
            slassert(42 == el.get_val().size());
        }
    };
    auto put = [&](size_t count) {
        test_string_generator gen{};
        for (size_t i = 0; i < count; i++) {
            std::string str = gen.generate(42);
            queue.emplace(std::move(str));
        }
    };
    std::thread producer1(put, 100);
    std::thread producer2(put, 100);
    std::thread producer3(put, 100);
    std::thread consumer1(take, 50);
    std::thread consumer2(take, 50);
    std::thread consumer3(take, 50);
    std::thread consumer4(take, 50);
    std::thread consumer5(take, 50);
    std::thread consumer6(take, 50);
    producer1.join();
    producer2.join();
    producer3.join();
    consumer1.join();
    consumer2.join();
    consumer3.join();
    consumer4.join();
    consumer5.join();
    consumer6.join();
}

void test_poll() {
    test_string_generator gen{};
    std::vector<std::string> data{};
    sc::mpmc_blocking_queue<my_movable_str> queue{};
    for (size_t i = 0; i < ELEMENTS_COUNT; i++) {
        std::string str = gen.generate(42);
        data.push_back(str);
        queue.emplace(std::move(str));
    }
    std::thread consumer([&] {
        for (size_t i = 0; i < ELEMENTS_COUNT; i++) {
            my_movable_str el{""};
            bool success = queue.poll(el);
            slassert(success);
            slassert(el.get_val() == data[i]);
        }
        my_movable_str el_fail{""};
        bool success = queue.poll(el_fail);
                slassert(!success);
    });
    consumer.join();
}

void test_take_wait() {
    sc::mpmc_blocking_queue<my_movable_str> queue{};
    std::thread producer([&queue] {
        std::this_thread::sleep_for(std::chrono::milliseconds{200});
        queue.emplace("aaa");
                std::this_thread::sleep_for(std::chrono::milliseconds{200});
        queue.emplace("bbb");
    });
    std::thread consumer([&queue] {
        // not yet available
        my_movable_str el1{""};
        bool success1 = queue.take(el1, std::chrono::milliseconds(100));
                slassert(!success1);
                slassert("" == el1.get_val());
                // first received
                my_movable_str el2{""};
        bool success2 = queue.take(el2, std::chrono::milliseconds(150));
                slassert(success2);
                slassert("aaa" == el2.get_val());
                // wait for next
                std::this_thread::sleep_for(std::chrono::milliseconds{200});
        // should be already there
        my_movable_str el3{""};
        bool success3 = queue.take(el3, std::chrono::milliseconds(10));
                slassert(success3);
                slassert("bbb" == el3.get_val());
    });

    producer.join();
    consumer.join();
}

void test_threshold() {
    test_string_generator gen{};
    std::vector<std::string> data{};
    sc::mpmc_blocking_queue<my_movable_str> queue{ELEMENTS_COUNT};
    for (size_t i = 0; i < ELEMENTS_COUNT; i++) {
        std::string str = gen.generate(42);
        data.push_back(str);
        queue.emplace(std::move(str));
    }
    bool emplaced = queue.emplace("");
    slassert(!emplaced);
    std::thread consumer([&] {
        for (size_t i = 0; i < ELEMENTS_COUNT; i++) {
            my_movable_str el{""};
            bool success = queue.take(el);
            slassert(success);
            slassert(el.get_val() == data[i]);
        }
        my_movable_str dest{""};
        auto got_it = queue.poll(dest);
                slassert(!got_it);
    });
    consumer.join();
}

void test_unblock() {
    sc::mpmc_blocking_queue<my_movable_str> queue{};
    std::thread consumer([&] {
        my_movable_str el{""};
        bool success = queue.poll(el);
                slassert(!success);
                slassert(el.get_val() == "");
    });
    // ensure lock
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    queue.unblock();
    consumer.join();
}

void test_integral() {
    sc::mpmc_blocking_queue<int> queue{};
    int a = 42;
    int b = 43;
    int& b_ref = b;
    queue.emplace(41);
    queue.emplace(a);
    queue.emplace(b_ref);
    slassert(3 == queue.size());
    int taken;
    slassert(queue.poll(taken));
    slassert(41 == taken);
    slassert(queue.poll(taken));
    slassert(42 == taken);
    slassert(queue.poll(taken));
    slassert(43 == taken);
    slassert(!queue.poll(taken));
}

void test_emplace_range() {
    sc::mpmc_blocking_queue<my_movable_str> queue;
    std::vector<my_movable_str> vec;
    vec.emplace_back("foo");
    vec.emplace_back("bar");
    vec.emplace_back("baz");
    queue.emplace_range(std::move(vec));
    slassert(3 == queue.size());
    my_movable_str dest{""};
    slassert(queue.take(dest));
    slassert("foo" == dest.get_val());
    slassert(queue.take(dest));
    slassert("bar" == dest.get_val());
    slassert(queue.take(dest));
    slassert("baz" == dest.get_val());
}

void test_poll_consume() {
    sc::mpmc_blocking_queue<my_movable_str> queue;
    queue.emplace("foo");
    queue.emplace("bar");
    queue.emplace("baz");
    std::vector<my_movable_str> vec;
    bool polled = queue.poll([&vec](my_movable_str&& el){
        vec.emplace_back(std::move(el));
    });
    slassert(polled);
    slassert(3 == vec.size());
    slassert("foo" == vec[0].get_val());
    slassert("bar" == vec[1].get_val());
    slassert("baz" == vec[2].get_val());
}

void test_common() {
    test_correctness<Maker < std::string, 0xfffe >> ();
    test_correctness<Maker<int, 0xfffe >> ();
    test_correctness<Maker<unsigned long long, 0xfffe >> ();

    // slow with valgrind
//    test_perf<Maker<std::string, 0xfffe>> ();
//    test_perf<Maker<int, 0xfffe>> ();
//    test_perf<Maker<unsigned long long, 0xfffe>> ();

    test_destructor<Maker<dtor_checker, 1024>>();
    test_destructor_wrapped<Maker<dtor_checker, 4>> ();
    test_empty_full<Maker<int, 3>> ();

    test_wait<Maker<std::string, 1>> ();
}

int main() {
    try {
        test_take();
        test_intermittent();
        test_multi();
        test_poll();
        test_take_wait();
        test_threshold();
        test_unblock();
        test_integral();
        test_emplace_range();
        test_poll_consume();
        test_common();
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        return 1;
    }
    return 0;
}

