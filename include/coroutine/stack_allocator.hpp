// ====================================================================================
// Copyright (c) 2012, ioriiod0@gmail.com All rights reserved.
// File         : stack_allocator.hpp
// Author       : ioriiod0@gmail.com
// Last Change  : 11/26/2012 04:38 PM
// Description  : 
// ====================================================================================
#ifndef __ORCHID_STACK_ALLOCATOR_H__
#define __ORCHID_STACK_ALLOCATOR_H__

#include <stdexcept>
#include <memory>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <iostream>
#include <cmath>
#include <boost/assert.hpp>
#include <boost/context/all.hpp>
// #include <cstddef>
// #include <cstdlib>



namespace orchid { namespace detail {

class stack_allocator {
public:
    static std::size_t page_size() {
        return sysconf(_SC_PAGESIZE);
    }

    static std::size_t default_stack_size() {
        std::size_t size =  16*page_size();
        std::size_t max_size = maximum_stack_size();
        return size < max_size ? size:max_size;
    }

    static std::size_t maximum_stack_size() {
        struct rlimit rlim;
        BOOST_ASSERT(getrlimit(RLIMIT_STACK, &rlim) >= 0);
        return rlim.rlim_cur;
    }

    static std::size_t minimum_stack_size() {
        return 2*page_size();
    }

    static std::size_t adjust_stack_size(std::size_t stacksize) {
        std::size_t min = minimum_stack_size();
        std::size_t max = maximum_stack_size();
        BOOST_ASSERT(min<max);
        std::size_t pagesize = page_size();
        if (stacksize < min) {
            stacksize = min;
        }
        stacksize = stacksize + sizeof(boost::context::fcontext_t) + 15;
        stacksize = ((stacksize + pagesize- 1) & ~(pagesize - 1)) +pagesize;
        return stacksize < max ? stacksize : max;
    }
public:
    void* allocate(std::size_t stacksize) {
        void* p = ::mmap( 0, stacksize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
        if(!p) {
            throw std::bad_alloc();
        }
        std::size_t pagesize = page_size();
        if (mprotect(p, pagesize, PROT_NONE) < 0) {
            munmap(p, stacksize);
            throw std::bad_alloc();
        }
        return static_cast<char*>(p) + stacksize;
    }

    void deallocate(void* p,std::size_t stacksize) {
        void * q = static_cast<char*>(p) - stacksize;
        munmap(q, stacksize);
    }

};
 
}}

#endif