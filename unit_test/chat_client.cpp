// ====================================================================================
// Copyright (c) 2012, ioriiod0@gmail.com All rights reserved.
// File         : chat_client.cpp
// Author       : ioriiod0@gmail.com
// Last Change  : 11/28/2012 05:55 PM
// Description  : 
// ====================================================================================

#include <stdio.h>
#include <iostream>
#include <string>
#include <algorithm>
#include <iterator>
#include <vector>
#include <boost/scoped_ptr.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/algorithm/string.hpp>

#include "../include/all.hpp"

using std::string;
using std::cerr;
using std::endl;


const static std::size_t STACK_SIZE = 64*1024;



class chat_client {
public:
    chat_client(const string& ip,const string& port)
        :sche_(),
        stdin_(sche_.get_io_service(),STDIN_FILENO),
        stdout_(sche_.get_io_service(),STDOUT_FILENO),
        sock_(sche_.get_io_service()),
        is_logined_(false),ip_(ip),port_(port) {

    }

    ~chat_client() {

    }
public:
    void run() {
        sche_.spawn(boost::bind(&chat_client::handle_console,this,_1),STACK_SIZE);
        sche_.run();
    }

    void stop() {
        sche_.stop();
    }

private:
    void receive_msg(orchid::coroutine_handle co) {
            string str;
            orchid::descriptor_ostream out(stdout_,co);
            orchid::tcp_istream in(sock_,co);
            //in.exceptions(std::istream::badbit | std::istream::failbit);
            std::getline(in, str);
            while (in) {
                out<<str<<endl;
                std::getline(in, str);
            }
    }

    void handle_console(orchid::coroutine_handle co) {
        string str;
        orchid::descriptor_istream in(stdin_,co);
        orchid::tcp_ostream out(sock_,co);
        try {
            sock_.connect(ip_,port_,co);
        } catch (boost::system::system_error& e) {
            cerr<<e.code()<<" "<<e.what()<<endl;
            return;
        }

        sche_.spawn(boost::bind(&chat_client::receive_msg,this,_1), STACK_SIZE);
        std::getline(in,str);
        while(in) {
            if(str.empty()) continue;
            if(str.size() >= 2 && str[0] == '/' && str[1] == 'q') {
                sock_.close();
                user_.clear();
                is_logined_ = false;
                cerr<<"closed"<<endl;
                stop();
            } else if(str.size() >= 4 && str[0] == '/' && str[1] =='s') {
                if(!is_logined_) {
                    cerr<<"login first"<<endl;
                } else {
                    out<<user_<<":"<<str.substr(3)<<endl;
                }
            } else if(str.size() >= 4 && str[0] == '/' && str[1] == 'l') {
                if (!is_logined_) {
                    user_ = str.substr(3);
                    is_logined_ = true;
                } else {
                    cerr<<"err: already logined!"<<endl;
                }
            } else {
                print_err();
            }
            std::getline(in,str);
        }

    }

    void print_err() {
           cerr<<"err: bad cmd!"<<endl
            <<"usage:"<<endl
            <<"login: /l 127.0.0.0.1 5678 name"<<endl
            <<"exit: /q"<<endl
            <<"send: /s xxxxxxxxxxxx"<<endl;
    }

private:
    orchid::scheduler sche_;
    orchid::descriptor stdin_;
    orchid::descriptor stdout_;
    orchid::socket sock_;
    string user_;
    bool is_logined_;
    string ip_;
    string port_;

};

int main(int argc,char* argv[]) {
    string ip = argv[1];
    string port = argv[2];
    chat_client client(ip,port);
    client.run();
}