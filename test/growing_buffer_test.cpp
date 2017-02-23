/* 
 * File:   growing_buffer_test.cpp
 * Author: alex
 *
 * Created on February 23, 2017, 10:34 AM
 */

#include "staticlib/concurrent/condition_latch.hpp"

#include <atomic>
#include <iostream>
#include <memory>
#include <thread>

#include "staticlib/config/assert.hpp"
#include "staticlib/concurrent/growing_buffer.hpp"

namespace sc = staticlib::concurrent;

void test_copy() {
    sc::growing_buffer foo{"foO", 3};
    slassert(3 == foo.size());
    
    sc::growing_buffer bar = std::move(foo);
    sc::growing_buffer baz{"a", 1};
    baz = std::move(foo);
    
    slassert(3 == foo.size());
    slassert('f' == *foo.data());
    slassert('o' == *(foo.data() + 1));
    slassert('O' == *(foo.data() + 2));
    slassert('f' == *bar.data());
    slassert('o' == *(bar.data() + 1));
    slassert('O' == *(bar.data() + 2));
    slassert('f' == *baz.data());
    slassert('o' == *(baz.data() + 1));
    slassert('O' == *(baz.data() + 2));
}

int main() {
    try {
        test_copy();
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        return 1;
    }
    return 0;
}
