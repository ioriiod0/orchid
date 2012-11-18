// ====================================================================================
// Copyright (c) 2012, ioriiod0@gmail.com All rights reserved.
// File         : test_chan.cpp
// Author       : ioriiod0@gmail.com
// Last Change  : 11/19/2012 12:35 AM
// Description  : 
// ====================================================================================

#include <string>
#include <iostream>

#include "../include/all.hpp"

using std::string;
using std::cout;
using std::endl;

class sender_coroutine : public orchid::coroutine {
public:
    sender_coroutine(orchid::scheduler& sche,int id,orchid::chan<int>* chan):
        orchid::coroutine(sche),id_(id),chan_(chan) {

    }

    virtual ~sender_coroutine() {
        cout<<"i am done!!"<<endl;
    }

    virtual void run() {
        try {
            int i = 0;
            for (;;) {
                cout<<"sender "<<id_<<" send: "<<i<<endl;
                chan_ -> put(i++,this);
            }
        } catch (...) {
            cout<<"error happened!!"<<endl;

        }

    }

    int id_;
    orchid::chan<int>* chan_;

};


class receiver_coroutine : public orchid::coroutine {
public:
    receiver_coroutine(orchid::scheduler& sche,int id,orchid::chan<int>* chan):
        orchid::coroutine(sche),id_(id),chan_(chan) {

    }

    virtual ~receiver_coroutine() {
        cout<<"i am done!!"<<endl;
    }

    virtual void run() {
        try {
            int i;
            for (;;) {
                cout<<"receiver "<<id_<<" reveive: "<<i<<endl;
                chan_ -> get(i,this);
            }
        } catch (...) {
            cout<<"error happened!!"<<endl;

        }

    }

    int id_;
    orchid::chan<int>* chan_;

};


int main() {
    orchid::scheduler sche;
    orchid::chan<int> ch(100);

    for (int i=0;i<2;++i) {
        sender_coroutine* co = new sender_coroutine(sche,i,&ch);
        sche.spawn(co);
    }

    for (int i=0;i<1;++i) {
        receiver_coroutine* co = new receiver_coroutine(sche,i,&ch);
        sche.spawn(co);
    }


    sche.run();
}