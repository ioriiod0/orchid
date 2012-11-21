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
#include "coroutine/scheduler_group.hpp"


#include "asio/connector.hpp"
#include "asio/acceptor.hpp"
#include "asio/socket.hpp"
#include "asio/timer.hpp"
#include "asio/io_service.hpp"


#ifdef USE_BOOST_IOSTREAMS
#include "utility/iostreams_helper.hpp"
#endif

namespace orchid {
    

typedef detail::scheduler_basic<detail::coroutine_basic,detail::io_service> scheduler;
typedef detail::scheduler_group_basic<scheduler> scheduler_group;
typedef detail::coroutine_basic<scheduler> coroutine;
typedef detail::socket_basic<coroutine> socket;
typedef detail::acceptor_basic<coroutine> acceptor;
typedef detail::connector_basic<coroutine> connector;
typedef detail::timer_basic<coroutine> timer;
typedef detail::stream_device_basic<socket> tcp_device;
//////trick for template class typedef../////
template <typename T,std::size_t N>
class chan:public detail::chan_basic<coroutine,T,N> {
public:
    typedef chan<T,N> self_type;
public:
    chan():detail::chan_basic<coroutine,T,N>() {

    }
    ~chan() {

    }
}; 


}

#endif
