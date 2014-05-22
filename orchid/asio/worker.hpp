// ====================================================================================
// Copyright (c) 2012, ioriiod0@gmail.com All rights reserved.
// File         : worker.hpp
// Author       : ioriiod0@gmail.com
// Last Change  : 11/19/2012 04:15 PM
// Description  : 
// ====================================================================================


#include <boost/asio.hpp>
#include <boost/assert.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

#include "io_service.hpp"
#include "throw_error.hpp"
#include "handlers.hpp"
#include "../utility/debug.hpp"

namespace orchid {

    template <typename CO>
    void run_in_thread(const boost::function<void()>& f,CO co) {
        detail::worker_handler<CO> handler(co,f);
        boost::thread t(handler);
        co -> yield();
    }

namespace detail {

    class worker_pool_basic {
    public:
        typedef io_service io_service_type;

    public:
        worker_pool_basic(std::size_t size)
            :size_(size),io_service_() {

        }
        ~worker_pool_basic() {

        }
    public:
        void run() {
            for(std::size_t i = 0;i < size_;++i) {
                tg_.create_thread(boost::bind(&io_service_type::run,&io_service_));
            }
            tg_.join_all();
        }

        void stop() {
            io_service_.stop();
        }

        template <typename CO>
        void post(const boost::function<void()>& f,CO co) {
            detail::worker_handler<CO> handler(co,f);
            io_service_.post(handler);
            co -> yield();
        } 

        std::size_t size() {
            return size_;
        }

    private:

        std::size_t size_;
        boost::thread_group tg_;
        io_service io_service_;

    };


}


}