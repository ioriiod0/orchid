// ====================================================================================
// Copyright (c) 2012, ioriiod0@gmail.com All rights reserved.
// File         : signal.hpp
// Author       : ioriiod0@gmail.com
// Last Change  : 12/13/2012 11:47 AM
// Description  : 
// ====================================================================================


#ifndef __ORCHID_SIGNAL_H__
#define __ORCHID_SIGNAL_H__

#include <boost/asio.hpp>
#include <boost/utility.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>

#include "io_service.hpp"
#include "throw_error.hpp"
#include "handlers.hpp"
#include "../utility/debug.hpp"


namespace orchid { namespace detail{

class signal_basic:public boost::asio::signal_set {
public:
    typedef io_service io_service_type;
    // typedef signal_basic<Coroutine> self_type;
    // typedef Coroutine coroutine_type;
    // typedef typename coroutine_type::coroutine_pointer coroutine_pointer;



public:
    signal_basic(io_service_type& io_s)
        :boost::asio::signal_set(io_s) {

    }
    ~signal_basic() {

    }
public:

    template <typename CO>
    int wait(CO co,boost::system::error_code& e) {
        int sig_num = 0;
        sig_handler<CO> handler(co,e,sig_num);
        async_wait(handler);
        /////////////////////////
        co -> yield();
        ///////////////////////////
        if(e) {
            ORCHID_DEBUG("signal wait error: %s",e.message().c_str());
        }
        return sig_num;
    }

    template <typename CO>
    int wait(CO co) {
        boost::system::error_code e;
        int sig_num = wait(co,e);
        if(e) {
            // throw boost::system::system_error(e);
            throw_error(e,"signal wait");
        }
        return sig_num;
    }

};
    
}}



#endif