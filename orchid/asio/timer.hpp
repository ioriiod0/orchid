// ====================================================================================
// Copyright (c) 2012, ioriiod0@gmail.com All rights reserved.
// File         : timer.hpp
// Author       : ioriiod0@gmail.com
// Last Change  : 11/19/2012 04:15 PM
// Description  : 
// ====================================================================================


#include <iostream>
#include <boost/asio.hpp>
#include <boost/assert.hpp>
#include "io_service.hpp"

namespace orchid { namespace detail {


template <typename Coroutine>
class timer_basic {
public:
    typedef io_service io_service_type;
    typedef boost::asio::deadline_timer impl_type;
    typedef io_service io_server_type;
    typedef Coroutine coroutine_type;
    typedef boost::shared_ptr<coroutine_type> coroutine_pointer;
    typedef timer_basic<Coroutine> self_type;

    struct timer_handler {
        timer_handler(coroutine_pointer co,boost::system::error_code& e)
            :co_(co),e_(e) {

        }

        void operator()(const boost::system::error_code& e) {
            e_ = e;
            co_ -> resume();
        }

        coroutine_pointer co_;
        boost::system::error_code& e_;
    };
public:
    timer_basic(io_server_type& io_s)
        :io_service_(io_s),timer_(io_s.get_impl()) {

    }

    ~timer_basic() {

    }
public:

    void sleep(std::size_t ms,coroutine_pointer co) {
        BOOST_ASSERT(co != NULL);
        if (ms == 0) {
            co -> resume();
            co -> yield();
            return;
        }
        boost::system::error_code e;
        timer_handler handler(co,e);
        timer_.expires_from_now(boost::posix_time::milliseconds(ms));
        timer_.async_wait(handler);
        //////////////////////////
        co -> yield();
        /////////////////////////
        if(e) {
            throw boost::system::system_error(e);
        }
    }


    const io_service_type& get_io_service() const {
        return io_service_;
    }

    io_service_type& get_io_service() {
        return io_service_;
    }

private:
    io_service_type& io_service_;
    impl_type timer_;

};






} }