// ====================================================================================
// Copyright (c) 2012, ioriiod0@gmail.com All rights reserved.
// File         : test_client.cpp
// Author       : ioriiod0@gmail.com
// Last Change  : 11/17/2012 07:51 PM
// Description  : 
// ====================================================================================


#include <string>
#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <signal.h>

#include "../orchid/all.hpp"

using std::string;
using std::cout;
using std::cerr;
using std::endl;

void handle_io(orchid::coroutine_handle co) {
    orchid::socket sock_(co -> get_io_service());
    std::size_t n = 0;

    try {
        sock_.connect("127.0.0.1","5678",co);
        ORCHID_DEBUG("id %lu connect success",co->id());
        orchid::writer<orchid::socket> writer(sock_,co);
        std::size_t n;

        char buf[409600] = {0};
        std::fill(buf,buf+64,'a');
        n = writer.write(buf,64);
        ORCHID_DEBUG("send %d bytes",n);

        std::fill(buf,buf+1024,'b');
        n = writer.write(buf,1024);
        ORCHID_DEBUG("send %d bytes",n);

        std::fill(buf,buf+409600,'c');
        n = writer.write(buf,409600);
        ORCHID_DEBUG("send %d bytes",n);

        std::fill(buf,buf+409600,'d');
        n = writer.write_at_least(buf,409600,4096);
        ORCHID_DEBUG("send %d bytes",n);
        
        std::fill(buf,buf+409600,'e');
        n = writer.write_full(buf,409600);
        ORCHID_DEBUG("send %d bytes",n);

        sock_.close();
 
    } catch (const orchid::io_error& e) {
        ORCHID_ERROR("id %lu msg:%s",co->id(),e.what());
        sock_.close();
    }
}

void handle_sig(orchid::coroutine_handle co) {
    orchid::signal sig(co -> get_io_service());
    try {
        sig.add(SIGINT);
        sig.add(SIGTERM);
        sig.wait(co);
        ORCHID_DEBUG("id %lu signal caught",co->id());
        co->get_scheduler().stop();

    } catch (const orchid::io_error& e) {
        ORCHID_ERROR("id %lu msg:%s",co->id(),e.what());
    }
}


int main() {
    orchid::scheduler sche;
    sche.spawn(handle_sig);
    ORCHID_DEBUG("stack size:%d",orchid::coroutine::maximum_stack_size());
    for (int i=0;i<1;++i) {
        sche.spawn(handle_io,orchid::coroutine::maximum_stack_size());
    }
    sche.run();
}