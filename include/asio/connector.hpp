// ====================================================================================
// Copyright (c) 2012, ioriiod0@gmail.com All rights reserved.
// File         : connector.hpp
// Author       : ioriiod0@gmail.com
// Last Change  : 11/16/2012 05:58 PM
// Description  : 
// ====================================================================================

#ifndef __ORCHID_CONNECTOR_H__
#define __ORCHID_CONNECTOR_H__
#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/assert.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>

#include "socket.hpp"

namespace orchid { namespace detail {

template <typename Coroutine>
class connector_basic {
public:
    typedef socket_basic<Coroutine> socket_type;
    typedef boost::asio::io_service io_service_type;
    typedef connector_basic<Coroutine> self_type;
    typedef Coroutine coroutine_type;
    typedef coroutine_type* coroutine_pointer;
    typedef boost::asio::ip::tcp::resolver resolver_type;

    // struct connect_handler {
    //     connect_handler(coroutine_pointer co,boost::system::error_code& e)
    //         :co_(co),e_(e) {
    //     }

    //     void operator()(const boost::system::error_code& e,) {
    //         e_ = e;
    //         co_ -> resume();
    //     }

    //     boost::system::error_code& e_;
    //     coroutine_pointer co_;
    // };

    struct io_handler {
        io_handler(coroutine_pointer co,
                    boost::system::error_code& e,
                    boost::asio::ip::tcp::resolver::iterator& it)
            :e_(e),co_(co),it_(it) {

        }

        void operator()(const boost::system::error_code& e,
                boost::asio::ip::tcp::resolver::iterator it) {
            it_ = it;
            e_ = e;
            co_ -> resume();
        }

        boost::system::error_code& e_;
        coroutine_pointer co_;
        boost::asio::ip::tcp::resolver::iterator& it_;

    };

public:
    connector_basic(io_service_type& io_s):resolver_(io_s),coroutine_(NULL) {

    }

    connector_basic(io_service_type& io_s,coroutine_pointer p)
        :resolver_(io_s),coroutine_(p) {

    }

    ~connector_basic() {

    }
public:

    socket_type* connect(const std::string& host,const std::string& port) {
        BOOST_ASSERT(coroutine_ != NULL);
        socket_type* p = new socket_type(resolver_.get_io_service());
        boost::system::error_code e;
        boost::asio::ip::tcp::resolver::iterator start;
        boost::asio::ip::tcp::resolver::iterator end;

        io_handler r_handler(coroutine_,e,start);
        boost::asio::ip::tcp::resolver::query query(host,port,
            boost::asio::ip::resolver_query_base::numeric_service);
        resolver_.async_resolve(query, r_handler);
        ///////////////////////////////////////////
        coroutine_ -> yield();
        ////////////////////////////////////////////
        if(e) {
            throw boost::system::system_error(e);
        }
        /////////////////////////////////////////
        e = boost::asio::error::host_not_found;
        io_handler c_handler(coroutine_,e,start);
        while (e && start != end)
        {
            p -> close();
            boost::asio::async_connect(p -> get_impl(),start, c_handler);
            coroutine_ -> yield();
            ++start;
        }
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

private:
    resolver_type resolver_;
    coroutine_pointer coroutine_;
};

}
}


#endif



