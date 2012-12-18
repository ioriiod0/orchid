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

#include "io_service.hpp"
#include "socket.hpp"


namespace orchid { namespace detail {

template <typename Coroutine>
class acceptor_basic {
public:
    typedef socket_basic<Coroutine> socket_type;
    typedef io_service io_service_type;
    typedef acceptor_basic<Coroutine> self_type;
    typedef Coroutine coroutine_type;
    typedef boost::shared_ptr<coroutine_type> coroutine_pointer;
    typedef boost::asio::ip::tcp::acceptor impl_type;
    typedef typename impl_type::native_handle_type native_handle_type;

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
        io_service_(io_s),acceptor_(io_s.get_impl()) {

    }

    ~acceptor_basic() {
        close();
    }
public:
    void bind_and_listen(const std::string& port,
                        bool reuse_addr = false) {
        boost::asio::ip::tcp::resolver resolver(acceptor_.get_io_service());
        boost::asio::ip::tcp::resolver::query query(boost::asio::ip::tcp::v4(),port);
        boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);
        acceptor_.open(endpoint.protocol());
        if(reuse_addr) {
            acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
        }
        acceptor_.bind(endpoint);
        acceptor_.listen();
    }

    void accept(socket_type& sock,coroutine_pointer co) {
        BOOST_ASSERT(co != NULL);
        boost::system::error_code e;
        accept_handler handler(co,e);
        acceptor_.async_accept(sock.get_impl(), handler);
        ///////////////////////////////////
        co -> yield();
        ////////////////////////////////////
        if(e) {
            throw boost::system::system_error(e);
        }
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

    native_handle_type native_handle() {
        return acceptor_.native_handle();
    }

    void close() {
        //if(acceptor_.is_open())
        acceptor_.close();
    }

    const io_service_type& get_io_service() const {
        return io_service_;
    }

    io_service_type& get_io_service() {
        return io_service_;
    }

private:
    io_service_type& io_service_;
    impl_type acceptor_;
};

}
}


#endif



