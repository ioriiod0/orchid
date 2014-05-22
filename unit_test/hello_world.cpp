// ====================================================================================
// Copyright (c) 2012, ioriiod0@gmail.com All rights reserved.
// File         : hello_world.cpp
// Author       : ioriiod0@gmail.com
// Last Change  : 12/18/2012 11:18 PM
// Description  : 
// ====================================================================================


#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <string>

#include "../orchid/all.hpp"


void f(orchid::coroutine_handle co) {
    std::cout<<"f:hello world"<<std::endl;
}

void f1(orchid::coroutine_handle co,const char* str) {
    std::cout<<str<<std::endl;
}

struct printer {
    printer(const std::string& s):str(s) {

    }
    void operator()(orchid::coroutine_handle co) {
        std::cout<<str<<std::endl;
    }
    std::string str;
};

void stop(orchid::coroutine_handle co) {
    co -> get_scheduler().stop();
}

int main(int argc,char* argv[]) {
    orchid::scheduler sche;
    printer f2("f2:hello world");
    sche.spawn(f,orchid::coroutine::minimum_stack_size());
    sche.spawn(boost::bind(f1,_1,"f1:hello world"),orchid::coroutine::minimum_stack_size());
    sche.spawn(f2,orchid::coroutine::minimum_stack_size());
    sche.spawn(stop);
    sche.run();
    std::cout<<"done!"<<std::endl;
}