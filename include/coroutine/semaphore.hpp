// ====================================================================================
// Copyright (c) 2012, ioriiod0@gmail.com All rights reserved.
// File         : semaphore.hpp
// Author       : ioriiod0@gmail.com
// Last Change  : 11/18/2012 09:09 PM
// Description  : 
// ====================================================================================

#include <iostream>
#include <queue>

namespace orchid { namespace detail {


template <typename Coroutine>
class semaphore_basic 
{
public:
    typedef Coroutine coroutine_type;
    typedef coroutine_type* coroutine_pointer;
    typedef semaphore_basic<Coroutine> self_type;
    typedef std::queue<coroutine_pointer> queue_type;

public:
    semaphore_basic(std::size_t n):n_(n) {

    }
    ~semaphore_basic() {

    }
public:
    int N() {
        return n_;
    }

    void P(coroutine_pointer co) {
        --n_;
        if(n_ < 0) {
            queues_.push(co);
            co -> yield();
        }
    }

    void V(coroutine_pointer co) {
        ++n_;
        if(n_ <= 0) {
            queues_.front() -> resume();
            queues_.pop();
        }
    }
private:
    int n_;
    queue_type queues_;

};



}
}