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

#include "stack_allocator.hpp"

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
    scheduler_basic() {

    }
    ~scheduler_basic() {
        stop();
        typename std::set<coroutine_pointer>::iterator it;
        for(it = all_.begin();it != all_.end();++it) {
            boost::context::jump_fcontext(&ctx_,&((*it)->ctx()),(intptr_t)((*it).get()));
            BOOST_ASSERT((*it) -> is_dead());
        }
        all_.clear();
    }

public:

    void run() {
        io_service_.run();
    }

    void stop() {
        io_service_.stop();
        typename std::set<coroutine_pointer>::iterator it;
        for(it = all_.begin();it != all_.end();++it) {
            (*it) -> stop();
        }
    }

    template <typename F>
    void spawn(const F& f,std::size_t stack_size = coroutine_type::default_stack_size()) {
        coroutine_pointer co(new coroutine_type(*this,f,stack_size));
        io_service_.post(boost::bind(&self_type::do_spawn,this,co));
    }

    void resume(coroutine_pointer co) {
        BOOST_ASSERT(co != NULL);
        io_service_.post(boost::bind(&self_type::do_schedule,this,co));
    }

    template <typename F>
    void post(const F& f) {
        io_service_.post(f);
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

    void do_schedule(coroutine_pointer co) {
        //jump to coroutine
        boost::context::jump_fcontext(&ctx_,&co->ctx(),(intptr_t)(co.get()));
        if(co -> is_dead()) {
            all_.erase(co);
        }
    }

    void do_spawn(coroutine_pointer co) {
        all_.insert(co);
        do_schedule(co);
    }

private:
    context_type ctx_;
    std::set<coroutine_pointer> all_;
    io_service_type io_service_;
};


} }

#endif
