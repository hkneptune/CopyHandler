// ============================================================================
//  Copyright (C) 2001-2016 by Jozef Starosczyk
//  ixen@copyhandler.com
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU Library General Public License
//  (version 2) as published by the Free Software Foundation;
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU Library General Public
//  License along with this program; if not, write to the
//  Free Software Foundation, Inc.,
//  59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// ============================================================================
#ifndef __WAITABLEQUEUE_H__
#define __WAITABLEQUEUE_H__

#include <type_traits>
#include <mutex>
#include <deque>
#include "TEvent.h"

namespace chcore
{
	template<class T>
	class WaitableQueue
	{
	public:
		WaitableQueue() :
			m_eventHasEntries(true, false)
		{
		}

		void PushBack(T&& arg)
		{
			std::lock_guard<std::mutex> lock(m_lock);
			m_queue.push_back(std::forward<T>(arg));
			m_eventHasEntries.SetEvent();
		}

		T PopFront()
		{
			std::lock_guard<std::mutex> lock(m_lock);

			T value = std::move(m_queue.front());
			m_queue.pop_front();

			m_eventHasEntries.SetEvent(!m_queue.empty());

			return value;
		}

		HANDLE GetWaitHandle() const
		{
			return m_eventHasEntries.Handle();
		}

	private:
		std::deque<T> m_queue;
		TEvent m_eventHasEntries;
		std::mutex m_lock;
	};
}

#endif
