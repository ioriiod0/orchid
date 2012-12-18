// ====================================================================================
// Copyright (c) 2012, ioriiod0@gmail.com All rights reserved.
// File         : chan.hpp
// Author       : ioriiod0@gmail.com
// Last Change  : 11/18/2012 01:35 AM
// Description  : 
// ====================================================================================
#include <iostream>
#include <memory>
#include <queue>

#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_array.hpp>

#include "spinlock.hpp"


namespace orchid { namespace detail {

////////////////////chan类型模拟了GOLANG中chan的概念/////////////////


template <typename Coroutine,typename T>
class chan_basic:boost::noncopyable 
{
public:
    typedef chan_basic<Coroutine,T> self_type;
    typedef T value_type;
    typedef Coroutine coroutine_type;
    typedef boost::shared_ptr<coroutine_type> coroutine_pointer;
public:
    chan_basic(std::size_t cap)
        :cap_(cap),array_(new T[cap]),r_(0),
        w_(0),is_closed_(false) {
    }
    ~chan_basic() {
        close();
    }
public:

    void close() {
        locker_.lock();
        is_closed_ = true;
        while(!r_queue_.empty()){
            r_queue_.front() -> resume();
            r_queue_.pop();
        }
        while(!w_queue_.empty()){
            w_queue_.front() -> resume();
            w_queue_.pop();
        }
        locker_.unlock();
    }

    template <typename U>
    bool send(const U& t,coroutine_pointer co) {
        locker_.lock();
        if(is_closed_) {
            locker_.unlock();
            return false;
        }
        while(size_ >= cap_) {
            w_queue_.push(co);
            locker_.unlock();
            co -> yield();
            locker_.lock();
            if(is_closed_) {
                locker_.unlock();
                return false;
            }
        }
        array_[w_++] = t;
        if(w_ == cap_) w_ = 0;
        ++size_;
        if(!r_queue_.empty()) {
            coroutine_pointer co = r_queue_.front();
            r_queue_.pop();
            locker_.unlock();
            co -> resume();
        } else {
            locker_.unlock();
        }
        return true;
    }

    template <typename U>
    bool recv(U& t,coroutine_pointer co) {
        locker_.lock();
        if(is_closed_) {
            locker_.unlock();
            return false;
        }
        while(size_ <= 0) {
            r_queue_.push(co);
            locker_.unlock();
            co -> yield();
            locker_.lock();
            if(is_closed_) {
                locker_.unlock();
                return false;
            }
        }
        t = array_[r_++];
        if(r_ == cap_) r_ = 0;
        --size_;
        if(!w_queue_.empty()) {
            coroutine_pointer co = w_queue_.front();
            w_queue_.pop();
            locker_.unlock();
            co -> resume();
        } else {
            locker_.unlock();
        }
        return true;
    }



private:
    spinlock locker_;
    std::size_t cap_;
    int size_;
    boost::scoped_array<T> array_;
    std::size_t r_;
    std::size_t w_;
    bool is_closed_;
    std::queue<coroutine_pointer> r_queue_;
    std::queue<coroutine_pointer> w_queue_;
    
};



}
}