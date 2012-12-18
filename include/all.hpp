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
#define USE_BOOST_IOSTREAMS


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

#include "utility/io_interface.hpp"

// #if defined(__linux__)
// #endif

namespace orchid {

typedef detail::scheduler_basic<detail::coroutine_basic,detail::io_service> scheduler;
typedef detail::scheduler_group_basic<scheduler> scheduler_group;
typedef detail::coroutine_basic<scheduler> coroutine;
typedef coroutine::coroutine_handle coroutine_handle;
typedef detail::socket_basic<coroutine> socket;
typedef detail::acceptor_basic<coroutine> acceptor;
typedef detail::timer_basic<coroutine> timer;
typedef detail::descriptor_basic<coroutine> descriptor;
typedef detail::signal_basic<coroutine> signal;
typedef detail::reader_basic<coroutine,socket> tcp_reader;
typedef detail::writer_basic<coroutine,socket> tcp_writer;
typedef detail::reader_basic<coroutine,descriptor> descriptor_reader;
typedef detail::writer_basic<coroutine,descriptor> descriptor_writer;
typedef boost::iostreams::stream<tcp_writer> tcp_ostream;
typedef boost::iostreams::stream<tcp_reader> tcp_istream;
typedef boost::iostreams::stream<descriptor_writer> descriptor_ostream;
typedef boost::iostreams::stream<descriptor_reader> descriptor_istream;

template <typename T>
class chan:public detail::chan_basic<coroutine,T> {
public:
    typedef chan<T> self_type;
public:
    chan(std::size_t cap)
        :detail::chan_basic<coroutine,T>(cap) {

    }
    ~chan() {

    }
};



}

#endif
