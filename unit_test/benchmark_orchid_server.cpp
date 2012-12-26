// ====================================================================================
// Copyright (c) 2012, ioriiod0@gmail.com All rights reserved.
// File         : orchid_benchmark.cpp
// Author       : ioriiod0@gmail.com
// Last Change  : 11/22/2012 06:28 PM
// Description  : 
// ====================================================================================

#include <string>
#include <iostream>
#include <boost/iostreams/stream.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/scoped_array.hpp>
#include <boost/shared_ptr.hpp>

#include "../orchid/all.hpp"

using std::string;
using std::cout;
using std::endl;

const static std::size_t STACK_SIZE = 64*1024;

typedef boost::shared_ptr<orchid::socket> socket_ptr;

void handle_io(orchid::coroutine_handle co,socket_ptr sock,std::size_t buffer_size) {
    try {
        boost::scoped_array<char> buffer(new char[buffer_size]);
        sock -> get_impl().set_option(boost::asio::ip::tcp::no_delay(true));
        printf("accept_sucess!!\r\n");
        int n = 0;
        for (;;) {
            n = sock -> read(buffer.get(),buffer_size,co);
            n = sock -> write(buffer.get(),n,co);
        }

    } catch (boost::system::system_error& e) {
        cout<<e.code()<<" "<<e.what()<<endl;
    }
}

void handle_accept(orchid::coroutine_handle co,
    orchid::scheduler_group& group,
    const std::string& port,
    std::size_t buffer_size) {
    try {
        orchid::acceptor acceptor(co -> get_scheduler().get_io_service());
        acceptor.bind_and_listen(port,true);
        int i = 0;
        for(;;) {
            socket_ptr sock(new orchid::socket(group[i].get_io_service()));
            acceptor.accept(*sock,co);
            group[i].spawn(boost::bind(handle_io,_1,sock,buffer_size),STACK_SIZE);
            if(++i == group.size()) i=0;
        }
    }
    catch(boost::system::system_error& e) {
        cout<<e.code()<<" "<<e.what()<<endl;
        cout<<"acceptor's done!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<endl;
    }
}

int main(int argc,const char* argv[]) {
    std::string port = "5678";
    int thread_count = boost::lexical_cast<int>(argv[1]);
    int buf_size = boost::lexical_cast<int>(argv[2]);
    cout<<"thread_count:"<<thread_count<<endl;
    orchid::scheduler_group group(thread_count);
    
    group[0].spawn(boost::bind(handle_accept,_1,boost::ref(group),boost::cref(port),buf_size),
                    STACK_SIZE);
    group.run();
}