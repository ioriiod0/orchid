// ====================================================================================
// Copyright (c) 2012, ioriiod0@gmail.com All rights reserved.
// File         : test_chan.cpp
// Author       : ioriiod0@gmail.com
// Last Change  : 11/19/2012 12:35 AM
// Description  : 
// ====================================================================================

#include <string>
#include <stdio.h>

#include "../include/all.hpp"

using std::string;
using std::cout;
using std::endl;

class sender_coroutine : public orchid::coroutine {
public:
    sender_coroutine(orchid::scheduler& sche,int id,orchid::chan<int,128>* chan):
        orchid::coroutine(sche),id_(id),chan_(chan),timer_(sche.get_io_service(),this) {

    }

    virtual ~sender_coroutine() {
        cout<<"i am done!!"<<endl;
    }

    virtual void run() {
        try {
            int i = 0;
            for (;;) {
                //timer_.sleep(1000);
                chan_ -> enqueue(i++,this);
                printf("sender %d send: %d\r\n",id_,i);
            }
        } catch (...) {
            cout<<"error happened!!\r\n";
        }

    }

    int id_;
    orchid::chan<int,128>* chan_;
    orchid::timer timer_;

};


class receiver_coroutine : public orchid::coroutine {
public:
    receiver_coroutine(orchid::scheduler& sche,int id,orchid::chan<int,128>* chan):
        orchid::coroutine(sche),id_(id),chan_(chan) {

    }

    virtual ~receiver_coroutine() {
        cout<<"i am done!!"<<endl;
    }

    virtual void run() {
        try {
            int i;
            for (;;) {
                //cout<<"receiver "<<id_<<" reveive: "<<i<<endl;
                chan_ -> dequeue(i,this);
                printf("receiver %d receive: %d\r\n",id_,i);
            }
        } catch (...) {
            cout<<"error happened!!\r\n";

        }

    }

    int id_;
    orchid::chan<int,128>* chan_;

};


int main() {
    orchid::scheduler_group group(4);
    orchid::chan<int,128> ch;

    for (int i=0;i<1000;++i) {
        sender_coroutine* co = new sender_coroutine(group.get_scheduler(),i,&ch);
        group.spawn(co);
    }

    for (int i=0;i<100;++i) {
        receiver_coroutine* co = new receiver_coroutine(group.get_scheduler(),i,&ch);
        group.spawn(co);
    }


    group.run();
}