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

#include "../include/all.hpp"


using std::string;
using std::cout;
using std::endl;



const static std::size_t STACK_SIZE = 64*1024;



void sender(orchid::coroutine_handle co,int id,orchid::chan<int>& ch) {
    for (;;) {
        ch.send(id,co);
    }
}

void receiver(orchid::coroutine_handle co,orchid::chan<int>& ch) {
    orchid::descriptor stdout(co -> get_scheduler().get_io_service(),STDOUT_FILENO);
    orchid::descriptor_ostream console(stdout,co);
    int id;
    for (;;) {
        ch.recv(id,co);
        console<<"receiver receive: "<<id<<std::endl;
    }
}

void test_one_scheduler() {
    orchid::scheduler sche;
    orchid::chan<int> ch(10);
    for (int i=0;i<100;++i) {
        sche.spawn(boost::bind(sender,_1,i,boost::ref(ch)));
    }
    sche.spawn(boost::bind(receiver,_1,boost::ref(ch)));
    sche.run();
}

void test_scheduler_group() {
    orchid::scheduler_group group(2);
    orchid::chan<int> ch(10);
    for (int i=0;i<100;++i) {
        group[i%2].spawn(boost::bind(sender,_1,i,boost::ref(ch)));
    }
    group[0].spawn(boost::bind(receiver,_1,boost::ref(ch)));
    group.run();
}

int main(int argc,const char* argv[]) {
    test_one_scheduler();
    test_scheduler_group();

}

 


