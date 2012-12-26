// ====================================================================================
// Copyright (c) 2012, ioriiod0@gmail.com All rights reserved.
// File         : iostreams_helper.hpp
// Author       : ioriiod0@gmail.com
// Last Change  : 11/17/2012 07:18 PM
// Description  : 
// ====================================================================================

#include <boost/shared_ptr.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/categories.hpp>



namespace orchid { namespace detail {

// template <typename Coroutine,typename IO>
// class io_device_basic
// {
// public:
//     //////////////////////////////////////
//     typedef Coroutine coroutine_type;
//     typedef coroutine_type* coroutine_pointer;
//     typedef char char_type;
//     typedef boost::iostreams::bidirectional_device_tag category;
// public:
//     io_device_basic(IO& io,coroutine_pointer co):io_(io),co_(co) {

//     }
//     ~io_device_basic() {
        
//     }
// public:
//     std::streamsize read(char* data,std::streamsize size) {
//         return io_.read(data,size,co_);
//     }

//     std::streamsize write(const char* data,std::streamsize size) {
//         return io_.write(data,size,co_);
//     }
// private:
//     IO& io_;
//     coroutine_pointer* co_;
// };
    


template <typename Coroutine,typename Input>
class reader_basic
{
public:
    //////////////////////////////////////
    typedef Coroutine coroutine_type;
    typedef boost::shared_ptr<coroutine_type> coroutine_pointer;
    typedef char char_type;
    typedef boost::iostreams::source_tag category;
public:
    reader_basic(Input& input,coroutine_pointer co):input_(input),co_(co) {

    }
    ~reader_basic() {
        
    }
public:
    std::streamsize read(char* data,std::streamsize size) {
        return input_.read(data,size,co_);
    }

private:
    Input& input_;
    coroutine_pointer co_;
};



template <typename Coroutine,typename Output>
class writer_basic
{
public:
    //////////////////////////////////////
    typedef Coroutine coroutine_type;
    typedef boost::shared_ptr<coroutine_type> coroutine_pointer;
    typedef char char_type;
    typedef boost::iostreams::sink_tag category;
public:
    writer_basic(Output& output,coroutine_pointer co):output_(output),co_(co) {

    }
    ~writer_basic() {
        
    }
public:
    std::streamsize write(const char* data,std::streamsize size) {
        return output_.write(data,size,co_);
    }

private:
    Output& output_;
    coroutine_pointer co_;
};




}
}