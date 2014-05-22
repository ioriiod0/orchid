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


const static std::size_t STACK_SIZE = 16*1024;



typedef boost::shared_ptr<orchid::socket> socket_ptr;

void handle_io(orchid::coroutine_handle co,socket_ptr sock) {
    orchid::buffered_reader<orchid::socket> reader(*sock,co,16);
    orchid::buffered_writer<orchid::socket> writer(*sock,co,16);

    try {
        std::string line;
        std::size_t n = 0;

        for(;;) {
            n = reader.read_until(line,'\n');
            ORCHID_DEBUG("id %lu recv: %s",co->id(),line.c_str());
            writer.write(line.c_str(),line.size());
            writer.flush();
        }

    } catch (const orchid::io_error& e) {
        if (e.code() == boost::asio::error::eof) {
            ORCHID_DEBUG("id %lu msg:%s",co->id(),"socket closed by remote side!");
        } else {
            ORCHID_ERROR("id %lu msg:%s",co->id(),e.what());
        }
    }

}

void handle_accept(orchid::coroutine_handle co) {
    try {
        orchid::acceptor acceptor(co -> get_io_service());
        acceptor.bind_and_listen("5678",false);
        for(;;) {
            socket_ptr sock(new orchid::socket(co -> get_io_service()));
            acceptor.accept(*sock,co);
            co -> get_scheduler().spawn(boost::bind(handle_io,_1,sock));
        }
    }
    catch(orchid::io_error& e) {
        ORCHID_ERROR("id %lu msg:%s",co->id(),e.what());
    }
}

int main() {

    orchid::scheduler sche;
    ORCHID_DEBUG("id:%lu",sche.id());
    sche.spawn(handle_accept,orchid::coroutine::minimum_stack_size());
    sche.run();
}