// ====================================================================================
// Copyright (c) 2012, ioriiod0@gmail.com All rights reserved.
// File         : signal.hpp
// Author       : ioriiod0@gmail.com
// Last Change  : 12/13/2012 11:47 AM
// Description  : 
// ====================================================================================


#ifndef __ORCHID_SIGNAL_H__
#define __ORCHID_SIGNAL_H__

#include <boost/asio.hpp>
#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>
#include "io_service.hpp"


namespace orchid { namespace detail{

template <typename Coroutine>
class signal_basic {
public:
    typedef boost::asio::signal_set impl_type;
    typedef io_service io_service_type;
    typedef signal_basic<Coroutine> self_type;
    typedef Coroutine coroutine_type;
    typedef boost::shared_ptr<coroutine_type> coroutine_pointer;

    struct sig_handler {
        sig_handler(coroutine_pointer co,boost::system::error_code& e,int& sig_num):
            e_(e),co_(co),sig_num_(sig_num) {
        }

        void operator()(const boost::system::error_code& e,int sig_num){
            e_ = e;
            sig_num_ = sig_num;
            co_ -> resume();
        }

        boost::system::error_code& e_;
        coroutine_pointer co_;
        int& sig_num_;
    };

public:
    signal_basic(io_service_type& io_s)
        :io_service_(io_s),sig_(io_s.get_impl()) {

    }
    ~signal_basic() {

    }
public:
    void add(int signal_num) {
        sig_.add(signal_num);
    }

    void cancel() {
        sig_.cancel();
    }

    void clear() {
        sig_.clear();
    }

    void remove(int signal_num) {
        sig_.remove(signal_num);
    }

    int wait(coroutine_pointer co) {
        boost::system::error_code e;
        int sig_num = 0;
        sig_handler handler(co,e,sig_num);
        sig_.async_wait(handler);
        /////////////////////////
        co -> yield();
        ///////////////////////////
        if(e) {
            throw boost::system::system_error(e);
        }
        return sig_num;
    }
private:
    io_service_type& io_service_;
    boost::asio::signal_set sig_;

};
    
}}



#endif