// ====================================================================================
// Copyright (c) 2012, ioriiod0@gmail.com All rights reserved.
// File         : spinlock.hpp
// Author       : ioriiod0@gmail.com
// Last Change  : 12/01/2012 09:14 PM
// Description  : 
// ====================================================================================


#ifndef __ORCHID_SPINLOCK_H__
#define __ORCHID_SPINLOCK_H__
#include <boost/smart_ptr/detail/spinlock.hpp>
namespace orchid {namespace detail {
class spinlock:boost::noncopyable {
private:
	boost::detail::spinlock lock_;
public:
	spinlock(){ lock_ = BOOST_DETAIL_SPINLOCK_INIT; }
  
  void lock()
  {
	  lock_.lock();
  }
  void unlock()
  {
	  lock_.unlock();
  }
};


}}


#endif