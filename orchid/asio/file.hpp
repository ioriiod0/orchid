// ====================================================================================
// Copyright (c) 2012, ioriiod0@gmail.com All rights reserved.
// File         : file.hpp
// Author       : ioriiod0@gmail.com
// Last Change  : 12/16/2012 10:21 PM
// Description  : 
// ====================================================================================

#ifndef __ORCHID_FILE_H__
#define __ORCHID_FILE_H__

#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>

namespace orchid { namespace detail {



class file_basic
{
public:
    struct impl_type {
        int fd_;
        template <typename ReadHandler>
        void read(const char* dst,std::size_t size,const ReadHandler& h) {

        }

        template <typename WriteHandler>
        void write(const char* src,std::size_t size,const WriteHandler& h) {

        }

        

    };

public:

    file_basic() {

    }
    ~file_basic() {

    }
private:
    

};

}}


#endif