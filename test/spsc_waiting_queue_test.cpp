/* 
 * File:   spsc_waiting_queue_test.cpp
 * Author: alex
 *
 * Created on February 21, 2017, 7:33 PM
 */

#include "staticlib/concurrent/spsc_waiting_queue.hpp"

#include <cstdlib>
#include <chrono>
#include <iostream>
#include <thread>

#include "staticlib/config/assert.hpp"

#include "test_support.hpp"

namespace sc = staticlib::concurrent;

void test_queue() {
    correctness_test_type<sc::spsc_waiting_queue<std::string>, 0xfffe> ("string");
    correctness_test_type<sc::spsc_waiting_queue<int>, 0xfffe>("int");
    correctness_test_type<sc::spsc_waiting_queue<unsigned long long>, 0xfffe>("unsigned long long");
    perf_test_type<sc::spsc_waiting_queue<std::string>, 0xfffe> ("string");
    perf_test_type<sc::spsc_waiting_queue<int>, 0xfffe>("int");
    perf_test_type<sc::spsc_waiting_queue<unsigned long long>, 0xfffe>("unsigned long long");
    test_destructor<sc::spsc_waiting_queue < dtor_checker >> ();
    test_empty_full<sc::spsc_waiting_queue<int>>();
}

int main() {
    try {
//        test_queue();
//        sc::spsc_waiting_queue<size_t> queue1(2);
//        test_speed(queue1);
        sc::spsc_waiting_queue<size_t> queue2(2);
        test_speed_wait(queue2);
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        return 1;
    }
    return 0;
}

