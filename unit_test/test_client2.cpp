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
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/copy.hpp>
#include <signal.h>

#include "../orchid/all.hpp"

using std::string;
using std::cout;
using std::cerr;
using std::endl;

void handle_io(orchid::coroutine_handle co) {
    orchid::socket sock_(co -> get_io_service());
    std::size_t n = 0;
    char buf[409600] = {0};

    try {
        sock_.connect("127.0.0.1","5678",co);
        ORCHID_DEBUG("id %lu connect success",co->id());
        orchid::writer<orchid::socket> writer(sock_,co);

        std::fill(buf,buf+409600,'a');
        for(int i = 0 ;i < 3;++ i) {
            n = writer.write_full(buf,409600);
            ORCHID_DEBUG("send %d bytes",n);
        }
        
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
    for (int i=0;i<1;++i) {
        sche.spawn(handle_io,orchid::coroutine::maximum_stack_size());
    }
    sche.run();
}