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

#include "../orchid/all.hpp"

using std::string;
using std::cerr;
using std::endl;


class chat_client {
public:
    chat_client(orchid::scheduler& sche,const string& ip,const string& port)
        :sche_(sche),
        stdin_(sche_.get_io_service(),::dup(STDIN_FILENO)),
        stdout_(sche_.get_io_service(),::dup(STDOUT_FILENO)),
        sock_(sche_.get_io_service()),
        is_logined_(false),ip_(ip),port_(port) {

    }

    ~chat_client() {
        stdin_.close();
        stdout_.close();
        sock_.close();
    }
public:
    void run() {
        sche_.spawn(boost::bind(&chat_client::handle_console,this,_1));
        sche_.spawn(boost::bind(&chat_client::handle_msg,this,_1));
        sche_.run();
    }

    void stop() {
        sche_.stop();
    }

private:
    void handle_msg(orchid::coroutine_handle co) {
        try {
            sock_.connect(ip_,port_,co);
        } catch (const orchid::io_error& e) {
            ORCHID_ERROR("err msg:%s",e.what());
            return;
        }

        orchid::writer<orchid::descriptor> out(stdout_,co);
        orchid::buffered_reader<orchid::socket> in(sock_,co);

        string line;
        try {
            for (;;) {
                in.read_until(line,'\n');
                out.write_full(line.c_str(),line.size());
            }
        } catch (const orchid::io_error& e) {
            ORCHID_ERROR("err msg:%s",e.what());
        }
        
    }

    void handle_console(orchid::coroutine_handle co) {
        orchid::buffered_reader<orchid::descriptor> in(stdin_,co);
        orchid::buffered_writer<orchid::socket> out(sock_,co);

        string str;
        char buf[1024] = {0};
        try {
            for(;;) {
                in.read_until(str,'\n');
                ORCHID_DEBUG("read from stdin:%s",str.c_str());

                if(str.empty()) continue;
                // 退出 /q
                if(str.size() >= 2 && str[0] == '/' && str[1] == 'q') { 
                    sock_.close();
                    user_.clear();
                    is_logined_ = false;
                    ORCHID_ERROR("closed!");
                    stop();
                }
                // 发送消息 /s message
                else if(str.size() >= 4 && str[0] == '/' && str[1] =='s') {
                    if(!is_logined_) {
                        ORCHID_ERROR("please login first!");
                    } else {
                        int n = sprintf(buf,"%s:%s\r\n",user_.c_str(),str.c_str()+3);
                        out.write(buf,n);
                        out.flush();
                        ORCHID_DEBUG("send:%s n:%d",buf,n);
                    }
                }
                // 登陆 /l username 
                else if(str.size() >= 4 && str[0] == '/' && str[1] == 'l') {
                    if (!is_logined_) {
                        user_.assign(str.begin()+3,str.end()-1);
                        is_logined_ = true;
                        ORCHID_DEBUG("user:%s",user_.c_str());
                    } else {
                        ORCHID_ERROR("already logined!");
                    }
                } else {
                    print_err();
                }
            }
        } catch (const orchid::io_error& e) {
            ORCHID_ERROR("err:%s",e.what());
        }

    }

    void print_err() {
        ORCHID_ERROR("usage:\r\nlogin: /l name\r\nexit: /q\r\nsend: /s ooxx");
    }

private:
    orchid::scheduler& sche_;
    orchid::descriptor stdin_;
    orchid::descriptor stdout_;
    orchid::socket sock_;
    string user_;
    bool is_logined_;
    string ip_;
    string port_;

};

int main(int argc,char* argv[]) {
    orchid::scheduler sche;
    string ip = argv[1];
    string port = argv[2];
    chat_client client(sche,ip,port);
    client.run();
}