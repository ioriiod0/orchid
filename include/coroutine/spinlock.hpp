// ====================================================================================
// Copyright (c) 2012, ioriiod0@gmail.com All rights reserved.
// File         : spinlock.hpp
// Author       : ioriiod0@gmail.com
// Last Change  : 12/01/2012 09:14 PM
// Description  : 
// ====================================================================================


#ifndef __ORCHID_SPINLOCK_H__
#define __ORCHID_SPINLOCK_H__
#include <boost/atomic.hpp>
#include <boost/utility.hpp>

namespace orchid {namespace detail {

class spinlock:boost::noncopyable {
private:
  typedef enum {Locked, Unlocked} LockState;
  boost::atomic<LockState> state_;

public:
  spinlock() : state_(Unlocked) {}
  
  void lock()
  {
    while (state_.exchange(Locked, boost::memory_order_acquire) == Locked) {
      /* busy-wait */
    }
  }
  void unlock()
  {
    state_.store(Unlocked, boost::memory_order_release);
  }
};


}}


#endif