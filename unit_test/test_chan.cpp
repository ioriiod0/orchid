// ====================================================================================
// Copyright (c) 2012, ioriiod0@gmail.com All rights reserved.
// File         : test_chan.cpp
// Author       : ioriiod0@gmail.com
// Last Change  : 11/19/2012 12:35 AM
// Description  : 
// ====================================================================================

#include <string>
#include <stdio.h>
#include <boost/lexical_cast.hpp>
#include "../include/all.hpp"

using std::string;
using std::cout;
using std::endl;



const static std::size_t STACK_SIZE = 64*1024;


void sender(orchid::coroutine_handle& co,int id,orchid::chan<int>& ch) {
    printf("sender%d start!!\r\n",id);
    try {
        int i = 0;
        for (;;) {
            printf("sender%d send: %d\r\n",id,i);
            ch.send(i++,co);
        }
    } catch (...) {
        cout<<"error happened!!\r\n";
    }

}

void receiver(orchid::coroutine_handle& co,int id,orchid::chan<int>& ch) {
    try {
        int i;
        for (;;) {
            ch.recv(i,co);
            printf("receiver%d receive: %d\r\n",id,i);
        }
    } catch (...) {
        cout<<"error happened!!\r\n";

    }
}



int main(int argc,const char* argv[]) {
    int sender_size = boost::lexical_cast<int>(argv[1]);
    int receiver_size = boost::lexical_cast<int>(argv[2]);
    int chan_size = boost::lexical_cast<int>(argv[3]);

    orchid::scheduler_group group(4);
    orchid::chan<int> ch(chan_size);
    int m=0;

    for (int i=0;i<sender_size;++i) {
        group[m++].spawn(boost::bind(sender,_1,i,boost::ref(ch)),STACK_SIZE);
        if(m==group.size()) m=0;
    }
    for (int i=0;i<receiver_size;++i) {
        group[m++].spawn(boost::bind(receiver,_1,i,boost::ref(ch)),STACK_SIZE);
        if(m==group.size()) m=0;
    }

    group.run();
}