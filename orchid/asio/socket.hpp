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

#include "throw_error.hpp"
#include "io_service.hpp"
#include "handlers.hpp"
#include "../utility/debug.hpp"

namespace orchid { namespace detail {


class socket_basic: public boost::asio::ip::tcp::socket
{
public:
    ////////////////////////////////////////
    typedef io_service io_service_type;
    // typedef socket_basic<Coroutine> self_type;
    // typedef Coroutine coroutine_type;
    // typedef typename coroutine_type::coroutine_pointer coroutine_pointer;

public:
    socket_basic(io_service_type& io_s):boost::asio::ip::tcp::socket(io_s) {

    }

    ~socket_basic() {

    }
public:

	template <typename CO>
	void connect(const boost::asio::ip::tcp::endpoint& addr, CO co, boost::system::error_code& e) {
		BOOST_ASSERT(co != NULL);
		/////////////////////////////////////////
		connect_without_resolver_handler<CO> c_handler(co, e);
		async_connect(addr, c_handler);
		co->yield();
		////////////////////////////////////
		if (e) {
			ORCHID_DEBUG("socket connect error: %s", e.message().c_str());
		}
		return;
	}


	template <typename CO>
	void connect(const boost::asio::ip::tcp::endpoint& addr, CO co) {
		BOOST_ASSERT(co != NULL);
		boost::system::error_code e;
		connect(addr, co, e);
		if (e) {
			throw_error(e, "socket connect");
		}
	}

    template <typename CO>
    void connect(const std::string& host,const std::string& port,CO co,boost::system::error_code& e) {
        BOOST_ASSERT(co != NULL);
        boost::asio::ip::tcp::resolver resolver(get_io_service());
        boost::asio::ip::tcp::resolver::iterator start;

        connect_handler<CO> r_handler(co,e,start);
        boost::asio::ip::tcp::resolver::query query(host,port);
        resolver.async_resolve(query, r_handler);
        //start = resolver.resolve(query);
        ///////////////////////////////////////////
        co -> yield();
        ////////////////////////////////////////////
        if(e) {
            ORCHID_DEBUG("socket connect error: %s",e.message().c_str());
            return;
        }
        /////////////////////////////////////////
        connect_handler<CO> c_handler(co,e,start);
        boost::asio::async_connect(*this,start, c_handler);
        co -> yield();
        ////////////////////////////////////
        if(e) {
            ORCHID_DEBUG("socket connect error: %s",e.message().c_str());
        }
        return;
    }

    template <typename CO>
    void connect(const std::string& host,const std::string& port,CO co) {
        BOOST_ASSERT(co != NULL);
        boost::system::error_code e;
        connect(host,port,co,e);
        if(e) {
            throw_error(e,"socket connect");
        }
    }

    template <typename MutableBufferSequence,typename CO>
    std::size_t read_some(const MutableBufferSequence& buf,CO co,boost::system::error_code& e) {
        BOOST_ASSERT(co != NULL);
        std::size_t bytes_transferred;
        io_handler<CO> handler(co,e,bytes_transferred);
        async_read_some(buf, handler);
        //////////////////////////////////
        co->yield();
        //////////////////////////////////
        if(e) {
            ORCHID_DEBUG("socket read error: %s",e.message().c_str());
        }
        return bytes_transferred;
    }


    template <typename MutableBufferSequence,typename CO>
    std::size_t read_some(const MutableBufferSequence& buf,CO co) {
        BOOST_ASSERT(co != NULL);
        boost::system::error_code e;
        std::size_t bytes_transferred = read_some(buf,co,e);
        //////////////////////////////////
        if (e) {
            // throw boost::system::system_error(e);
            throw_error(e,"socket read");
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
        if(e) {
            ORCHID_DEBUG("socket write error: %s",e.message().c_str());
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
            throw_error(e,"socket write");
        }
        return bytes_transferred;
    }



};


}
}



#endif