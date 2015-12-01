// ====================================================================================
// Copyright (c) 2012, ioriiod0@gmail.com All rights reserved.
// File         : chan.hpp
// Author       : ioriiod0@gmail.com
// Last Change  : 11/18/2012 01:35 AM
// Description  : 
// ====================================================================================

#ifndef __ORCHID_CHAN_MAP_H__
#define __ORCHID_CHAN_MAP_H__

#include <iostream>
#include <memory>
#include <map>

#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>

#include "chan.hpp"
#include "spinlock.hpp"


namespace orchid {
	namespace detail {
		
		template <typename CO, typename K, typename T>
		class chan_map_basic :boost::noncopyable
		{
		public:
			typedef chan_map_basic<CO, K, T> self_type;
			typedef chan_basic<CO, T> chan_type;
			typedef K key_type;
			typedef T value_type;
			typedef CO coroutine_pointer;
		public:
			chan_map_basic(std::size_t chan_cap)
				:chan_cap_(chan_cap) {
			}
			~chan_map_basic() {
			}
		public:
			bool open(const K& k)
			{
				locker_.lock();
				std::pair<typename std::map<key_type, boost::shared_ptr<chan_type> >::iterator, bool> ret = map_.insert(std::make_pair(k, boost::shared_ptr<chan_type>()));
				if (ret.second)
					ret.first->second.reset(new chan_type(1));
				locker_.unlock();
				return ret.second;
			}
			void close(const K& k) {
				locker_.lock();
				typename std::map<key_type, boost::shared_ptr<chan_type> >::iterator it = map_.find(k);
				if (it != map_.end())
				{
					it->second->close();
					map_.erase(it);
				}
				locker_.unlock();
			}
			bool send(const K& k, const T& t, coroutine_pointer co) {
				bool ret = false;
				locker_.lock();
				typename std::map<key_type, boost::shared_ptr<chan_type> >::iterator it = map_.find(k);
				if (it != map_.end())
				{
					boost::shared_ptr<chan_type> chanptr(it->second);
					locker_.unlock();
					ret = chanptr->send(t, co);
				}
				else
				{
					locker_.unlock();
				}
				return ret;
			}
			bool recv(const K& k, T& t, coroutine_pointer co) {
				bool ret = false;
				locker_.lock();
				typename std::map<key_type, boost::shared_ptr<chan_type> >::iterator it = map_.find(k);
				if (it != map_.end())
				{
					boost::shared_ptr<chan_type> chanptr(it->second);
					locker_.unlock();
					ret = chanptr->recv(t, co);
				}
				else
				{
					locker_.unlock();
				}
				return ret;
			}
		private:
			spinlock locker_;
			std::map<key_type, boost::shared_ptr<chan_type> > map_;
			std::size_t chan_cap_;
		};



	}
}

#endif//__ORCHID_CHAN_MAP_H__