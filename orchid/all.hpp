// ====================================================================================
// Copyright (c) 2012, ioriiod0@gmail.com All rights reserved.
// File         : all.hpp
// Author       : ioriiod0@gmail.com
// Last Change  : 11/16/2012 10:12 PM
// Description  : 
// ====================================================================================

#ifndef __ORCHID_ALL_H__
#define __ORCHID_ALL_H__

#define USE_BOOST_ASIO

#include <memory>

#include "coroutine/scheduler.hpp"
#include "coroutine/coroutine.hpp"
#include "coroutine/chan.hpp"
#include "coroutine/stack_allocator.hpp"
#include "coroutine/scheduler_group.hpp"

#include "asio/acceptor.hpp"
#include "asio/socket.hpp"
#include "asio/timer.hpp"
#include "asio/io_service.hpp"
#include "asio/descriptor.hpp"
#include "asio/signal.hpp"
#include "asio/io_interface.hpp"
#include "asio/io_funcs.hpp"
#include "asio/throw_error.hpp"
#include "asio/worker.hpp"

#include "utility/debug.hpp"

// #if defined(__linux__)
// #endif

namespace orchid {

typedef detail::scheduler_basic<detail::coroutine_basic,detail::io_service> scheduler;
typedef detail::scheduler_group_basic<scheduler> scheduler_group;
typedef detail::coroutine_basic<scheduler> coroutine;
typedef coroutine::coroutine_handle coroutine_handle;
typedef detail::socket_basic socket;
typedef detail::acceptor_basic acceptor;
typedef detail::timer_basic timer;
typedef detail::descriptor_basic descriptor;
typedef detail::signal_basic signal;
typedef detail::io_error io_error;
typedef detail::worker_pool_basic worker_pool;


template <typename Input>
class reader:public detail::reader_basic<coroutine_handle,Input> {
public:
    reader(Input& input,coroutine_handle co)
        :detail::reader_basic<coroutine_handle,Input>(input,co) {

        }

    ~reader() {

    }
};


template <typename Output>
class writer:public detail::writer_basic<coroutine_handle,Output> {
public:
    writer(Output& output,coroutine_handle co)
        :detail::writer_basic<coroutine_handle,Output>(output,co) {

        }

    ~writer() {

    }

};

template <typename Input,typename Alloc=std::allocator<char> >
class buffered_reader:public detail::buffered_reader_basic<coroutine_handle,Input,Alloc> {
public:
    buffered_reader(Input& input,coroutine_handle co,std::size_t size=detail::buffered_reader_basic<coroutine_handle,Input,Alloc>::default_size)
        :detail::buffered_reader_basic<coroutine_handle,Input,Alloc>(input,co) {

    }

    ~buffered_reader() {

    }

};

template <typename Output,typename Alloc=std::allocator<char> >
class buffered_writer:public detail::buffered_writer_basic<coroutine_handle,Output,Alloc> {
public:
    buffered_writer(Output& output,coroutine_handle co,std::size_t size=detail::buffered_reader_basic<coroutine_handle,Output,Alloc>::default_size)
        :detail::buffered_writer_basic<coroutine_handle,Output,Alloc>(output,co) {

    }

    ~buffered_writer() {

    }

};


template <typename T>
class chan:public detail::chan_basic<coroutine_handle,T> {
public:
    typedef chan<T> self_type;
public:
    chan(std::size_t cap)
        :detail::chan_basic<coroutine_handle,T>(cap) {

    }
    ~chan() {

    }
};


}

#endif
