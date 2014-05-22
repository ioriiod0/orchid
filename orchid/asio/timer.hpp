// ====================================================================================
// Copyright (c) 2012, ioriiod0@gmail.com All rights reserved.
// File         : timer.hpp
// Author       : ioriiod0@gmail.com
// Last Change  : 11/19/2012 04:15 PM
// Description  : 
// ====================================================================================


#include <iostream>
#include <boost/asio.hpp>
#include <boost/assert.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>

#include "io_service.hpp"
#include "throw_error.hpp"
#include "handlers.hpp"
#include "../utility/debug.hpp"

namespace orchid { namespace detail {


class timer_basic:public boost::asio::deadline_timer {
public:
    typedef io_service io_service_type;
    // typedef timer_basic<Coroutine> self_type;
    // typedef Coroutine coroutine_type;
    // typedef typename coroutine_type::coroutine_pointer coroutine_pointer;


public:
    timer_basic(io_service_type& io_s):boost::asio::deadline_timer(io_s) {

    }

    ~timer_basic() {

    }
public:

    template <typename CO>
    void sleep(std::size_t ms,CO co,boost::system::error_code& e) {
        BOOST_ASSERT(co != NULL);
        if (ms == 0) {
            return;
        }
        timer_handler<CO> handler(co,e);
        expires_from_now(boost::posix_time::milliseconds(ms));
        async_wait(handler);
        //////////////////////////
        co -> yield();
        /////////////////////////
        if(e) {
            ORCHID_DEBUG("timer sleep error: %s",e.message().c_str());
        }        
        return;
    }

    template <typename CO>
    void sleep(std::size_t ms,CO co) {
        BOOST_ASSERT(co != NULL);
        boost::system::error_code e;
        sleep(ms,co,e);
        if(e) {
            // throw boost::system::system_error(e);
            throw_error(e,"timer wait");
        }
    }


};






} }