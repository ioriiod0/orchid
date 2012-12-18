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
#include <iostream>

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
    typedef boost::shared_ptr<coroutine_type> coroutine_pointer;
    typedef typename impl_type::native_handle_type native_handle_type;

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

    struct connect_handler {
        connect_handler(coroutine_pointer co,
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
    socket_basic(io_service_type& io_s):
        io_service_(io_s),socket_(io_s.get_impl()) {
        //printf("socket_con\r\n");
    }

    ~socket_basic() {
        //close();
        //socket_.close();
        //printf("socket_des\r\n");
    }
public:

    void connect(const std::string& host,const std::string& port,coroutine_pointer co) {
        BOOST_ASSERT(co != NULL);
        boost::system::error_code e;
        boost::asio::ip::tcp::resolver resolver(socket_.get_io_service());
        boost::asio::ip::tcp::resolver::iterator start;

        connect_handler r_handler(co,e,start);
        boost::asio::ip::tcp::resolver::query query(host,port);
        resolver.async_resolve(query, r_handler);
        //start = resolver.resolve(query);
        ///////////////////////////////////////////
        co -> yield();
        ////////////////////////////////////////////
        if(e) {
            throw boost::system::system_error(e);
        }
        /////////////////////////////////////////
        connect_handler c_handler(co,e,start);
        boost::asio::async_connect(socket_,start, c_handler);
        co -> yield();
        ////////////////////////////////////
        if(e) {
            throw boost::system::system_error(e);
        }
    }


    std::size_t read(char* data,std::size_t size,coroutine_pointer co) {
        BOOST_ASSERT(co != NULL);
        boost::system::error_code e;
        std::size_t bytes_transferred;
        io_handler handler(co,e,bytes_transferred);
        socket_.async_read_some(boost::asio::buffer(data,size), handler);
        //////////////////////////////////
        co->yield();
        //////////////////////////////////
        if (e) {
            throw boost::system::system_error(e);
        }
        return bytes_transferred; 
    }

    std::size_t write(const char* data,std::size_t size,coroutine_pointer co) {
        BOOST_ASSERT(co != NULL);
        boost::system::error_code e;
        std::size_t bytes_transferred;
        io_handler handler(co,e,bytes_transferred);
        socket_.async_write_some(boost::asio::buffer(data,size), handler);
        //////////////////////////////////
        co->yield();
        //////////////////////////////////
        if (e) {
            throw boost::system::system_error(e);
        }
        return bytes_transferred; 
    }

    bool is_open() {
        return socket_.is_open();
    }

    impl_type& get_impl() {
        return socket_;
    }

    const impl_type& get_impl() const {
        return socket_;
    }

    void close() {
        // socket_.shutdown();
        // if(socket_.is_open()) {
        socket_.close();
        // printf("socket_closed!!\r\n");
            // std::cout<<"socket_closed!!"<<std::endl;
        // }
    }

    const io_service_type& get_io_service() const {
        return io_service_;
    }

    io_service_type& get_io_service() {
        return io_service_;
    }

    native_handle_type native_handle() {
        return socket_.native_handle();
    }


private:
    io_service_type& io_service_;
    impl_type socket_;

};


}
}



#endif