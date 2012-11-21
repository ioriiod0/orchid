// ====================================================================================
// Copyright (c) 2012, ioriiod0@gmail.com All rights reserved.
// File         : coroutine.hpp
// Author       : ioriiod0@gmail.com
// Last Change  : 11/08/2012 12:34 AM
// Description  : 
// ====================================================================================

#ifndef __ORCHID_COROUTINE_H__
#define __ORCHID_COROUTINE_H__

#include <iostream>
#include <boost/assert.hpp>
#include <boost/context/all.hpp>
#include <boost/function.hpp>
#include <boost/utility.hpp>

namespace orchid { namespace detail {



template <typename Scheduler>
class coroutine_basic:boost::noncopyable {
public:
    typedef boost::ctx::fcontext_t context_type;
    typedef Scheduler scheduler_type;
    typedef typename Scheduler::io_service_type io_service_type;
    typedef coroutine_basic<Scheduler> self_type;

    static void trampoline(intptr_t q) 
    {
        self_type* p = (self_type*)q;
        p -> run();
        p -> is_dead_ = true;
        boost::ctx::jump_fcontext(&p->ctx(),&p->sche_.ctx(),0); //NOTITY SCHEDULER that coroutine is dead
    }

public:

    coroutine_basic(Scheduler& s):is_dead_(false),sche_(s) {
        void* st = alloc_.allocate(boost::ctx::default_stacksize());
        ctx_.fc_stack.base = st;
        ctx_.fc_stack.limit = static_cast<char*>(st) - boost::ctx::default_stacksize();
        boost::ctx::make_fcontext(&ctx_, trampoline);
    }

    virtual ~coroutine_basic() {
        alloc_.deallocate(ctx_.fc_stack.base, boost::ctx::default_stacksize());
    }

public:

    virtual void run() = 0;

    void yield() {
        boost::ctx::jump_fcontext(&ctx_,&sche_.ctx(),0);
    }

    void resume() {
        sche_.resume(this);
    }

    scheduler_type& get_scheduler() {
        return sche_;
    }

    const scheduler_type& get_scheduler() const {
        return sche_;
    }

    bool is_dead() const {
        return is_dead_;
    }

    context_type& ctx() {
        return ctx_;
    }

    const context_type& ctx() const {
        return ctx_;
    }

private:
    bool is_dead_;
    context_type ctx_;
    boost::ctx::stack_allocator alloc_;
    scheduler_type& sche_;
};


} }


#endif
