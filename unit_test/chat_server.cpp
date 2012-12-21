// ====================================================================================
// Copyright (c) 2012, ioriiod0@gmail.com All rights reserved.
// File         : chat_server.cpp
// Author       : ioriiod0@gmail.com
// Last Change  : 11/30/2012 12:13 AM
// Description  : 
// ====================================================================================

#include <stdio.h>
#include <iostream>
#include <string>
#include <algorithm>
#include <iterator>
#include <vector>
#include <boost/bind.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/variant.hpp>

#include "../include/all.hpp"

using std::string;
using std::cerr;
using std::cout;
using std::endl;

const static std::size_t STACK_SIZE = 64*1024;

template <typename Client>
struct server {
    typedef server<Client> self_type;
    typedef boost::shared_ptr<Client> client_sp_type;
    typedef std::list<client_sp_type> client_list_type;
    enum {REGISTER,UNREGISTER};
    struct ctrl_t {
        int cmd_;
        client_sp_type client_;
    };
    typedef boost::variant<string,ctrl_t> msg_type;

    ///////////////////////
    orchid::scheduler_group& schedulers_;
    orchid::acceptor acceptor_;
    std::string port_;
    orchid::chan<msg_type> msg_ch_;
    client_list_type clients_;
    ///////////////////////////

    server(orchid::scheduler_group& s,const string& port)
        :schedulers_(s),acceptor_(schedulers_[0].get_io_service()),port_(port),msg_ch_(512) {
    }
    ~server() {
    }

    void process_msg(orchid::coroutine_handle co) {
        msg_type msg;
        for (;;) {
            msg_ch_.recv(msg,co);
            if(msg.which() == 0) {// string
                cout<<boost::get<string>(msg)<<endl;
                for(typename client_list_type::iterator it = clients_.begin(); it != clients_.end(); ++it) {
                    (*it) -> ch_.send(boost::get<string>(msg),co);
                }
            } else if(msg.which() == 1) {//ctrl_t
                if(boost::get<ctrl_t>(msg).cmd_ == REGISTER) {
                    cout<<"on register"<<endl;
                    clients_.push_back(boost::get<ctrl_t>(msg).client_);
                } else if(boost::get<ctrl_t>(msg).cmd_ == UNREGISTER) {
                    cout<<"on unregister"<<endl;
                    boost::get<ctrl_t>(msg).client_ -> ch_.close();
                    clients_.remove(boost::get<ctrl_t>(msg).client_);
                } else {
                    throw std::runtime_error("unkonw cmd! should never hanppened!");
                }
            } else {
                throw std::runtime_error("unkonw msg! should never hanppened!");
            }
        }
    }

    void accept(orchid::coroutine_handle co) {
    try {
        int index = 1;
        acceptor_.bind_and_listen(port_);
        for (;;) {
            if(index >= schedulers_.size()) index = 0;
            boost::shared_ptr<Client> c(new Client(schedulers_[index++],*this));
            acceptor_.accept(c->sock_,co);
            cout<<"on accept!"<<endl;
            c -> start();
            ctrl_t msg;
            msg.cmd_ = REGISTER;
            msg.client_ = c;
            msg_ch_.send(msg,co);
        }
    } catch (boost::system::system_error& e) {
        cout<<e.code()<<" "<<e.what()<<endl;
    }
}

    void run() {
        schedulers_[0].spawn(boost::bind(&self_type::accept,this,_1),STACK_SIZE);
        schedulers_[0].spawn(boost::bind(&self_type::process_msg,this,_1),STACK_SIZE);
        schedulers_.run();
    }

};


struct client:public boost::enable_shared_from_this<client> {
    orchid::scheduler& sche_;
    server<client>& server_;
    orchid::socket sock_;
    orchid::chan<string> ch_;

    client(orchid::scheduler& sche,server<client>& s)
        :sche_(sche),server_(s),
        sock_(sche_.get_io_service()),ch_(32) {

    }
    ~client() {
        cout<<"client's done!"<<endl;
    }

    void start() {
         sche_.spawn(boost::bind(&client::sender,this -> shared_from_this(),_1),STACK_SIZE);
         sche_.spawn(boost::bind(&client::receiver,this -> shared_from_this(),_1),STACK_SIZE);
    }

    void sender(orchid::coroutine_handle& co) {
        string str;
        orchid::tcp_ostream out(sock_,co);
        try {
            while(ch_.recv(str,co)) {
                out<<str<<endl;
            }
        } catch (boost::system::system_error& e) {
            cerr<<e.code()<<" "<<e.what()<<endl;
        }
    }

    void receiver(orchid::coroutine_handle& co) {
        try {
            orchid::tcp_istream in(sock_,co);
            for (string str;std::getline(in,str);) {
                server_.msg_ch_.send(str,co);
            }

        } catch (boost::system::system_error& e) {
            cerr<<e.code()<<" "<<e.what()<<endl;
        }
        if(sock_.is_open())
            sock_.close();
        server<client>::ctrl_t ctrl_msg;
        ctrl_msg.cmd_ = server<client>::UNREGISTER;
        ctrl_msg.client_ = this -> shared_from_this();
        server_.msg_ch_.send(ctrl_msg, co);

    }
};

int main(int argc,char* argv[]) {
    string port = argv[1];
    orchid::scheduler_group group(4);
    server<client> s(group,port);
    s.run();

}