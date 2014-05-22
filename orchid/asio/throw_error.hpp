
#ifndef __ORCHID_THROW_EXCEPTION_H__
#define __ORCHID_THROW_EXCEPTION_H__

#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>
#include <iostream>
#include <boost/asio.hpp>

namespace orchid { namespace detail {

    class io_error: public boost::system::system_error {
        
    public:
        explicit io_error(const boost::system::error_code& err)
            :boost::system::system_error(err) {

        }

        io_error(const boost::system::error_code& err,const char* location)
            :boost::system::system_error(err,location) {

        }

    };

    void throw_error(const boost::system::error_code& err) {
      io_error e(err);
      throw e;
    }

    void throw_error(const boost::system::error_code& err, const char* location) {
      io_error e(err, location);
      throw e;
    }
}}


#endif