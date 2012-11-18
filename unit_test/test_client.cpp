// ====================================================================================
// Copyright (c) 2012, ioriiod0@gmail.com All rights reserved.
// File         : test_client.cpp
// Author       : ioriiod0@gmail.com
// Last Change  : 11/17/2012 07:51 PM
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
    client_coroutine(orchid::scheduler& sche):
        orchid::coroutine(sche),
        connector_(sche.get_io_service(),this),
        sock_(NULL) {

    }

    virtual ~client_coroutine() {
        delete sock_;
        cout<<"i am done!!"<<endl;
    }

    virtual void run() {
        try {
            string str;
            sock_ = connector_.connect("127.0.0.1","5678");
            sock_ -> attach(this);
            cout<<"connect success!"<<endl;
            for (;;) {
                boost::iostreams::stream<orchid::tcp_device> tcp_stream(*sock_);
                tcp_stream << "ping" <<endl;
                tcp_stream >> str;
                cout << str << endl;
            }
        } catch (boost::system::system_error& e) {
            cout<<e.code()<<" "<<e.what()<<endl;

        }

    }

    orchid::connector connector_;
    orchid::socket* sock_;
};

int main() {
    orchid::scheduler sche;
    for (int i=0;i<100;++i) {
        client_coroutine* co = new client_coroutine(sche);
        sche.spawn(co);
    }
    sche.run();
}