// ====================================================================================
// Copyright (c) 2012, ioriiod0@gmail.com All rights reserved.
// File         : test_worker.cpp
// Author       : ioriiod0@gmail.com
// Last Change  : 11/19/2012 12:35 AM
// Description  : 
// ====================================================================================

#include <unistd.h>
#include <string>
#include <stdio.h>
#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include "../orchid/all.hpp"


void test(orchid::coroutine_handle co) {
    int a = 1,b = 2;
    int c;
    orchid::run_in_thread([a,b,&c](){
        boost::this_thread::sleep(boost::posix_time::seconds(3));
        c = a+b;
    },co);

    ORCHID_DEBUG("c:%d",c);
}

int main(int argc,const char* argv[]) {
    orchid::scheduler sche;
    orchid::worker_pool pool(1);
    boost::thread t([&pool](){ pool.run(); });

    sche.spawn(test);
    sche.spawn([&pool](orchid::coroutine_handle co){
        int a = 1,b = 2;
        int c;
        pool.post([a,b,&c](){
            boost::this_thread::sleep(boost::posix_time::seconds(3));
            c = a+b;
        },co);
        ORCHID_DEBUG("c:%d",c);
    });

    sche.run();
}

 


