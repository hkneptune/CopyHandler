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
#include "TSharedCountMT.h"

namespace chcore
{
	class TOverlappedDataBuffer;

	class TBufferList
	{
	public:
		TBufferList() : 
			m_spCount(std::make_shared<TSharedCountMT<size_t>>())
		{
		}

		void Push(TOverlappedDataBuffer* pBuffer)
		{
			if(!pBuffer)
				throw TCoreException(eErr_InvalidArgument, L"pBuffer", LOCATION);

			boost::unique_lock<boost::mutex> lock(m_mutex);

			m_queueBuffers.push_front(pBuffer);
			m_spCount->Increase();
		}

		TOverlappedDataBuffer* Pop()
		{
			TOverlappedDataBuffer* pBuffer = nullptr;

			{
				boost::unique_lock<boost::mutex> lock(m_mutex);

				if(m_queueBuffers.empty())
					return nullptr;

				pBuffer = m_queueBuffers.front();
				m_queueBuffers.pop_front();
				m_spCount->Decrease();
			}

			return pBuffer;
		}

		void Clear()
		{
			boost::unique_lock<boost::mutex> lock(m_mutex);

			m_queueBuffers.clear();
			m_spCount->SetValue(0);
		}

		size_t GetCount() const
		{
			boost::unique_lock<boost::mutex> lock(m_mutex);
			return m_spCount->GetValue();
		}

		bool IsEmpty() const
		{
			boost::unique_lock<boost::mutex> lock(m_mutex);
			return m_spCount->GetValue() == 0;
		}

		void SetExpectedBuffersCount(size_t stExpectedBuffers) // thread-unsafe by design
		{
			boost::unique_lock<boost::mutex> lock(m_mutex);
			m_stExpectedBuffers = stExpectedBuffers;
		}

		bool AreAllBuffersAccountedFor() const
		{
			boost::unique_lock<boost::mutex> lock(m_mutex);
			return m_stExpectedBuffers == m_spCount->GetValue();
		}

		TSharedCountMTPtr<size_t> GetSharedCount()
		{
			return m_spCount;
		}

	private:
		mutable boost::mutex m_mutex;
		TSharedCountMTPtr<size_t> m_spCount;

		size_t m_stExpectedBuffers = 0;		// count of buffers there should be in m_queueBuffers when no buffer is in use
		std::deque<TOverlappedDataBuffer*> m_queueBuffers;
	};

	using TBufferListPtr = std::shared_ptr<TBufferList>;
}

#endif
