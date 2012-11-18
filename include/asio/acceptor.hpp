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

#include "socket.hpp"

namespace orchid { namespace detail {

template <typename Coroutine>
class acceptor_basic {
public:
    typedef socket_basic<Coroutine> socket_type;
    typedef boost::asio::io_service io_service_type;
    typedef acceptor_basic<Coroutine> self_type;
    typedef Coroutine coroutine_type;
    typedef coroutine_type* coroutine_pointer;
    typedef boost::asio::ip::tcp::acceptor impl_type;
    typedef boost::asio::ip::tcp::resolver resolver_type;

    struct accept_handler {
        accept_handler(coroutine_pointer co,boost::system::error_code& e):
            co_(co),e_(e) {
        }

        void operator()(const boost::system::error_code& e) {
            e_ = e;
            co_ -> resume();
        }

        coroutine_pointer co_;
        boost::system::error_code& e_;
    };

public:
    acceptor_basic(io_service_type& io_s):
        acceptor_(io_s),coroutine_(NULL),resolver_(io_s) {

    }

    acceptor_basic(io_service_type& io_s,coroutine_pointer p):
       acceptor_(io_s),coroutine_(p),resolver_(io_s) {

    }

    ~acceptor_basic() {
        if(acceptor_.is_open())
            acceptor_.close();
    }
public:
    void bind_and_listen(const std::string& port,
                        bool reuse_addr = false) {
        boost::asio::ip::tcp::resolver::query query(boost::asio::ip::tcp::v4(),port);
        boost::asio::ip::tcp::endpoint endpoint = *resolver_.resolve(query);
        acceptor_.open(endpoint.protocol());
        if(reuse_addr) {
            acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
        }
        acceptor_.bind(endpoint);
        acceptor_.listen();
    }

    socket_type* accept() {
        BOOST_ASSERT(coroutine_ != NULL);
        socket_type* p = new socket_type(acceptor_.get_io_service());
        boost::system::error_code e;
        accept_handler handler(coroutine_,e);
        acceptor_.async_accept(p -> get_impl(), handler);
        ///////////////////////////////////
        coroutine_ -> yield();
        ////////////////////////////////////
        if(e) {
            throw boost::system::system_error(e);
        }
        return p;
    }

    void attach(coroutine_pointer p) {
        BOOST_ASSERT(p != NULL);
        coroutine_ = p;
    }

    bool is_attached() const {
        return coroutine_ != NULL;
    }

    void is_open() const {
        return acceptor_.is_open();
    }

    impl_type& get_impl() {
        return acceptor_;
    }

    const impl_type& get_impl() const {
        return acceptor_;
    }

    void close() {
        acceptor_.close();
    }

private:
    impl_type acceptor_;
    coroutine_pointer coroutine_;
    resolver_type resolver_;
};

}
}


#endif



