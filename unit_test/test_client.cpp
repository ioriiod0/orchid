// ====================================================================================
// Copyright (c) 2012, ioriiod0@gmail.com All rights reserved.
// File         : test_client.cpp
// Author       : ioriiod0@gmail.com
// Last Change  : 11/17/2012 07:51 PM
// Description  : 
// ====================================================================================


#include <string>
#include <iostream>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/copy.hpp>
#include <signal.h>

#include "../include/all.hpp"

using std::string;
using std::cout;
using std::cerr;
using std::endl;

const static std::size_t STACK_SIZE = 16*1024;

void handle_io(orchid::coroutine_handle co) {
        orchid::socket sock_(co -> get_scheduler().get_io_service());
        try {
            sock_.connect("127.0.0.1","5678",co);
            orchid::tcp_istream in(sock_,co);
            orchid::tcp_ostream out(sock_,co);
            in.exceptions(std::istream::failbit | std::istream::badbit);
            string str;
            for (;;) {
                out << "hello world !!!!" <<endl;
                std::getline(in, str);
                cout << str << endl;
            }
        } catch (const boost::system::system_error& e) {
            cerr<<e.code()<<" "<<e.what()<<endl;
        }
}

void handle_sig(orchid::coroutine_handle co) {
    orchid::signal sig(co -> get_scheduler().get_io_service());
    try {
        sig.add(SIGINT);
        sig.add(SIGTERM);
        sig.wait(co);
        cout<<"sig caught"<<endl;
        co->get_scheduler().stop();

    } catch (const boost::system::system_error& e) {
        cerr<<e.code()<<" "<<e.what()<<endl;
    }
}


int main() {
    orchid::scheduler sche;
    cout<<orchid::coroutine::default_stack_size()<<endl;
    sche.spawn(handle_sig,orchid::coroutine::minimum_stack_size());
    for (int i=0;i<100;++i) {
        sche.spawn(handle_io);
    }
    sche.run();
}