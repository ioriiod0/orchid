// ====================================================================================
// Copyright (c) 2012, ioriiod0@gmail.com All rights reserved.
// File         : test_server.cpp
// Author       : ioriiod0@gmail.com
// Last Change  : 11/17/2012 06:11 PM
// Description  : 
// ====================================================================================


#include <string>
#include <iostream>
#include <boost/iostreams/stream.hpp>


#include "../include/all.hpp"

using std::string;
using std::cout;
using std::endl;



class client_coroutine : public orchid::coroutine {
public:
    client_coroutine(orchid::scheduler& sche,orchid::socket* sock):
        orchid::coroutine(sche),
        sock_(sock) {
        sock_ -> attach(this);
    }

    virtual ~client_coroutine() {
        delete sock_;
        cout<<"i am done!!"<<endl;
    }

    virtual void run() {
        try {
            string str;
            for (;;) {
                boost::iostreams::stream<orchid::tcp_device> tcp_stream(*sock_);
                tcp_stream >> str;
                cout << str << endl;
                tcp_stream << "pong" <<endl;
            }
        } catch (boost::system::system_error& e) {
            cout<<e.code()<<" "<<e.what()<<endl;
        }
    }

    orchid::socket* sock_;
};

class server_coroutine: public orchid::coroutine {
public:
    server_coroutine(orchid::scheduler& sche):
        orchid::coroutine(sche),
        acceptor_(sche.get_io_service(),this) {

        }
    virtual ~server_coroutine() {

    }
public:
    virtual void run() {
        try {
            acceptor_.bind_and_listen("5678");
            for(;;) {
                orchid::socket* sock = acceptor_.accept();
                cout<<"accept success!"<<endl;
                client_coroutine* co = new client_coroutine(get_scheduler(),sock);
                get_scheduler().spawn(co);
            }
        }
        catch(boost::system::system_error& e) {
            cout<<e.code()<<" "<<e.what()<<endl;
        }
    }
private:
    orchid::acceptor acceptor_;
};



int main() {
    orchid::scheduler sche;
    server_coroutine* co = new server_coroutine(sche);
    sche.spawn(co);
    sche.run();
}