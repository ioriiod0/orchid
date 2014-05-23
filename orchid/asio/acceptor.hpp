// ====================================================================================
// Copyright (c) 2012, ioriiod0@gmail.com All rights reserved.
// File         : acceptor.hpp
// Author       : ioriiod0@gmail.com
// Last Change  : 11/16/2012 05:58 PM
// Description  : 
// ====================================================================================

#ifndef __ORCHID_ACCEPTOR_H__
#define __ORCHID_ACCEPTOR_H__
#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/assert.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>
// #include <boost/lexical_cast.hpp>

#include "io_service.hpp"
#include "socket.hpp"
#include "throw_error.hpp"
#include "handlers.hpp"
#include "../utility/debug.hpp"


namespace orchid { namespace detail {

class acceptor_basic:public boost::asio::ip::tcp::acceptor {
public:
    typedef socket_basic socket_type;
    typedef io_service io_service_type;
    // typedef acceptor_basic<Coroutine> self_type;
    // typedef Coroutine coroutine_type;
    // typedef typename coroutine_type::coroutine_pointer coroutine_pointer;


public:
    acceptor_basic(io_service_type& io_s):boost::asio::ip::tcp::acceptor(io_s) {

    }

    ~acceptor_basic() {
        
    }
public:
    void bind_and_listen(const std::string& port,bool reuse_addr) {
        boost::asio::ip::tcp::resolver resolver(get_io_service());
        boost::asio::ip::tcp::resolver::query query(boost::asio::ip::tcp::v4(),port);
        boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);
        open(endpoint.protocol());
        if(reuse_addr) {
            set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
        }
        bind(endpoint);
        listen(boost::asio::socket_base::max_connections);
    }

    void bind_and_listen(const std::string& port,bool reuse_addr,boost::system::error_code& e) {
        try {
            bind_and_listen(port,reuse_addr);
        } catch (const boost::system::system_error& ex) {
            e = ex.code();
            ORCHID_DEBUG("bind_and_listen error: %s",e.message().c_str());
        }
        
    }


    template <typename CO>
    void accept(socket_type& sock,CO co,boost::system::error_code& e) {
        BOOST_ASSERT(co != NULL);
        accept_handler<CO> handler(co,e);
        async_accept(sock, handler);
        ///////////////////////////////////
        co -> yield();
        ////////////////////////////////////
        if(e) {
            ORCHID_DEBUG("acceptor accept error: %s",e.message().c_str());
        }
        return;
    }

    template <typename CO>
    void accept(socket_type& sock,CO co) {
        BOOST_ASSERT(co != NULL);
        boost::system::error_code e;
        accept(sock,co,e);
        ////////////////////////////////////
        if(e) {
            throw_error(e,"acceptor accept");
        }
    }


};

}
}


#endif



