// ====================================================================================
// Copyright (c) 2012, ioriiod0@gmail.com All rights reserved.
// File         : iostreams_helper.hpp
// Author       : ioriiod0@gmail.com
// Last Change  : 11/17/2012 07:18 PM
// Description  : 
// ====================================================================================

#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/categories.hpp>



namespace orchid { namespace detail {

template <typename Socket>
class tcp_device_basic 
{
public:
    //////////////////////////////////////
    typedef char char_type;
    typedef boost::iostreams::bidirectional_device_tag category;
public:
    tcp_device_basic(Socket& sock):sock_(sock) {

    }
    ~tcp_device_basic() {
        
    }
public:
    std::streamsize read(char* data,std::streamsize size) {
        return sock_.read(data,size);
    }

    std::streamsize write(const char* data,std::streamsize size) {
        return sock_.write(data,size);
    }
private:
    Socket& sock_;
};
    


}
}