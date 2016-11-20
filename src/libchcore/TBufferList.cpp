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
#include "stdafx.h"
#include "TBufferList.h"
#include "TCoreException.h"
#include <boost/thread/locks.hpp>

namespace chcore
{
	TBufferList::TBufferList() :
		m_eventAllBuffersAccountedFor(true, true)
	{
	}

	void TBufferList::Push(TOverlappedDataBuffer* pBuffer)
	{
		if(!pBuffer)
			throw TCoreException(eErr_InvalidArgument, L"pBuffer", LOCATION);

		{
			boost::unique_lock<boost::shared_mutex> lock(m_mutex);

			m_listBuffers.push_front(pBuffer);
			UpdateEvent();
		}

		m_notifier();
	}

	TOverlappedDataBuffer* TBufferList::Pop()
	{
		TOverlappedDataBuffer* pBuffer = nullptr;

		{
			boost::unique_lock<boost::shared_mutex> lock(m_mutex);

			if(m_listBuffers.empty())
				return nullptr;

			pBuffer = m_listBuffers.front();
			m_listBuffers.pop_front();

			UpdateEvent();
		}

		m_notifier();

		return pBuffer;
	}

	void TBufferList::Clear()
	{
		bool bRemoved = false;
		{
			boost::unique_lock<boost::shared_mutex> lock(m_mutex);

			bRemoved = !m_listBuffers.empty();
			m_listBuffers.clear();

			if(bRemoved)
				UpdateEvent();
		}

		if(bRemoved)
			m_notifier();
	}

	size_t TBufferList::GetCount() const
	{
		boost::shared_lock<boost::shared_mutex> lock(m_mutex);
		return m_listBuffers.size();
	}

	bool TBufferList::IsEmpty() const
	{
		boost::shared_lock<boost::shared_mutex> lock(m_mutex);
		return m_listBuffers.empty();
	}

	void TBufferList::SetExpectedBuffersCount(size_t stExpectedBuffers)
	{
		boost::shared_lock<boost::shared_mutex> lock(m_mutex);
		m_stExpectedBuffers = stExpectedBuffers;
		UpdateEvent();
	}

	HANDLE TBufferList::GetAllBuffersAccountedForEvent() const
	{
		return m_eventAllBuffersAccountedFor.Handle();
	}

	boost::signals2::signal<void()>& TBufferList::GetNotifier()
	{
		return m_notifier;
	}

	void TBufferList::UpdateEvent()
	{
		m_eventAllBuffersAccountedFor.SetEvent(m_listBuffers.size() == m_stExpectedBuffers);
	}
}
