// ====================================================================================
// Copyright (c) 2012, ioriiod0@gmail.com All rights reserved.
// File         : stream_descriptor.hpp
// Author       : ioriiod0@gmail.com
// Last Change  : 11/27/2012 10:15 PM
// Description  : 
// ====================================================================================
#ifndef __ORCHID_STREAM_DESCRIPTOR_H__
#define __ORCHID_STREAM_DESCRIPTOR_H__

#include <boost/asio.hpp>
#include <boost/utility.hpp>
#include "io_service.hpp"

namespace orchid { namespace detail {

template <typename Coroutine>
class descriptor_basic:boost::noncopyable {
public:
    typedef boost::asio::posix::stream_descriptor impl_type;
    typedef typename impl_type::native_handle_type native_handle_type;
    typedef io_service io_service_type;
    typedef descriptor_basic<Coroutine> self_type;
    typedef Coroutine coroutine_type;
    typedef boost::shared_ptr<coroutine_type> coroutine_pointer;

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
    descriptor_basic(io_service_type& io_s)
        :io_service_(io_s),descriptor_(io_s.get_impl()) {

    }


    descriptor_basic(io_service_type& io_s,const native_handle_type& handle)
        :io_service_(io_s),descriptor_(io_s.get_impl(),handle) {

    }


    ~descriptor_basic() {
        //if(descriptor_.is_open())
        descriptor_.close();
    }
    
public:

    std::size_t read(char* data,std::size_t size,coroutine_pointer co) {
        BOOST_ASSERT(co != NULL);
        boost::system::error_code e;
        std::size_t bytes_transferred;
        io_handler handler(co,e,bytes_transferred);
        descriptor_.async_read_some(boost::asio::buffer(data,size), handler);
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
        descriptor_.async_write_some(boost::asio::buffer(data,size), handler);
        //////////////////////////////////
        co->yield();
        //////////////////////////////////
        if (e) {
            throw boost::system::system_error(e);
        }
        return bytes_transferred; 
    }

    bool is_open() {
        return descriptor_.is_open();
    }

    impl_type& get_impl() {
        return descriptor_;
    }

    const impl_type& get_impl() const {
        return descriptor_;
    }

    void close() {
        descriptor_.close();
    }

    const io_service_type& get_io_service() const {
        return io_service_;
    }

    io_service_type& get_io_service() {
        return io_service_;
    }

    void assign(const native_handle_type& handle) {
        descriptor_.assign(handle);
    }

    native_handle_type release() {
        return descriptor_.release();
    }

    native_handle_type native_handle() {
        return descriptor_.native_handle();
    }

private:
    io_service_type& io_service_;
    impl_type descriptor_;


};


}}


#endif