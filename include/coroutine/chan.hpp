// ====================================================================================
// Copyright (c) 2012, ioriiod0@gmail.com All rights reserved.
// File         : chan.hpp
// Author       : ioriiod0@gmail.com
// Last Change  : 11/18/2012 01:35 AM
// Description  : 
// ====================================================================================
#include <iostream>
#include <memory>

namespace orchid { namespace detail {

template <typename T,typename Semaphore,
            typename Alloc = std::allocator<T> >
class chan_basic
{
public:
    typedef T value_type;
    typedef Alloc allocator_type;
    typedef Semaphore semaphore_type;
    typedef typename semaphore_type::coroutine_type coroutine_type;
    typedef typename semaphore_type::coroutine_pointer coroutine_pointer;
    typedef chan_basic<T,Semaphore,Alloc> self_type;
public:
    chan_basic(std::size_t capacity)
        :capacity_(capacity),size_(0),
        r_semaphore_(0),w_semaphore_(capacity_) {
        data_ = alloc_.allocate(capacity);
    }
    ~chan_basic() {
        alloc_.deallocate(data_,capacity_);
    }
public:
    void put(const value_type& t,coroutine_pointer co) {
            w_semaphore_.P(co);
            data_[w_++] = t;
            if(w_ == capacity_) w_ = 0;
            ++size_;
            r_semaphore_.V(co);
        
    }

    void get(value_type& v,coroutine_pointer co) {
        r_semaphore_.P(co);
        v = data_[r_++];
        if(r_ == capacity_) r_ = 0;
        --size_;
        w_semaphore_.V(co);
    }

    std::size_t size() {
        return size_;
    }

    std::size_t capacity() {
        return capacity_;
    }

private:

    allocator_type alloc_;
    std::size_t capacity_;
    std::size_t size_;
    T* data_;
    std::size_t r_;
    std::size_t w_;
    semaphore_type r_semaphore_;
    semaphore_type w_semaphore_;


};

}
}