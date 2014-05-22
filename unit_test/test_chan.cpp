// ====================================================================================
// Copyright (c) 2012, ioriiod0@gmail.com All rights reserved.
// File         : test_chan.cpp
// Author       : ioriiod0@gmail.com
// Last Change  : 11/19/2012 12:35 AM
// Description  : 
// ====================================================================================

#include <unistd.h>
#include <string>
#include <stdio.h>
#include <iostream>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include "../orchid/all.hpp"


using std::string;
using std::cout;
using std::endl;



const static std::size_t STACK_SIZE = 64*1024;



void sender(orchid::coroutine_handle co,int id,orchid::chan<int>& ch) {
    orchid::descriptor stdout(co -> get_io_service(),::dup(STDOUT_FILENO));
    orchid::writer<orchid::descriptor> console(stdout,co);
    char buf[128] = {0};

    for (;;) {
        ch.send(id,co);
        int n = sprintf(buf,"sender %d send:%d\r\n",id,id);
        console.write(buf,n);
    }
}

void receiver(orchid::coroutine_handle co,int id,orchid::chan<int>& ch) {
    orchid::descriptor stdout(co -> get_io_service(),::dup(STDOUT_FILENO));
    orchid::writer<orchid::descriptor> console(stdout,co);
    int sender = 0;
    char buf[128] = {0};

    for (;;) {
        ch.recv(sender,co);
        int n = sprintf(buf,"receiver %d receive:%d\r\n",id,sender);
        console.write(buf,n);
    }

}

void test_one_scheduler() {
    orchid::scheduler sche;
    orchid::chan<int> ch(100);
    for (int i=0;i<100;++i) {
        sche.spawn(boost::bind(sender,_1,i,boost::ref(ch)));
    }
    for (int i=0;i<100;++i) {
        sche.spawn(boost::bind(receiver,_1,i,boost::ref(ch)));
    }
    
    sche.run();
}

void test_scheduler_group() {
    int N = 2;
    orchid::scheduler_group group(N);
    orchid::chan<int> ch(10);
    for (int i=0;i<100;++i) {
        group[i%N].spawn(boost::bind(sender,_1,i,boost::ref(ch)));
    }
    for (int i=0;i<100;++i) {
        group[i%N].spawn(boost::bind(receiver,_1,i,boost::ref(ch)));
    }
    group.run();
    // boost::thread t(boost::bind(&orchid::scheduler_group::run,&group));
    // t.join()
}

int main(int argc,const char* argv[]) {
    // test_one_scheduler();
    test_scheduler_group();

}

 


