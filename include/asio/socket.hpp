// ====================================================================================
// Copyright (c) 2012, ioriiod0@gmail.com All rights reserved.
// File         : socket.hpp
// Author       : ioriiod0@gmail.com
// Last Change  : 11/13/2012 04:04 PM
// Description  : 
// ====================================================================================

#ifndef __ORCHID_SOCKET_H__
#define __ORCHID_SOCKET_H__

#include <string>

#include <boost/assert.hpp>
#include <boost/asio.hpp>
#include <boost/utility.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>

#include "io_service.hpp"


namespace orchid { namespace detail {


template <typename Coroutine>
class socket_basic:boost::noncopyable
{
public:
    ////////////////////////////////////////
    typedef boost::asio::ip::tcp::socket impl_type;
    typedef io_service io_service_type;
    typedef socket_basic<Coroutine> self_type;
    typedef Coroutine coroutine_type;
    typedef coroutine_type* coroutine_pointer;

    struct io_handler {
        io_handler(coroutine_pointer co,boost::system::error_code& e,std::size_t& bytes_transferred):
            e_(e),co_(co),bytes_transferred_(bytes_transferred) {
        }

        void operator()(const boost::system::error_code& e,std::size_t bytes_transferred){
            e_ = e;
            bytes_transferred_ = bytes_transferred;
            co_ -> resume();
        }

        boost::system::error_code& e_;
        coroutine_pointer co_;
        std::size_t& bytes_transferred_;
    };

public:
    socket_basic(io_service_type& io_s):
        io_service_(io_s),coroutine_(NULL),socket_(io_s.get_impl()) {

    }

    socket_basic(io_service_type& io_s,coroutine_pointer p):
        io_service_(io_s),coroutine_(p),socket_(io_s.get_impl()) {

    }

    ~socket_basic() {
        if(socket_.is_open())
            socket_.close();
    }
public:

    std::size_t read(char* data,std::size_t size) {
        BOOST_ASSERT(coroutine_ != NULL);
        boost::system::error_code e;
        std::size_t bytes_transferred;
        io_handler handler(coroutine_,e,bytes_transferred);
        socket_.async_read_some(boost::asio::buffer(data,size), handler);
        //////////////////////////////////
        coroutine_->yield();
        //////////////////////////////////
        if (e) {
            throw boost::system::system_error(e);
        }
        return bytes_transferred; 
    }

    std::size_t write(const char* data,std::size_t size) {
        BOOST_ASSERT(coroutine_ != NULL);
        boost::system::error_code e;
        std::size_t bytes_transferred;
        io_handler handler(coroutine_,e,bytes_transferred);
        socket_.async_write_some(boost::asio::buffer(data,size), handler);
        //////////////////////////////////
        coroutine_->yield();
        //////////////////////////////////
        if (e) {
            throw boost::system::system_error(e);
        }
        return bytes_transferred; 
    }

    void attach(coroutine_pointer p) {
        BOOST_ASSERT(p != NULL);
        coroutine_ = p;
    }

    bool is_attached() const {
        return coroutine_ != NULL;
    }

    void is_open() {
        return socket_.is_open();
    }

    impl_type& get_impl() {
        return socket_;
    }

    const impl_type& get_impl() const {
        return socket_;
    }

    void close() {
        socket_.close();
    }

    const io_service_type& get_io_service() const {
        return io_service_;
    }

    io_service_type& get_io_service() {
        return io_service_;
    }



private:
    io_service_type& io_service_;
    coroutine_pointer coroutine_;
    impl_type socket_;

};


}
}



#endif