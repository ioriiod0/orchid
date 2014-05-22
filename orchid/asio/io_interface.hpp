// ====================================================================================
// Copyright (c) 2012, ioriiod0@gmail.com All rights reserved.
// File         : io_interface.hpp
// Author       : ioriiod0@gmail.com
// Last Change  : 11/17/2012 07:18 PM
// Description  : 
// ====================================================================================

#ifndef __IO_INTERFACE_HPP__
#define __IO_INTERFACE_HPP__


#include <iostream>
#include <memory>
#include <string>
#include <algorithm>
#include <boost/asio.hpp>
// #include <boost/iostreams/stream.hpp>
// #include <boost/iostreams/categories.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>
#include <boost/utility.hpp>
#include <boost/range/algorithm.hpp>

#include "throw_error.hpp"
#include "io_funcs.hpp"
#include "../utility/debug.hpp"



namespace orchid { namespace detail {


template <typename CO,typename Input>
class reader_basic
{
public:
    //////////////////////////////////////
    // typedef Coroutine coroutine_type;
    // typedef boost::shared_ptr<coroutine_type> coroutine_pointer;
    typedef CO coroutine_pointer;
    typedef char char_type;
    // typedef boost::iostreams::source_tag category;
public:
    reader_basic(Input& input,coroutine_pointer co):input_(input),co_(co) {

    }
    ~reader_basic() {
        
    }
public:
    std::size_t read(char* data,std::size_t size,boost::system::error_code& e) {
        return input_.read_some(boost::asio::buffer(data,size),co_,e);
    }

    std::size_t read(char* data,std::size_t size) {
        return input_.read_some(boost::asio::buffer(data,size),co_);
    }

    std::size_t read_full(char* data,std::size_t size,boost::system::error_code& e) {
        return orchid::read(input_,boost::asio::buffer(data,size),co_,e);
    }

    std::size_t read_full(char* data,std::size_t size) {
        return orchid::read(input_,boost::asio::buffer(data,size),co_);
    }

    std::size_t read_at_least(char* data,std::size_t size,std::size_t n,boost::system::error_code& e) {
        return orchid::read(input_,boost::asio::buffer(data,size),boost::asio::transfer_at_least(n),co_,e);
    }

    std::size_t read_at_least(char* data,std::size_t size,std::size_t n) {
        return orchid::read(input_,boost::asio::buffer(data,size),boost::asio::transfer_at_least(n),co_);
    }

private:
    Input& input_;
    coroutine_pointer co_;
};



template <typename CO,typename Output>
class writer_basic
{
public:
    //////////////////////////////////////
    typedef CO coroutine_pointer;
    typedef char char_type;
    // typedef boost::iostreams::sink_tag category;
public:
    writer_basic(Output& output,coroutine_pointer co):output_(output),co_(co) {

    }
    ~writer_basic() {
        
    }
public:
    std::size_t write(const char* data,std::size_t size,boost::system::error_code& e) {
        return output_.write_some(boost::asio::buffer(data,size),co_,e);
    }

    std::size_t write(const char* data,std::size_t size) {
        return output_.write_some(boost::asio::buffer(data,size),co_);
    }

    std::size_t write_full(const char* data,std::size_t size,boost::system::error_code& e) {
        return orchid::write(output_,boost::asio::buffer(data,size),co_,e);
    }

    std::size_t write_full(const char* data,std::size_t size) {
        return orchid::write(output_,boost::asio::buffer(data,size),co_);
    }

    std::size_t write_at_least(const char* data,std::size_t size,std::size_t n,boost::system::error_code& e) {
        return orchid::write(output_,boost::asio::buffer(data,size),boost::asio::transfer_at_least(n),co_,e);
    }

    std::size_t write_at_least(const char* data,std::size_t size,std::size_t n) {
        return orchid::write(output_,boost::asio::buffer(data,size),boost::asio::transfer_at_least(n),co_);
    }

private:
    Output& output_;
    coroutine_pointer co_;
};



template <typename CO,typename Input,typename Alloc=std::allocator<char> >
class buffered_reader_basic
{
public:
    //////////////////////////////////////
    // typedef Coroutine coroutine_type;
    // typedef boost::shared_ptr<coroutine_type> coroutine_pointer;
    typedef CO coroutine_pointer;
    typedef char char_type;
    enum {
        min_size = 16,
        default_size = 4096,
    };
    // typedef std::pair<char*,char*> range_type;
    // typedef boost::iostreams::source_tag category;
public:
    buffered_reader_basic(Input& input,coroutine_pointer co,std::size_t size = default_size)
        :input_(input),co_(co),w_(0),r_(0) {
            if(size < min_size) {
                size = min_size;
            }
            r_buff_.resize(size);
    }

    ~buffered_reader_basic() {
        
    }
public:
    std::size_t buffered() const {
        return w_ - r_;
    }

    std::size_t size() const {
        return r_buff_.size();
    }
public:

    std::size_t read(char* data,std::size_t size,boost::system::error_code& e) {
        if (size <= 0 ) {
            return 0;
        }

        if ( buffered() == 0 ) {

            if (size >= this->size()) {
                return input_.read_some(boost::asio::buffer(data,size),co_,e);
            }

            fill(e);

            if (buffered() == 0) {
                return 0;
            }

        }

        if (size > buffered()) {
            size = buffered();
        } 

        std::copy(data,r_buff_.begin(),r_buff_.begin()+size);
        r_ += size;
        return size;

    }

    std::size_t read(char* data,std::size_t size) {
        boost::system::error_code e;
        std::size_t bytes_transferred = read(data,size,e);
        if(e) {
            throw_error(e,"buffered io read");
        }
        return bytes_transferred;
    }



    template <typename Traits,typename Allocator>
    std::size_t read_until(std::basic_string<char,Traits,Allocator>& buf,char delim,boost::system::error_code& e) {

        buf.clear();
        int index = -1;
        std::size_t size = 0;
        int n = 0;

        index = find(r_buff_.begin()+r_,r_buff_.begin()+w_,delim);

        if (index != -1) {
            n = index+1;
            buf.reserve(n+1);
            std::copy(r_buff_.begin() + r_,r_buff_.begin()+ r_ + n,std::back_inserter(buf));
            r_ += n;
            size += n;
            return size;
        }
        // buf.reserve(this->size());
        
        for(;;) {

            n = buffered();
            if (n > 0) {
                std::copy(r_buff_.begin() + r_,r_buff_.begin() + w_,std::back_inserter(buf));
                r_ += n;
                size += n;
            }
            

            fill(e);

            if(e) {
                n = buffered();
                std::copy(r_buff_.begin() + r_,r_buff_.begin() + w_,std::back_inserter(buf));
                r_ += n;
                size += n;
                return size;
            }

            index = find(r_buff_.begin()+r_,r_buff_.begin()+w_,delim);

            if(index >= 0) {
                n = index+1;
                std::copy(r_buff_.begin() + r_,r_buff_.begin() + r_ + n,std::back_inserter(buf));
                r_ += n;
                size += n;
                return size;
            }

        }

    }

    template <typename Traits,typename Allocator>
    std::size_t read_until(std::basic_string<char,Traits,Allocator>& buf,char delim) {
        boost::system::error_code e;
        std::size_t bytes_transferred = read_until(buf,delim,e);
        if(e) {
            throw_error(e,"buffered io read_until");
        }
        return bytes_transferred;
    }


    template <typename Traits,typename Allocator>
    std::size_t read_until(std::basic_string<char,Traits,Allocator>& buf,const std::basic_string<char,Traits,Allocator>& delim,boost::system::error_code& e) {

        buf.clear();
        int index = -1;
        std::size_t size = 0;
        int n = 0;

        index = find(r_buff_.begin()+r_,r_buff_.begin()+w_,delim);

        if (index != -1) {
            n = index+delim.size();
            buf.reserve(n+1);
            std::copy(r_buff_.begin() + r_,r_buff_.begin()+ r_ + n,std::back_inserter(buf));
            r_ += n;
            size += n;
            return size;
        }

        // buf.reserve(this->size());
        
        for(;;) {

            n = buffered()-delim.size() + 1;
            if (n > 0) {
                std::copy(r_buff_.begin() + r_,r_buff_.begin() + r_ + n,std::back_inserter(buf));
                r_ += n;
                size += n;
            }
            
            fill(e);

            if(e) {
                n = buffered();
                std::copy(r_buff_.begin() + r_,r_buff_.begin() + r_ + n,std::back_inserter(buf));
                r_ += n;
                size += n;
                return size;
            }

            index = find(r_buff_.begin()+r_,r_buff_.begin()+w_,delim);

            if(index >= 0) {
                n = index + delim.size();
                std::copy(r_buff_.begin() + r_,r_buff_.begin() + r_ + n,std::back_inserter(buf));
                r_ += n;
                size += n;
                return size;
            }

        }

    }

    template <typename Traits,typename Allocator>
    std::size_t read_until(std::basic_string<char,Traits,Allocator>& buf,const std::basic_string<char,Traits,Allocator>& delim) {
        boost::system::error_code e;
        std::size_t bytes_transferred = read_until(buf,delim,e);
        if(e) {
            throw_error(e,"buffered io read_until");
        }
        return bytes_transferred;
    }


    template <typename Traits,typename Allocator>
    std::size_t read_until(std::basic_string<char,Traits,Allocator>& buf,const char* delim,boost::system::error_code& e) {
        return read_until(buf,std::string(delim),e);
    }


    template <typename Traits,typename Allocator>
    std::size_t read_until(std::basic_string<char,Traits,Allocator>& buf,const char* delim) {
        return read_until(buf,std::string(delim));
    }


    std::size_t read_full(char* data,std::size_t size,boost::system::error_code& e) {
        std::size_t n = 0;
        std::size_t size_to_read = size;
        std::size_t size_read = 0;
        while (size_to_read) {
            n = read(data,size_to_read,e);
            size_to_read -= n;
            size_read += n;
            data += n;
            if(e) {
                break;
            }
        }
        return size_read;
    }

    std::size_t read_full(char* data,std::size_t size) {
        boost::system::error_code e;
        std::size_t bytes_transferred = read_full(data,size,e);
        if(e) {
            throw_error(e,"buffered io read_full");
        }
        return bytes_transferred;
    }



private:
    void fill(boost::system::error_code& e) {
        if (r_ > 0) {
            std::memmove(&r_buff_[0],&r_buff_[r_],buffered());
            w_ -= r_;
            r_ = 0;
        }

        std::size_t n = input_.read_some(boost::asio::buffer(&r_buff_[w_], size() - buffered()), co_, e);
        w_ += n;

    }

    template <typename IT>
    int find(IT first, IT last,char delim) {
        for(std::size_t i = 0;first != last;++first,++i) {
            if(*first == delim) {
                return i;
            }
        }
        return -1;
    }

    template <typename IT,typename Traits,typename Allocator>
    int find(IT first, IT last,const std::basic_string<char,Traits,Allocator>& delim) {
        std::pair<IT,IT> range1(first,last);
        IT found = boost::search(range1,delim);
        if (found == last) {
            return -1;
        } else {
            return std::distance(first,found);
        }

    }

private:
    Input& input_;
    coroutine_pointer co_;
    std::vector<char,Alloc> r_buff_;
    std::size_t r_;
    std::size_t w_;
};




template <typename CO,typename Output,typename Alloc = std::allocator<char> >
class buffered_writer_basic
{
public:
    //////////////////////////////////////
    // typedef Coroutine coroutine_type;
    // typedef boost::shared_ptr<coroutine_type> coroutine_pointer;
    typedef CO coroutine_pointer;
    typedef char char_type;
    enum {
        min_size = 16,
        default_size = 4196,
    };
    // typedef boost::iostreams::sink_tag category;
public:
    buffered_writer_basic(Output& output,coroutine_pointer co,std::size_t size=default_size)
        :output_(output),co_(co),n_(0) {
        if(size < min_size) {
            size = min_size;
        }
        w_buff_.resize(size);

    }
    ~buffered_writer_basic() {

    }
public:
    std::size_t buffered() const {
        return n_;
    }

    std::size_t size() const {
        return w_buff_.size();
    }

    std::size_t available() const {
        return size() - buffered();
    }

public:

    std::size_t write(const char* data,std::size_t size,boost::system::error_code& e) {
        std::size_t total = 0;
        while (size > available()) {
            std::size_t n = 0;
            if (buffered() == 0) {
                n = output_.write_some(boost::asio::buffer(data,size),co_,e);
            } else {
                n = available();
                std::copy(data,data+n,w_buff_.begin()+buffered());
                n_ += n;
                do_flush(e);
            }

            total += n;
            size -= n;
            data += n;

            if (e) {
                break;
            }
        }

        if (e) {
            return total;
        }

        std::copy(data,data+size,w_buff_.begin()+buffered());
        total += size;
        n_ += size;
        return total;
    }

    std::size_t write(const char* data,std::size_t size) {
        boost::system::error_code e;
        std::size_t n = write(data,size,e);
        if (e) {
            throw_error(e,"buffered io write");
        }
        return n;
    }

    void do_flush(boost::system::error_code& e) {
        if (buffered() == 0) {
            return;
        }

        std::size_t n = 0;
        n = output_.write_some(boost::asio::buffer(&w_buff_[0],buffered()),co_,e);

        if (n > 0 && n < buffered()) {
            std::memmove(&w_buff_[0],&w_buff_[n],buffered()-n);
        }

        n_ -= n;
    }

    void flush(boost::system::error_code& e) {
        while(buffered() > 0) {
            do_flush(e);
            if (e) {
                break;
            }
        }
    }

    void flush() {
        boost::system::error_code e;
        flush(e);
        if(e) {
            throw_error(e,"buffered io flush");
        }
    }

    // std::size_t write_full(const char* data,std::size_t size,boost::system::error_code& e) {
    //     std::size_t n = 0;
    //     std::size_t size_to_write = size;
    //     std::size_t size_wrote = 0;
    //     while (size_to_write) {
    //         n = write(data,size_to_write,e);
    //         size_to_write -= n;
    //         size_wrote += n;
    //         data += n;
    //         if(e) {
    //             break;
    //         }
    //     }
    //     return size_wrote;
    // }

    // std::size_t write_full(const char* data,std::size_t size) {
    //     boost::system::error_code e;
    //     write_full(data,size,e);
    //     if(e) {
    //         throw_error(e,"buffered io write_full");
    //     }
    // }

private:
    Output& output_;
    coroutine_pointer co_;
    // boost::asio::streambuf<Alloc> w_buff_;
    std::vector<char> w_buff_;
    std::size_t n_;

};


// template <typename T>
// class stream:public boost::iostreams::stream<T> {
// public:
//     template <typename IO,typename Coroutine>
//     stream(const IO& io,const Coroutine& co):boost::iostreams::stream<T>(io,co) {
//         this->exceptions(std::ios_base::badbit);
//     }

//     template <typename IO,typename Coroutine>
//     stream(IO& io,const Coroutine& co):boost::iostreams::stream<T>(io,co) {
//         this->exceptions(std::ios_base::badbit);
//     }

// };


}
}

#endif