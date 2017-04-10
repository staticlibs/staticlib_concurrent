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
 * File:   spsc_inobject_waiting_queue_test.cpp
 * Author: alex
 *
 * Created on February 21, 2017, 7:52 PM
 */

#include "staticlib/concurrent/spsc_inobject_waiting_queue.hpp"

#include <cstdlib>
#include <chrono>
#include <iostream>
#include <thread>

#include "staticlib/config/assert.hpp"

#include "test_support.hpp"

template<typename T, size_t Size>
class queue_maker {
public:
    using queue_type = sl::concurrent::spsc_inobject_waiting_queue<T, Size>;

    std::shared_ptr<queue_type> make_queue() {
        return std::make_shared<queue_type>();
    }
};

int main() {
    try {
        // slow with valgrind
//        test_correctness<queue_maker<std::string, 256>>();
//        test_correctness<queue_maker<int, 256>>();
//        test_correctness<queue_maker<unsigned long long, 256>>();
//        test_perf<queue_maker<std::string, 1024>>();
//        test_perf<queue_maker<int, 1024>>();
//        test_perf<queue_maker<unsigned long long, 1024>>();
                
        test_destructor<queue_maker<dtor_checker, 1024>>();
        test_destructor_wrapped<queue_maker<dtor_checker, 4>>();
        test_empty_full<queue_maker<int, 3>>();
        
        test_wait<queue_maker<std::string, 1>> ();
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        return 1;
    }
    return 0;
}

