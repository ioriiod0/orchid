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
#include <boost/thread.hpp>

#include "../orchid/all.hpp"

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
    orchid::scheduler& main_sche_;
    orchid::scheduler_group& schedulers_;
    orchid::acceptor acceptor_;
    std::string port_;
    orchid::chan<msg_type> msg_ch_;
    client_list_type clients_;
    ///////////////////////////

    server(orchid::scheduler& sche,orchid::scheduler_group& s,const string& port)
        :main_sche_(sche),
        schedulers_(s),
        acceptor_(main_sche_.get_io_service()),
        port_(port),msg_ch_(1) {
    }
    ~server() {
        acceptor_.close();
    }

    //主服务协程，不停的从chan里面的读取消息，并处理消息。
    void process_msg(orchid::coroutine_handle co) {
        for (;;) {
            msg_type msg;
            if (!msg_ch_.recv(msg,co)) {
                ORCHID_DEBUG("chan closed");
                break;
            }

            switch (msg.which()) {
                case 0:
                    ORCHID_DEBUG("process msg:%s",boost::get<string>(msg).c_str());
                    for(typename client_list_type::iterator it = clients_.begin(); it != clients_.end(); ++it) {
                        (*it) -> ch_.send(boost::get<string>(msg),co);
                    }
                    break;
                case 1:
                    if(boost::get<ctrl_t>(msg).cmd_ == REGISTER) {
                        ORCHID_DEBUG("on register");
                        boost::get<ctrl_t>(msg).client_->start();
                        clients_.push_back(boost::get<ctrl_t>(msg).client_);
                    } else if(boost::get<ctrl_t>(msg).cmd_ == UNREGISTER) {
                        ORCHID_DEBUG("on unregister");
                        boost::get<ctrl_t>(msg).client_->close();
                        clients_.remove(boost::get<ctrl_t>(msg).client_);
                    } else {
                        throw std::runtime_error("unkonw cmd! should never hanppened!");
                    }
                    break;
                default:
                    throw std::runtime_error("unkonw msg! should never hanppened!");
            }
        }
    }

    void accept(orchid::coroutine_handle co) {
        try {
            int index = 0;
            acceptor_.bind_and_listen(port_,true);
            for (;;) {
                if(index >= schedulers_.size()) index = 0;
                boost::shared_ptr<Client> c(new Client(schedulers_[index++],*this));
                acceptor_.accept(c->sock_,co);
                ORCHID_DEBUG("on accept");
                ctrl_t msg;
                msg.cmd_ = REGISTER;
                msg.client_ = c;
                msg_ch_.send(msg,co);
            }
        } catch (boost::system::system_error& e) {
            ORCHID_ERROR("err:%s",e.what());
        }
    }

    void run() {
        main_sche_.spawn(boost::bind(&self_type::accept,this,_1));
        main_sche_.spawn(boost::bind(&self_type::process_msg,this,_1));
        boost::thread t(boost::bind(&orchid::scheduler_group::run,&schedulers_));
        main_sche_.run();
        t.join();
    }

};


struct client_agent:public boost::enable_shared_from_this<client_agent> {
    orchid::scheduler& sche_;
    server<client_agent>& server_;
    orchid::socket sock_;
    orchid::chan<string> ch_;

    client_agent(orchid::scheduler& sche,server<client_agent>& s)
        :sche_(sche),server_(s),
        sock_(sche_.get_io_service()),ch_(32) {

    }
    ~client_agent() {
        ORCHID_DEBUG("~client_agent()");
        sock_.close();
    }

    void start() {
         sche_.spawn(boost::bind(&client_agent::sender,this -> shared_from_this(),_1));
         sche_.spawn(boost::bind(&client_agent::receiver,this -> shared_from_this(),_1));
    }

    void close() {
        ch_.close();
    }

    void sender(orchid::coroutine_handle co) {
        string str;
        orchid::buffered_writer<orchid::socket> out(sock_,co);
        try {
            while(ch_.recv(str,co)) {
                out.write(str.c_str(),str.size());
                out.flush();
            }
        } catch(const orchid::io_error& e) {
            ORCHID_ERROR("err:%s",e.what());
        }
        ORCHID_DEBUG("sender: exit!");
    }

    void receiver(orchid::coroutine_handle co) {
        orchid::buffered_reader<orchid::socket> in(sock_,co);
        string str;
        try {
            for (;;) {
                in.read_until(str,'\n');
                ORCHID_DEBUG("recv:%s",str.c_str());
                server_.msg_ch_.send(str,co);
            }
        } catch(const orchid::io_error& e) {
            ORCHID_ERROR("err:%s",e.what());
        }

        server<client_agent>::ctrl_t ctrl_msg;
        ctrl_msg.cmd_ = server<client_agent>::UNREGISTER;
        ctrl_msg.client_ = this -> shared_from_this();
        server_.msg_ch_.send(ctrl_msg, co);
        ORCHID_DEBUG("receiver: exit!");
    }
};

int main(int argc,char* argv[]) {
    string port = argv[1];
    orchid::scheduler main_sche;
    orchid::scheduler_group group(4);
    server<client_agent> s(main_sche,group,port);
    s.run();

}