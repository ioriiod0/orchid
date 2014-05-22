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


template <typename CO,typename T>
class chan_basic:boost::noncopyable 
{
public:
    typedef chan_basic<CO,T> self_type;
    typedef T value_type;
    typedef CO coroutine_pointer;
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
        if (is_closed_) {
            locker_.unlock();
            return;
        }

        is_closed_ = true;
        while(!r_queue_.empty()){
            r_queue_.front() -> sche_resume();
            r_queue_.pop();
        }
        while(!w_queue_.empty()){
            w_queue_.front() -> sche_resume();
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
            coroutine_pointer r = r_queue_.front();
            r_queue_.pop();
            locker_.unlock();
            r -> sche_resume();
        } else {
            locker_.unlock();
        }
        // co -> sche_resume();
        // co -> yield();
        return true;
    }

    template <typename U>
    bool recv(U& t,coroutine_pointer co) {
        locker_.lock();
        while(size_ <= 0) {

            if(is_closed_) {
                locker_.unlock();
                return false;
            }

            r_queue_.push(co);
            locker_.unlock();
            co -> yield();
            locker_.lock();
 
        }

        std::swap(t,array_[r_++]);
        //t = array_[r++];
        if(r_ == cap_) r_ = 0;
        --size_;

        if(!w_queue_.empty()) {
            coroutine_pointer w = w_queue_.front();
            w_queue_.pop();
            locker_.unlock();
            w -> sche_resume();
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