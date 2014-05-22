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
#include <boost/shared_ptr.hpp>

#include "../orchid/all.hpp"


using std::string;
using std::cout;
using std::cerr;
using std::endl;


typedef boost::shared_ptr<orchid::socket> socket_ptr;

void handle_io(orchid::coroutine_handle co,socket_ptr sock) {
    orchid::reader<orchid::socket> reader(*sock,co);
    std::size_t n = 0;
    char buf[409600] = {0};
    try {
        for(;;) {
            n = reader.read(buf,409600);
            ORCHID_DEBUG("id %lu n:%d",co->id(),n);
        }

    } catch (const orchid::io_error& e) {
        ORCHID_ERROR("id %lu msg:%s",co->id(),e.what());
        sock->close();
    }

}

void handle_accept(orchid::coroutine_handle co) {
    try {
        orchid::acceptor acceptor(co -> get_io_service());
        acceptor.bind_and_listen("5678",true);
        for(;;) {
            socket_ptr sock(new orchid::socket(co -> get_io_service()));
            acceptor.accept(*sock,co);
            co -> get_scheduler().spawn(boost::bind(handle_io,_1,sock),orchid::coroutine::maximum_stack_size());
        }
    }
    catch(orchid::io_error& e) {
        ORCHID_ERROR("id %lu msg:%s",co->id(),e.what());
    }
}

int main() {

    orchid::scheduler sche;
    ORCHID_DEBUG("id:%lu",sche.id());
    ORCHID_DEBUG("stack size:%lu",orchid::coroutine::maximum_stack_size());
    sche.spawn(handle_accept,orchid::coroutine::maximum_stack_size());
    sche.run();
}