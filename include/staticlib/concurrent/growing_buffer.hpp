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

/**
 * Non-shrinkable `char` heap buffer with non-destructive `move` (the same as `copy`) logic,
 * grows if needed on `move-in` operation
 */
class growing_buffer {
    size_t length;
    std::unique_ptr<char, detail_growing_buffer::free_deleter> buf;

public:
    /**
     * Constructor, creates empty buffer
     */
    growing_buffer() :
    length(0) { }
    
    /**
     * Copy constructor
     * 
     * @param other instance
     */
    growing_buffer(const growing_buffer& other) :
    length(other.size()),
    buf(static_cast<char*> (std::malloc(length)), detail_growing_buffer::free_deleter()) {
        if (nullptr == buf.get()) {
            throw std::bad_alloc();
        }
        std::memcpy(data(), other.data(), length);
    }
    
    /**
     * Copy assignment operator
     * 
     * @param other instance
     * @return this instance
     */
    growing_buffer& operator=(const growing_buffer& other) {
        resize(other.length);
        std::memcpy(buf.get(), other.data(), length);
        return *this;
    }

    /**
     * Resize buffer extending data storage if necessary
     * 
     * @param new_size new size
     */
    void resize(size_t new_size) {
        if (new_size > length) {
            buf.reset(static_cast<char*> (std::malloc(new_size)));
            if (nullptr == buf.get()) {
                throw std::bad_alloc();
            }
        }
        length = new_size;
    }
    
    /**
     * Pointer to stored data
     * 
     * @return pointer to stored data
     */
    char* data() {
        return buf.get();
    }

    /**
     * Pointer to stored data
     * 
     * @return pointer to stored data
     */
    const char* data() const {
        return buf.get();
    }
    
    /**
     * Buffer size
     * 
     * @return buffer size
     */
    size_t size() const {
        return length;
    }   
};

} // namespace
}

#endif	/* STATICLIB_CONCURRENT_GROWING_BUFFER_HPP */

