// ====================================================================================
// Copyright (c) 2012, ioriiod0@gmail.com All rights reserved.
// File         : handlers.hpp
// Author       : ioriiod0@gmail.com
// Last Change  : 11/16/2012 05:58 PM
// Description  : 
// ====================================================================================
#ifndef __ORCHID_HANDLERS_H__
#define __ORCHID_HANDLERS_H__

#include <iostream>
#include <boost/asio.hpp>
#include <boost/utility.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>


namespace orchid { namespace detail {

template <typename CO>
struct io_handler {

    typedef CO coroutine_pointer;

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

template <typename CO>
struct connect_handler {

    typedef CO coroutine_pointer;

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

template <typename CO>
struct sig_handler {

    typedef CO coroutine_pointer;

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

template <typename CO>
struct timer_handler {

    typedef CO coroutine_pointer;

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

template <typename CO>
struct accept_handler {

    typedef CO coroutine_pointer;

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



template <typename CO>
struct worker_handler {
    typedef CO coroutine_pointer;

    worker_handler(coroutine_pointer co,const boost::function<void()>& f)
    :co_(co),f_(f){

    }

    void operator()() {
        f_();
        co_ -> sche_resume();
    }

    boost::function<void()> f_;
    coroutine_pointer co_;
};


}}

#endif


