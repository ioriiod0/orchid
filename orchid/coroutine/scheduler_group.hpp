// ====================================================================================
// Copyright (c) 2012, ioriiod0@gmail.com All rights reserved.
// File         : scheduler_group_basic.hpp
// Author       : ioriiod0@gmail.com
// Last Change  : 11/21/2012 08:47 PM
// Description  : 
// ====================================================================================

#ifndef __ORCHID_SCHEDULER_GROUP_H__
#define __ORCHID_SCHEDULER_GROUP_H__

#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/scoped_array.hpp>
#include <boost/assert.hpp>
#include <boost/atomic.hpp>
// #include <boost/random.hpp>

namespace orchid { namespace detail {
template <typename Scheduler>
class scheduler_group_basic {
public:
    typedef Scheduler scheduler_type;
    typedef typename scheduler_type::io_service_type io_service_type;

public:
    scheduler_group_basic(std::size_t size)
        :size_(size),sches_(new scheduler_type[size]) {

    }
    ~scheduler_group_basic() {
    }
public:
    void run() {
        for(std::size_t i = 0;i < size_;++i) {
            tg_.create_thread(boost::bind(&scheduler_type::run,&sches_[i]));
        }
        tg_.join_all();
    }

    void stop() {
        for(std::size_t i=0;i<size_;++i) {
            sches_[i].stop();
        }
    }

    std::size_t size() {
        return size_;
    }

    //
    scheduler_type& operator[](std::size_t i) {
        return sches_[i];
    }

    const scheduler_type& operator[](std::size_t i) const {
        return sches_[i];
    }


private:

    std::size_t size_;
    boost::thread_group tg_;
    boost::scoped_array<scheduler_type> sches_;

};


}}


#endif
