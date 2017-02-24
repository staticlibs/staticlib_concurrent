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
 * File:   growing_buffer.hpp
 * Author: alex
 *
 * Created on February 23, 2017, 10:07 AM
 */

#ifndef STATICLIB_CONCURRENT_GROWING_BUFFER_HPP
#define	STATICLIB_CONCURRENT_GROWING_BUFFER_HPP

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <memory>

namespace staticlib {
namespace concurrent {

namespace detail_growing_buffer {

class free_deleter {
public:
    void operator()(char* data) {
        std::free(data);
    }
};

}

class growing_buffer {
    size_t length;
    std::unique_ptr<char, detail_growing_buffer::free_deleter> buf;

public:    
    growing_buffer(const char* buffer, size_t length) :
    length(length),
    buf(static_cast<char*> (std::malloc(length)), detail_growing_buffer::free_deleter()) {
        if (nullptr == buf.get()) {
            throw std::bad_alloc();
        }
        std::memcpy(data(), buffer, length);
    }
    
    growing_buffer(const growing_buffer& other) :
    length(other.size()),
    buf(static_cast<char*> (std::malloc(length)), detail_growing_buffer::free_deleter()) {
        if (nullptr == buf.get()) {
            throw std::bad_alloc();
        }
        std::memcpy(data(), other.data(), length);
    }
    
    growing_buffer& operator=(const growing_buffer& other) {
        if (length < other.length) {
            buf.reset(static_cast<char*> (std::malloc(other.length)));
            if (nullptr == buf.get()) {
                throw std::bad_alloc();
            }
        }
        length = other.length;
        std::memcpy(buf.get(), other.data(), length);
        return *this;
    }
    
    char* data() {
        return buf.get();
    }

    const char* data() const {
        return buf.get();
    }
    
    size_t size() const {
        return length;
    }
};

} // namespace
}

#endif	/* STATICLIB_CONCURRENT_GROWING_BUFFER_HPP */

