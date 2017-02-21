/* 
 * File:   spsc_onstack_concurrent_queue_test.cpp
 * Author: alex
 *
 * Created on February 21, 2017, 2:28 PM
 */

#include "staticlib/concurrent/spsc_onstack_concurrent_queue.hpp"

#include <iostream>

#include "staticlib/config/assert.hpp"

#include "test_support.hpp"

namespace sc = staticlib::concurrent;

int main() {
    try {
        correctness_test_type<sc::spsc_onstack_concurrent_queue<std::string, 256>, 256> ("string");
        correctness_test_type<sc::spsc_onstack_concurrent_queue<int, 256>, 256>("int");
        correctness_test_type<sc::spsc_onstack_concurrent_queue<unsigned long long, 256>, 256>("unsigned long long");
        perf_test_type<sc::spsc_onstack_concurrent_queue<std::string, 256>, 256> ("string");
        perf_test_type<sc::spsc_onstack_concurrent_queue<int, 256>, 256>("int");
        perf_test_type<sc::spsc_onstack_concurrent_queue<unsigned long long, 256>, 256>("unsigned long long");
        test_destructor<sc::spsc_onstack_concurrent_queue<dtor_checker, 1024>>();
        test_empty_full<sc::spsc_onstack_concurrent_queue<int, 3>>();
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        return 1;
    }
    return 0;
}
