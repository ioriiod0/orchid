// ====================================================================================
// Copyright (c) 2012, ioriiod0@gmail.com All rights reserved.
// File         : test_server.cpp
// Author       : ioriiod0@gmail.com
// Last Change  : 11/17/2012 06:11 PM
// Description  : 
// ====================================================================================


#include <iterator>
#include <algorithm>
#include <string>
#include <iostream>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/shared_ptr.hpp>

#include "../include/all.hpp"

using std::string;
using std::cout;
using std::cerr;
using std::endl;


const static std::size_t STACK_SIZE = 16*1024;

typedef boost::shared_ptr<orchid::socket> socket_ptr;

void handle_io(orchid::coroutine_handle co,socket_ptr sock) {
    string str;
    orchid::tcp_ostream out(*sock,co);
    orchid::tcp_istream in(*sock,co);
    for(std::string str;std::getline(in, str) && out;)
    {
        out<<str<<endl;
    }
  
}

void handle_accept(orchid::coroutine_handle co) {
    try {
        orchid::acceptor acceptor(co -> get_scheduler().get_io_service());
        acceptor.bind_and_listen("5678",true);
        for(;;) {
            socket_ptr sock(new orchid::socket(co -> get_scheduler().get_io_service()));
            acceptor.accept(*sock,co);
            co -> get_scheduler().spawn(boost::bind(handle_io,_1,sock),STACK_SIZE);
        }
    }
    catch(boost::system::system_error& e) {
        cerr<<e.code()<<" "<<e.what()<<endl;
    }
}

int main() {
    orchid::scheduler sche;
    sche.spawn(boost::bind(handle_accept,_1),STACK_SIZE);
    sche.run();
}