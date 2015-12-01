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
#include <boost/bind.hpp>
#include <boost/utility.hpp>
#include <boost/atomic.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "stack_allocator.hpp"
#include "scheduler.hpp"
#include "../utility/debug.hpp"

namespace orchid { namespace detail {


class stack_unwind_exception {
public:
    stack_unwind_exception() {

    }
    ~stack_unwind_exception() {

    }
};


template <typename Scheduler,typename Alloc = stack_allocator>
class coroutine_basic
    :public boost::enable_shared_from_this<coroutine_basic<Scheduler,stack_allocator> >,
    boost::noncopyable {
public:
    typedef coroutine_basic<Scheduler,Alloc> self_type;
    typedef Alloc allocator_type;
    typedef Scheduler scheduler_type;
    typedef typename Scheduler::io_service_type io_service_type;
    typedef boost::context::fcontext_t context_type;
    typedef boost::shared_ptr<self_type> coroutine_handle;
    typedef boost::shared_ptr<self_type> coroutine_pointer;
    typedef boost::function<void(coroutine_handle)> func_type;

    static std::size_t default_stack_size() {
        return allocator_type::default_stack_size();
    }

    static std::size_t minimum_stack_size() {
        return allocator_type::minimum_stack_size();
    }

    static std::size_t maximum_stack_size() {
        return allocator_type::maximum_stack_size();
    }

    static void trampoline(intptr_t q) 
    {
        self_type* p = (self_type*)q;
        ORCHID_DEBUG("sche:%lu,coroutine:%lu,start running",p->sche_id(),p->id());
        try {
            if(p->is_stoped()) {
                throw stack_unwind_exception();
            }
            p -> f_(p -> get_handle());
        } catch(const stack_unwind_exception& e) {
            //do nothing.
            ORCHID_DEBUG("sche:%lu,coroutine:%lu,stack unwind",p->sche_id(),p->id());
        }
        p -> is_dead_.store(true);
        ORCHID_DEBUG("sche:%lu,coroutine:%lu,dead",p->sche_id(),p->id());
        boost::context::jump_fcontext(&p->ctx(),p->sche_.ctx(),0); //NOTITY SCHEDULER that coroutine is dead
    }

public:

    template <typename F>
    coroutine_basic(Scheduler& s,unsigned long id,const F& f,std::size_t stack_size)
		:id_(id),
		is_dead_(false),
        is_stoped_(false),
        alloc_(),
        f_(f), sche_(s) {
            ORCHID_DEBUG("sche:%lu,coroutine:%lu,coroutine_basic",this->sche_id(),this->id());
            stack_size_ = allocator_type::adjust_stack_size(stack_size);
            stack_pointer_ = alloc_.allocate(stack_size_);
            ctx_ = boost::context::make_fcontext(stack_pointer_,stack_size_,trampoline);
    }

    ~coroutine_basic() {
        ORCHID_DEBUG("sche:%lu,coroutine:%lu,~coroutine_basic()",sche_id(),id());
        alloc_.deallocate(stack_pointer_,stack_size_);
    }

public:
    /////////////////////////////接口函数////////////////////////////////////////

    //放弃运行，切换至调度器的上下文中
    //该函数只应该在当前协程的run函数中调用
    void yield() {
        ORCHID_DEBUG("sche:%lu,coroutine:%lu,yield",sche_id(),id());
        BOOST_ASSERT(!is_dead());
        boost::context::jump_fcontext(&ctx(),sche_.ctx(),0);
        if(is_stoped()) {
            //std::cout<<"throw here"<<std::endl;
            throw stack_unwind_exception();
        }
    }


    //恢复当前协程的上下文，立即切换
    void resume() {
        if(is_stoped() || is_dead()) {
            ORCHID_DEBUG("sche:%lu,coroutine:%lu,resume in coroutine,stop:%s dead:%s",
                sche_id(),id(),is_stoped()?"true":"false",is_dead()?"true":"false");
            return;
        }
        // ORCHID_DEBUG("sche:%lu,coroutine:%lu,resume",id(),sche_id());
        sche_.resume(this -> shared_from_this());
    }

    //恢复当前协程的上下文，但是并不是立即切换，
    //而是将切换的请求在调度器的IO_SERVICE里面排队，等待切换回上次运行的现场
    void sche_resume() {
        if(is_stoped() || is_dead()) {
            ORCHID_DEBUG("sche:%lu,coroutine:%lu,shce_resume in coroutine,stop:%s dead:%s",
                sche_id(),id(),is_stoped()?"true":"false",is_dead()?"true":"false");
            return;
        }
        // ORCHID_DEBUG("sche:%lu,coroutine:%lu,sche_resume",id(),sche_id());
        sche_.post(boost::bind(&scheduler_type::resume,&sche_,this->shared_from_this()));
    }

    //获取当前协程的运行状态。
    //该函数是线程安全的。
    bool is_dead() {
        return is_dead_.load();
    }


    //获取当前协程所在的调度器。
    scheduler_type& get_scheduler() {
        return sche_;
    }

    const scheduler_type& get_scheduler() const {
        return sche_;
    }

    coroutine_handle get_handle() {
        return this -> shared_from_this();
    }

    void stop() {
        is_stoped_.store(true);
    }

    bool is_stoped() {
        return is_stoped_.load();
    }

    std::size_t stack_size() {
        return stack_size_;
    }

    unsigned long id() const {
        return id_;
    }

    unsigned long sche_id() const {
        return sche_.id();
    }

    io_service_type& get_io_service() {
        return sche_.get_io_service();
    }

    const io_service_type& get_io_service() const {
        return sche_.get_io_service();
    }


    ///////////////////////////////内部函数//////////////////////////////////////

    context_type& ctx() {
        return ctx_;
    }

    const context_type& ctx() const {
        return ctx_;
    }

private:
    const unsigned long id_;
    boost::atomic_bool is_dead_;
    boost::atomic_bool is_stoped_;
    allocator_type alloc_;
    std::size_t stack_size_;
    void* stack_pointer_;
    context_type ctx_;
    func_type f_;
    scheduler_type& sche_;
};


} }


#endif
