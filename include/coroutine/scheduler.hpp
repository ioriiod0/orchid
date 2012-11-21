// ====================================================================================
// Copyright (c) 2012, ioriiod0@gmail.com All rights reserved.
// File         : scheduler.hpp
// Author       : ioriiod0@gmail.com
// Last Change  : 11/08/2012 12:35 AM
// Description  : 
// ====================================================================================
#ifndef __ORCHID_SCHEDULER_H__
#define __ORCHID_SCHEDULER_H__

#include <set>
#include <algorithm>
#include <iostream>

#include <boost/assert.hpp>
#include <boost/function.hpp>
#include <boost/context/all.hpp>
#include <boost/utility.hpp>
#include <boost/bind.hpp>


namespace orchid { namespace detail {

template <template <class> class Coroutine,typename IOservice>
class scheduler_basic:boost::noncopyable {
public:
    typedef boost::ctx::fcontext_t context_type;
    typedef scheduler_basic<Coroutine,IOservice> self_type;
    typedef Coroutine<self_type> coroutine_type;
    typedef IOservice io_service_type;

    struct deleter {
        void operator()(coroutine_type* p) {
            if(p)
                delete p;
        }
    };
public:
    scheduler_basic() {

    }
    ~scheduler_basic() {
        io_service_.stop();
        std::for_each(all_.begin(), all_.end(), deleter());
    }

public:

    void run() {
        // for (;;) {
        //     schedule_all();
        //     if(0 == io_service_.run_one())
        //         io_service_.reset();
        // }
        io_service_.run();
    }

    void stop() {
        io_service_.stop();
    }

    void spawn(coroutine_type* co) {
        BOOST_ASSERT(co != NULL);
        all_.insert(co);
        resume(co);
    }

    void resume(coroutine_type* co) {
        BOOST_ASSERT(co != NULL);
        io_service_.post(boost::bind(&self_type::do_schedule,this,co));
    }

    std::size_t size() const {
        return all_.size();
    }

    io_service_type& get_io_service() {
        return io_service_;
    }

    const io_service_type& get_io_service() const {
        return io_service_;
    }

    context_type& ctx() {
        return ctx_;
    }

    const context_type& ctx() const {
        return ctx_;
    }
private:

    // void schedule_all() {
    //     while(!readys_.empty()) {
    //         do_schedule(readys_.front());
    //         readys_.pop_front();
    //     }
    // }

    // void schedule_one() {
    //     if(!readys_.empty()) {
    //         do_schedule(readys_.front());
    //         readys_.pop_front();
    //     }
    // }

    void do_schedule(coroutine_type* co) {
        //jump to coroutine
        boost::ctx::jump_fcontext(&ctx_,&co->ctx(),(intptr_t)co);
        if(co -> is_dead()) {
            delete co;
            all_.erase(co);
        }
    }
private:
    context_type ctx_;
    //std::deque<coroutine_type*> readys_;
    std::set<coroutine_type*> all_;
    io_service_type io_service_;
};


} }

#endif
