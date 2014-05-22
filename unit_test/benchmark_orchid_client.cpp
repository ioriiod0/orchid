// ====================================================================================
// Copyright (c) 2012, ioriiod0@gmail.com All rights reserved.
// File         : benchmark_client.cpp
// Author       : ioriiod0@gmail.com
// Last Change  : 11/22/2012 06:35 PM
// Description  : 
// ====================================================================================
#include <string>
#include <iostream>
#include <vector>

#include <boost/iostreams/stream.hpp>
#include <boost/chrono.hpp>
#include <boost/atomic.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/scoped_array.hpp>
#include <boost/shared_ptr.hpp>

#include "../orchid/all.hpp"

using std::string;
using std::cout;
using std::endl;

const static std::size_t STACK_SIZE = 64*1024;

struct counter_t {
    long long total_read;
    long long total_write;
};

typedef boost::shared_ptr<orchid::socket> socket_ptr;

void handle_io(orchid::coroutine_handle co,orchid::chan<counter_t>& ch,
        socket_ptr sock,
        std::size_t buffer_size) {
    counter_t c;
    c.total_read = 0;
    c.total_write = 0;
    int n = buffer_size;
    try {
        boost::scoped_array<char> buff(new char[buffer_size]);
        sock -> connect("127.0.0.1","5678",co);
        sock -> set_option(boost::asio::ip::tcp::no_delay(true));
        printf("connect_sucess!!\r\n");
        for(;;) {
            n = sock -> write(buff.get(),n,co);
            c.total_write += n;
            n = sock -> read(buff.get(),buffer_size,co);
            c.total_read += n;
        }
    } catch (boost::system::system_error& e) {
        printf("errr:%s\r\n",e.what());
    }

    ch.send(c,co);
    printf("io_done!\r\n");

}


void controller(orchid::coroutine_handle co,
                    orchid::chan<counter_t>& ch,
                    std::size_t size,
                    std::size_t ms,
                    std::size_t buffer_size) {
        ////time start/////
    std::vector<socket_ptr> sockets;
    for(std::size_t i=0;i<size;++i) {
        socket_ptr sock(new orchid::socket(co -> get_scheduler().get_io_service()));
        sockets.push_back(sock);
        co -> get_scheduler().spawn(boost::bind(handle_io,_1,boost::ref(ch),sock,buffer_size), STACK_SIZE);
    }
    orchid::timer timer(co -> get_scheduler().get_io_service());
    timer.sleep(ms,co);
    for(std::size_t i=0;i<sockets.size();++i) {
        sockets[i] -> close();
        printf("lala_closing\r\n");
    }
}

void counter(orchid::coroutine_handle co,
            orchid::chan<counter_t>& ch,
            orchid::scheduler_group& group,
            std::size_t size) {
    counter_t c;
    counter_t tmp;
    c.total_read = 0;
    c.total_write = 0;
    //printf("total_%d\n",size);
    while(size-- > 0) {
        ch.recv(tmp,co);
        c.total_write += tmp.total_write;
        c.total_read += tmp.total_read;
    }
    cout<<"total_read:"<<c.total_read<<endl;
    cout<<"total_write:"<<c.total_write<<endl;
    group.stop();

}


int main(int arc,const char* argv[]) {
    int scheduler_count = boost::lexical_cast<int>(argv[1]);
    int session_count_per_scheduler = boost::lexical_cast<int>(argv[2]);
    int buffer_size = boost::lexical_cast<int>(argv[3]);
    int ms = boost::lexical_cast<int>(argv[4]) * 1000;

    cout<<"scheduler_count:"<<scheduler_count<<endl;
    cout<<"session_count_per_scheduler:"<<session_count_per_scheduler<<endl;

    orchid::scheduler_group sg(scheduler_count);
    int total_count = scheduler_count*session_count_per_scheduler;

    orchid::chan<counter_t> ch(total_count);
    sg[0].spawn(boost::bind(counter,_1,boost::ref(ch),boost::ref(sg),total_count), STACK_SIZE);

    for(std::size_t i=0;i<sg.size();++i) {
        sg[i].spawn(boost::bind(controller,_1,boost::ref(ch),session_count_per_scheduler,ms,buffer_size),
                    STACK_SIZE);
    }

    sg.run();
}