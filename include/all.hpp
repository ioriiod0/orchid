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
#define USE_BOOST_CONTEXT
#define USE_BOOST_IOSTREAMS


#include <memory>

#ifdef USE_BOOST_CONTEXT
#include "coroutine/scheduler.hpp"
#include "coroutine/coroutine.hpp"
#include "coroutine/semaphore.hpp"
#include "coroutine/chan.hpp"
#endif

#ifdef USE_BOOST_ASIO
#include <boost/asio.hpp>
#include "asio/connector.hpp"
#include "asio/acceptor.hpp"
#include "asio/socket.hpp"
#endif

#ifdef USE_BOOST_IOSTREAMS
#include "utility/iostreams_helper.hpp"
#endif

namespace orchid {
    

typedef detail::scheduler_basic<detail::coroutine_basic,boost::asio::io_service> scheduler;
typedef detail::coroutine_basic<scheduler> coroutine;
typedef detail::semaphore_basic<coroutine> semaphore;
typedef detail::socket_basic<coroutine> socket;
typedef detail::acceptor_basic<coroutine> acceptor;
typedef detail::connector_basic<coroutine> connector;
typedef detail::tcp_device_basic<socket> tcp_device;
//////trick for template class typedef../////
template <typename T,typename Alloc = std::allocator<T> >
class chan:public detail::chan_basic<T,semaphore,Alloc> {
public:
    typedef chan<T,Alloc> self_type;
public:
    chan(std::size_t capacity):detail::chan_basic<T,semaphore,Alloc>(capacity) {

    }
    ~chan() {

    }
}; 


}

#endif
