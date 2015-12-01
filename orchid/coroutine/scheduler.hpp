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
#include <boost/function.hpp>
#include <boost/atomic.hpp>

#include "stack_allocator.hpp"
#include "../utility/debug.hpp"

namespace orchid { namespace detail {

template <template <class,class> class Coroutine,typename IOservice,typename Alloc = stack_allocator>
class scheduler_basic:boost::noncopyable {
public:
    typedef scheduler_basic<Coroutine,IOservice,Alloc> self_type;
    typedef IOservice io_service_type;
    typedef Alloc allocator_type;
    typedef Coroutine<self_type,allocator_type> coroutine_type;
    typedef boost::shared_ptr<coroutine_type> coroutine_pointer;
    typedef boost::context::fcontext_t context_type;
public:
    static boost::atomic<unsigned long> scheduler_id_gen_;

public:
	scheduler_basic() :id_(scheduler_id_gen_.fetch_add(1)), coroutine_id_gen_(0) {
        ORCHID_DEBUG("sche %lu: scheduler_basic()",id());
    }
    ~scheduler_basic() {
        ORCHID_DEBUG("sche %lu: ~scheduler_basic()",id());
        typename std::set<coroutine_pointer>::iterator it;
        for(it = all_.begin();it != all_.end();++it) {
            (*it) -> stop();
            ORCHID_DEBUG("sche %lu: clear coroutine %lu",id(),(*it)->id());
            boost::context::jump_fcontext(&ctx_,(*it)->ctx(),(intptr_t)((*it).get()));
            BOOST_ASSERT((*it) -> is_dead());
        }
        all_.clear();
    }

public:

    void run() {
        ORCHID_DEBUG("sche %lu run",id_);
        io_service_.run();
    }

    void stop() {
        io_service_.stop();
        // typename std::set<coroutine_pointer>::iterator it;
        // for(it = all_.begin();it != all_.end();++it) {
        //     (*it) -> stop();
        // }
    }

    template <typename F>
    void spawn(const F& f,std::size_t stack_size = coroutine_type::default_stack_size()) {
        coroutine_pointer co(new coroutine_type(*this,coroutine_id_gen_.fetch_add(1),f,stack_size));
        io_service_.post(boost::bind(&self_type::do_spawn,this,co));
        ORCHID_DEBUG("sche %lu spawn done",id_);
    }

    void resume(coroutine_pointer co) {
        BOOST_ASSERT(co != NULL);
        do_schedule(co);
    }

    template <typename F>
    void post(const F& f) {
        io_service_.post(f);
    }

    io_service_type& get_io_service() {
        return io_service_;
    }

    const io_service_type& get_io_service() const {
        return io_service_;
    }
    /////////////////////////////////////////////////

    context_type& ctx() {
        return ctx_;
    }

    const context_type& ctx() const {
        return ctx_;
    }

    unsigned long id() const {
        return id_;
    }

private:

    void do_schedule(coroutine_pointer co) {
        //jump to coroutine
        ORCHID_DEBUG("sche:%lu,coroutine:%lu,resume",id(),co->id());
        boost::context::jump_fcontext(&ctx_,co->ctx(),(intptr_t)(co.get()));
        if(co -> is_dead()) {
            all_.erase(co);
        }
    }

    void do_spawn(coroutine_pointer co) {
        ORCHID_DEBUG("sche %lu do spawn",id_);
        all_.insert(co);
        do_schedule(co);
    }

private:
    const unsigned long id_;
    boost::atomic<unsigned long> coroutine_id_gen_;
    context_type ctx_;
    std::set<coroutine_pointer> all_;
    io_service_type io_service_;
};



template <template <class,class> class Coroutine,typename IOservice,typename Alloc>
boost::atomic<unsigned long> scheduler_basic<Coroutine,IOservice,Alloc>::scheduler_id_gen_(0);


} }

#endif
