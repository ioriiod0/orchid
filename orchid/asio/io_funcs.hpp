// ====================================================================================
// Copyright (c) 2012, ioriiod0@gmail.com All rights reserved.
// File         : io_funcs.hpp
// Author       : ioriiod0@gmail.com
// Last Change  : 11/16/2012 05:58 PM
// Description  : 
// ====================================================================================
#ifndef __ORCHID_IO_FUNCS_H__
#define __ORCHID_IO_FUNCS_H__

#include <iostream>
#include <boost/asio.hpp>
#include <boost/utility.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>
#include <boost/assert.hpp>
#include <boost/regex.hpp>

#include "handlers.hpp"
#include "throw_error.hpp"
#include "../utility/debug.hpp"


namespace orchid { 

    template<typename AsyncReadStream,
            typename MutableBufferSequence,
            typename CO>
    std::size_t read(AsyncReadStream& s,
                    const MutableBufferSequence & buffers,
                    CO co,
                    boost::system::error_code& e) {

        BOOST_ASSERT(co != NULL);
        std::size_t bytes_transferred;
        detail::io_handler<CO> handler(co,e,bytes_transferred);
        boost::asio::async_read(s,buffers,handler);
        //////////////////////////////////
        co->yield();
        //////////////////////////////////
        if(e) {
            ORCHID_DEBUG("io_funcs read error: %s",e.message().c_str());
        }
        return bytes_transferred; 

    }

    template<typename AsyncReadStream,
            typename MutableBufferSequence,
            typename CO>
    std::size_t read(AsyncReadStream& s,
                    const MutableBufferSequence & buffers,
                    CO co) {

        boost::system::error_code e;
        std::size_t bytes_transferred = orchid::read(s,buffers,co,e);
        if (e) {
            detail::throw_error(e,"io_funcs read");
        }

        return bytes_transferred;
    }



    template<typename AsyncReadStream,
            typename MutableBufferSequence,
            typename CompletionCondition,
            typename CO>
    std::size_t read(AsyncReadStream& s,
                    const MutableBufferSequence & buffers,
                    CompletionCondition completion_condition,
                    CO co,
                    boost::system::error_code& e) {

        BOOST_ASSERT(co != NULL);
        std::size_t bytes_transferred;
        detail::io_handler<CO> handler(co,e,bytes_transferred);
        boost::asio::async_read(s,buffers,completion_condition,handler);
        //////////////////////////////////
        co->yield();
        //////////////////////////////////
        if(e) {
            ORCHID_DEBUG("io_funcs read error: %s",e.message().c_str());
        }
        return bytes_transferred; 

    }


    template<typename AsyncReadStream,
            typename MutableBufferSequence,
            typename CompletionCondition,
            typename CO>
    std::size_t read(AsyncReadStream& s,
                    const MutableBufferSequence & buffers,
                    CompletionCondition completion_condition,
                    CO co) {

        BOOST_ASSERT(co != NULL);
        boost::system::error_code e;
        std::size_t bytes_transferred = orchid::read(s,buffers,completion_condition,co,e);
        if (e) {
            detail::throw_error(e,"io_funcs read");
        }

        return bytes_transferred;

    }

    template<typename AsyncReadStream,typename Allocator,typename CO>
    std::size_t read(AsyncReadStream& s,
                    boost::asio::basic_streambuf< Allocator > & b,
                    CO co,
                    boost::system::error_code& e) {

        BOOST_ASSERT(co != NULL);
        std::size_t bytes_transferred;
        detail::io_handler<CO> handler(co,e,bytes_transferred);
        boost::asio::async_read(s,b,handler);
        //////////////////////////////////
        co->yield();
        //////////////////////////////////
        if(e) {
            ORCHID_DEBUG("io_funcs read error: %s",e.message().c_str());
        }
        return bytes_transferred; 

    }

    template<typename AsyncReadStream,typename Allocator,typename CO>
    std::size_t read(AsyncReadStream& s,boost::asio::basic_streambuf< Allocator > & b,CO co) {

        BOOST_ASSERT(co != NULL);
        boost::system::error_code e;
        std::size_t bytes_transferred = orchid::read(s,b,co,e);
        if (e) {
            detail::throw_error(e,"io_funcs read");
        }

        return bytes_transferred;

    }



    template<typename AsyncReadStream,typename Allocator,typename CompletionCondition,typename CO>
    std::size_t read(AsyncReadStream& s,
                    boost::asio::basic_streambuf< Allocator > & b,
                    CompletionCondition completion_condition,
                    CO co,
                    boost::system::error_code& e) {

        BOOST_ASSERT(co != NULL);
        std::size_t bytes_transferred;
        detail::io_handler<CO> handler(co,e,bytes_transferred);
        boost::asio::async_read(s,b,completion_condition,handler);
        //////////////////////////////////
        co->yield();
        //////////////////////////////////
        if(e) {
            ORCHID_DEBUG("io_funcs read error: %s",e.message().c_str());
        }
        return bytes_transferred; 

    }

    template<typename AsyncReadStream,typename Allocator,typename CompletionCondition,typename CO>
    std::size_t read(AsyncReadStream& s,
                    boost::asio::basic_streambuf< Allocator > & b,
                    CompletionCondition completion_condition,
                    CO co) {

        BOOST_ASSERT(co != NULL);
        boost::system::error_code e;
        std::size_t bytes_transferred = orchid::read(s,b,completion_condition,co,e);
        if (e) {
            detail::throw_error(e,"io_funcs read");
        }

        return bytes_transferred;

    }



    template<typename AsyncReadStream,
            typename Allocator,
            typename CO>
    std::size_t read_until(AsyncReadStream & s,
                boost::asio::basic_streambuf< Allocator > & b,
                char delim,
                CO co,
                boost::system::error_code& e) {

        BOOST_ASSERT(co != NULL);
        std::size_t bytes_transferred;
        detail::io_handler<CO> handler(co,e,bytes_transferred);
        boost::asio::async_read_until(s,b,delim,handler);
        //////////////////////////////////
        co->yield();
        //////////////////////////////////
        if(e) {
            ORCHID_DEBUG("io_funcs read_until error: %s",e.message().c_str());
        }
        return bytes_transferred; 

    }

    template<typename AsyncReadStream,
            typename Allocator,
            typename CO>
    std::size_t read_until(AsyncReadStream & s,
                boost::asio::basic_streambuf< Allocator > & b,
                char delim,
                CO co) {

        BOOST_ASSERT(co != NULL);
        boost::system::error_code e;
        std::size_t bytes_transferred = orchid::read_until(s,b,delim,co,e);
        if (e) {
            detail::throw_error(e,"io_funcs read_until");
        }

        return bytes_transferred;

    }


    template<typename AsyncReadStream,
            typename Allocator,
            typename CO>
    std::size_t read_until(AsyncReadStream & s,
                boost::asio::basic_streambuf< Allocator > & b,
                const std::string& delim,
                CO co,
                boost::system::error_code& e) {

        BOOST_ASSERT(co != NULL);
        std::size_t bytes_transferred;
        detail::io_handler<CO> handler(co,e,bytes_transferred);
        boost::asio::async_read_until(s,b,delim,handler);
        //////////////////////////////////
        co->yield();
        //////////////////////////////////
        if(e) {
            ORCHID_DEBUG("io_funcs read_until error: %s",e.message().c_str());
        }
        return bytes_transferred; 

    }

    template<typename AsyncReadStream,
            typename Allocator,
            typename CO>
    std::size_t read_until(AsyncReadStream & s,
                boost::asio::basic_streambuf< Allocator > & b,
                const std::string& delim,
                CO co) {

        BOOST_ASSERT(co != NULL);
        boost::system::error_code e;
        std::size_t bytes_transferred = orchid::read_until(s,b,delim,co,e);
        if (e) {
            detail::throw_error(e,"io_funcs read_until");
        }

        return bytes_transferred;

    }


    template<typename AsyncReadStream,
            typename Allocator,
            typename CO>
    std::size_t read_until(AsyncReadStream & s,
                boost::asio::basic_streambuf< Allocator > & b,
                const boost::regex& delim,
                CO co,
                boost::system::error_code& e) {

        BOOST_ASSERT(co != NULL);
        std::size_t bytes_transferred;
        detail::io_handler<CO> handler(co,e,bytes_transferred);
        boost::asio::async_read_until(s,b,delim,handler);
        //////////////////////////////////
        co->yield();
        //////////////////////////////////
        if(e) {
            ORCHID_DEBUG("io_funcs read_until error: %s",e.message().c_str());
        }
        return bytes_transferred; 

    }

    template<typename AsyncReadStream,
            typename Allocator,
            typename CO>
    std::size_t read_until(AsyncReadStream & s,
                boost::asio::basic_streambuf< Allocator > & b,
                const boost::regex& delim,
                CO co) {

        BOOST_ASSERT(co != NULL);
        boost::system::error_code e;
        std::size_t bytes_transferred = orchid::read_until(s,b,delim,co,e);
        if (e) {
            detail::throw_error(e,"io_funcs read_until");
        }

        return bytes_transferred;

    }

    //////////////////////////////////////////////////////////
    template<typename AsyncWriteStream,
            typename ConstBufferSequence,
            typename CO>
    std::size_t write(AsyncWriteStream & s,
                const ConstBufferSequence & buffers,
                CO co,
                boost::system::error_code& e) {

        BOOST_ASSERT(co != NULL);
        std::size_t bytes_transferred;
        detail::io_handler<CO> handler(co,e,bytes_transferred);
        boost::asio::async_write(s,buffers,handler);
        //////////////////////////////////
        co->yield();
        //////////////////////////////////
        if(e) {
            ORCHID_DEBUG("io_funcs write error: %s",e.message().c_str());
        }
        return bytes_transferred; 

    }


    template<typename AsyncWriteStream,
            typename ConstBufferSequence,
            typename CO>
    std::size_t write(AsyncWriteStream & s,
                const ConstBufferSequence & buffers,
                CO co) {

        BOOST_ASSERT(co != NULL);
        boost::system::error_code e;
        std::size_t bytes_transferred = orchid::write(s,buffers,co,e);
        if (e) {
            detail::throw_error(e,"io_funcs write");
        }

        return bytes_transferred;

    }

    template<typename AsyncWriteStream,
            typename ConstBufferSequence,
            typename CompletionCondition,
            typename CO>
    std::size_t write(AsyncWriteStream & s,
                const ConstBufferSequence & buffers,
                CompletionCondition completion_condition,
                CO co,
                boost::system::error_code& e) {

        BOOST_ASSERT(co != NULL);
        std::size_t bytes_transferred;
        detail::io_handler<CO> handler(co,e,bytes_transferred);
        boost::asio::async_write(s,buffers,completion_condition,handler);
        //////////////////////////////////
        co->yield();
        //////////////////////////////////
        if(e) {
            ORCHID_DEBUG("io_funcs write error: %s",e.message().c_str());
        }
        return bytes_transferred; 

    }


    template<typename AsyncWriteStream,
            typename ConstBufferSequence,
            typename CompletionCondition,
            typename CO>
    std::size_t write(AsyncWriteStream & s,
                const ConstBufferSequence & buffers,
                CompletionCondition completion_condition,
                CO co) {

        BOOST_ASSERT(co != NULL);
        boost::system::error_code e;
        std::size_t bytes_transferred = orchid::write(s,buffers,completion_condition,co,e);
        if (e) {
            detail::throw_error(e,"io_funcs write");
        }

        return bytes_transferred;

    }


    template<typename AsyncWriteStream,
            typename Allocator,
            typename CO>
    std::size_t write(AsyncWriteStream & s,
                boost::asio::basic_streambuf< Allocator > & b,
                CO co,
                boost::system::error_code& e) {

        BOOST_ASSERT(co != NULL);
        std::size_t bytes_transferred;
        detail::io_handler<CO> handler(co,e,bytes_transferred);
        boost::asio::async_write(s,b,handler);
        //////////////////////////////////
        co->yield();
        //////////////////////////////////
        if(e) {
            ORCHID_DEBUG("io_funcs write error: %s",e.message().c_str());
        }
        return bytes_transferred; 

    }


    template<typename AsyncWriteStream,
            typename Allocator,
            typename CO>
    std::size_t write(AsyncWriteStream & s,
                boost::asio::basic_streambuf< Allocator > & b,
                CO co) {

        BOOST_ASSERT(co != NULL);
        boost::system::error_code e;
        std::size_t bytes_transferred = orchid::write(s,b,co,e);
        if (e) {
            detail::throw_error(e,"io_funcs write");
        }

        return bytes_transferred;

    }


    template<typename AsyncWriteStream,
            typename Allocator,
            typename CompletionCondition,
            typename CO>
    std::size_t write(AsyncWriteStream & s,
                boost::asio::basic_streambuf< Allocator > & b,
                CompletionCondition completion_condition,
                CO co,
                boost::system::error_code& e) {

        BOOST_ASSERT(co != NULL);
        std::size_t bytes_transferred;
        detail::io_handler<CO> handler(co,e,bytes_transferred);
        boost::asio::async_write(s,b,completion_condition,handler);
        //////////////////////////////////
        co->yield();
        //////////////////////////////////
        if(e) {
            ORCHID_DEBUG("io_funcs write error: %s",e.message().c_str());
        }
        return bytes_transferred; 

    }


    template<typename AsyncWriteStream,
            typename Allocator,
            typename CompletionCondition,
            typename CO>
    std::size_t write(AsyncWriteStream & s,
                boost::asio::basic_streambuf< Allocator > & b,
                CompletionCondition completion_condition,
                CO co) {

        BOOST_ASSERT(co != NULL);
        boost::system::error_code e;
        std::size_t bytes_transferred = orchid::write(s,b,completion_condition,co,e);
        if (e) {
            detail::throw_error(e,"io_funcs write");
        }

        return bytes_transferred;

    }


}

#endif


