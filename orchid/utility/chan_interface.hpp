// ====================================================================================
// Copyright (c) 2012, ioriiod0@gmail.com All rights reserved.
// File         : chan_interface.hpp
// Author       : ioriiod0@gmail.com
// Last Change  : 11/29/2012 04:15 PM
// Description  : 
// ====================================================================================


#ifndef __ORCHID_CHAN_INTERFACE_H__
#define __ORCHID_CHAN_INTERFACE_H__

#include "../coroutine/coroutine_handle.hpp"

namespace orchid {namespace detail {

template <typename Chan>
class chan_receiver_basic {
public:
    typedef coroutine_handle* coroutine_pointer;
    typedef typename Chan::value_type value_type;

public:
    chan_receiver_basic(Chan& ch,coroutine_pointer co)
        :ch_(ch),co_(co) {}
    ~chan_receiver_basic() {}

public:

    void recv(value_type& t) {
        ch_.recv(t,*co_);
    }

    bool peek(value_type& t) {
        return ch_.peek();
    }
private:
    Chan& ch_;
    coroutine_pointer co_;
};



template <typename Chan>
class chan_sender_basic {
public:
    typedef coroutine_handle* coroutine_pointer;
    typedef typename Chan::value_type value_type;

public:
    chan_sender_basic(Chan& ch,coroutine_pointer co)
        :ch_(ch),co_(co) {}
    ~chan_sender_basic() {}

public:
    void send(const value_type& t) {
        ch_.send(t,*co_);
    }

    bool post(const value_type& t) {
        return ch_.post(t);
    }

private:
    Chan& ch_;
    coroutine_pointer co_;
};


}}




#endif