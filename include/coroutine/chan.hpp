// ====================================================================================
// Copyright (c) 2012, ioriiod0@gmail.com All rights reserved.
// File         : chan.hpp
// Author       : ioriiod0@gmail.com
// Last Change  : 11/18/2012 01:35 AM
// Description  : 
// ====================================================================================
#include <iostream>
#include <memory>
#include <boost/lockfree/ringbuffer.hpp>
#include <boost/lockfree/fifo.hpp>

namespace orchid { namespace detail {

template <typename Coroutine,typename T,std::size_t N>
class chan_basic
{
public:
    const static std::size_t Capacity = N;
    typedef chan_basic<Coroutine,T,N> self_type;
    typedef T value_type;
    typedef Coroutine coroutine_type;
    typedef coroutine_type* coroutine_pointer;
public:
    chan_basic() {
    }
    ~chan_basic() {
    }
public:
    void enqueue(const value_type& t,coroutine_pointer co) {
        BOOST_ASSERT(co != NULL);
        //尝试向数据队列中放数据，如果放不进去，说明已经满了，则yield
        while(!datas_.enqueue(t)) {
            if(w_queue_.enqueue(co))
                co -> yield();
        }
        //成功的放入数据后，判断有没有读者阻塞，如果有的话尝试唤醒一个读者。
        while(!r_queue_.empty()) {
            coroutine_pointer tmp = NULL;
            if(r_queue_.dequeue(&tmp)) {
                tmp -> resume();
                break;
            }
        }
    }

    void dequeue(value_type& v,coroutine_pointer co) {
        BOOST_ASSERT(co != NULL);
        //尝试从数据队列中读数据，如果读不到，说明队列是空的，则yield
        while(!datas_.dequeue(&v)) {
            if(r_queue_.enqueue(co))
                co -> yield();
        }
        //取出数据后，判断有没有写者阻塞，如果有则尝试唤醒一个写者。
        while(!w_queue_.empty()) {
            coroutine_pointer tmp = NULL;
            if(w_queue_.dequeue(&tmp)) {
                tmp -> resume();
                break;
            }
        }
    }

private:
    boost::lockfree::fifo<coroutine_pointer> r_queue_;
    boost::lockfree::fifo<coroutine_pointer> w_queue_;
    boost::lockfree::ringbuffer<T,128> datas_;

};

}
}