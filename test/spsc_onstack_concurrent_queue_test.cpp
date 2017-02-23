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

template<typename T, size_t Size>
class Maker {
public:
    using queue_type = sc::spsc_onstack_concurrent_queue<T, Size>;

    std::shared_ptr<queue_type> make_queue() {
        return std::make_shared<queue_type>();
    }
};

int main() {
    try {
        //        slow with valgrind
//        test_correctness<Maker<std::string, 256>> ();
//        test_correctness<Maker<int, 256>> ();
//        test_correctness<Maker<unsigned long long, 256>> ();
//        test_perf<Maker<std::string, 1024>> ();
//        test_perf<Maker<int, 1024>> ();
//        test_perf<Maker<unsigned long long, 1024>> ();
        test_destructor<Maker<dtor_checker, 1024>> ();
        test_destructor_wrapped<Maker<dtor_checker, 4>> ();
        test_empty_full<Maker<int, 3>> ();
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        return 1;
    }
    return 0;
}
