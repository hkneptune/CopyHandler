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
#ifndef __TBUFFERLIST_H__
#define __TBUFFERLIST_H__

#include <boost/signals2/signal.hpp>
#include <deque>
#include "TCoreException.h"
#include <boost/thread/locks.hpp>

namespace chcore
{
	class TOverlappedDataBuffer;

	class TBufferList
	{
	public:
		TBufferList()
		{
		}

		void Push(TOverlappedDataBuffer* pBuffer)
		{
			if(!pBuffer)
				throw TCoreException(eErr_InvalidArgument, L"pBuffer", LOCATION);

			{
				boost::unique_lock<boost::shared_mutex> lock(m_mutex);

				m_queueBuffers.push_front(pBuffer);
			}

			m_notifier();
		}

		TOverlappedDataBuffer* Pop()
		{
			TOverlappedDataBuffer* pBuffer = nullptr;

			{
				boost::unique_lock<boost::shared_mutex> lock(m_mutex);

				if(m_queueBuffers.empty())
					return nullptr;

				pBuffer = m_queueBuffers.front();
				m_queueBuffers.pop_front();
			}

			m_notifier();

			return pBuffer;
		}

		void Clear()
		{
			bool bRemoved = false;
			{
				boost::unique_lock<boost::shared_mutex> lock(m_mutex);

				bRemoved = !m_queueBuffers.empty();
				m_queueBuffers.clear();
			}

			if(bRemoved)
				m_notifier();
		}

		size_t GetCount() const
		{
			boost::shared_lock<boost::shared_mutex> lock(m_mutex);
			return m_queueBuffers.size();
		}

		bool IsEmpty() const
		{
			boost::shared_lock<boost::shared_mutex> lock(m_mutex);
			return m_queueBuffers.empty();
		}

		void SetExpectedBuffersCount(size_t stExpectedBuffers) // thread-unsafe by design
		{
			boost::unique_lock<boost::shared_mutex> lock(m_mutex);
			m_stExpectedBuffers = stExpectedBuffers;
		}

		bool AreAllBuffersAccountedFor() const
		{
			boost::shared_lock<boost::shared_mutex> lock(m_mutex);
			return m_stExpectedBuffers == m_queueBuffers.size();
		}

		boost::signals2::signal<void()>& GetNotifier()
		{
			return m_notifier;
		}

	private:
		mutable boost::shared_mutex m_mutex;

		size_t m_stExpectedBuffers = 0;		// count of buffers there should be in m_queueBuffers when no buffer is in use
		std::deque<TOverlappedDataBuffer*> m_queueBuffers;

		boost::signals2::signal<void()> m_notifier;
	};

	using TBufferListPtr = std::shared_ptr<TBufferList>;
}

#endif
