// ====================================================================================
// Copyright (c) 2012, ioriiod0@gmail.com All rights reserved.
// File         : stream_descriptor.hpp
// Author       : ioriiod0@gmail.com
// Last Change  : 11/27/2012 10:15 PM
// Description  : 
// ====================================================================================
#ifndef __ORCHID_STREAM_DESCRIPTOR_H__
#define __ORCHID_STREAM_DESCRIPTOR_H__

#include <iostream>
#include <boost/asio.hpp>
#include <boost/utility.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>

#include "io_service.hpp"
#include "throw_error.hpp"
#include "handlers.hpp"
#include "../utility/debug.hpp"

namespace orchid { namespace detail {

class descriptor_basic:public boost::asio::posix::stream_descriptor {
public:
    typedef boost::asio::posix::stream_descriptor::native_handle_type native_handle_type;
    typedef io_service io_service_type;
    // typedef descriptor_basic<Coroutine> self_type;
    // typedef Coroutine coroutine_type;
    // typedef typename coroutine_type::coroutine_pointer coroutine_pointer;

public:
    descriptor_basic(io_service_type& io_s)
        :boost::asio::posix::stream_descriptor(io_s) {

    }


    descriptor_basic(io_service_type& io_s,const native_handle_type& handle)
        :boost::asio::posix::stream_descriptor(io_s,handle) {

    }


    ~descriptor_basic() {
        
    }
    
public:


    template <typename MutableBufferSequence,typename CO>
    std::size_t read_some(const MutableBufferSequence& buf,CO co,boost::system::error_code& e) {
        BOOST_ASSERT(co != NULL);
        std::size_t bytes_transferred;
        io_handler<CO> handler(co,e,bytes_transferred);
        async_read_some(buf, handler);
        //////////////////////////////////
        co->yield();
        //////////////////////////////////
        if (e) {
            ORCHID_DEBUG("description read error: %s",e.message().c_str());
        }
        return bytes_transferred; 
    }

    template <typename MutableBufferSequence,typename CO>
    std::size_t read_some(const MutableBufferSequence& buf,CO co) {
        BOOST_ASSERT(co != NULL);
        boost::system::error_code e;
        std::size_t bytes_transferred = read_some(buf,co,e);

        if (e) {
            // throw boost::system::system_error(e);
            throw_error(e,"descriptor read");
        }

        return bytes_transferred; 
    }


    template <typename ConstBufferSequence,typename CO>
    std::size_t write_some(const ConstBufferSequence& buf,CO co,boost::system::error_code& e) {
        BOOST_ASSERT(co != NULL);
        std::size_t bytes_transferred;
        io_handler<CO> handler(co,e,bytes_transferred);
        async_write_some(buf, handler);
        //////////////////////////////////
        co->yield();
        //////////////////////////////////
        if (e) {
            ORCHID_DEBUG("description write error: %s",e.message().c_str());
        }
        return bytes_transferred; 
    }


    template <typename ConstBufferSequence,typename CO>
    std::size_t write_some(const ConstBufferSequence& buf,CO co) {
        BOOST_ASSERT(co != NULL);
        boost::system::error_code e;
        std::size_t bytes_transferred = write_some(buf,co,e);
        if (e) {
            // throw boost::system::system_error(e);
            throw_error(e,"descriptor write");
        }

        return bytes_transferred; 
    }



};


}}


#endif