// ====================================================================================
// Copyright (c) 2012, ioriiod0@gmail.com All rights reserved.
// File         : io_service.hpp
// Author       : ioriiod0@gmail.com
// Last Change  : 11/21/2012 11:57 AM
// Description  : 
// ====================================================================================

#ifndef __ORCHID_IO_SERVICE_H__
#define __ORCHID_IO_SERVICE_H__

#include "boost/asio.hpp"
#include "boost/bind.hpp"
#include "boost/assert.hpp"

namespace orchid { namespace detail {


class io_service {
public:
    typedef boost::asio::io_service impl_type;

public:
    io_service():impl_(),work_(impl_) {

    }
    ~io_service() {
        
    }
public:
    void run() {
        impl_.run();
    }

    template <typename F>
    void post(const F& f) {
        impl_.post(f);
    }

    impl_type& get_impl() {
        return impl_;
    }

    const impl_type& get_impl() const {
        return impl_;
    }

    void stop() {
        impl_.stop();
    }

private:
    impl_type impl_;
    boost::asio::io_service::work work_; //keep running 
    
};


}}



#endif