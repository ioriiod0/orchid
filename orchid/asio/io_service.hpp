// ====================================================================================
// Copyright (c) 2012, ioriiod0@gmail.com All rights reserved.
// File         : io_service.hpp
// Author       : ioriiod0@gmail.com
// Last Change  : 11/21/2012 11:57 AM
// Description  : 
// ====================================================================================

#ifndef __ORCHID_IO_SERVICE_H__
#define __ORCHID_IO_SERVICE_H__

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/assert.hpp>
#include <boost/utility.hpp>

namespace orchid { namespace detail {


class io_service:public boost::asio::io_service {
    boost::asio::io_service::work work_;
public:
    io_service():boost::asio::io_service(),work_(*this) {

    }
    ~io_service() {
        
    }

    
};
    // typedef boost::asio::io_service io_service;

}}



#endif