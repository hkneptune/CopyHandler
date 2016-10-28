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
#include "TEvent.h"

namespace chcore
{
	class TOverlappedDataBuffer;

	class TBufferList
	{
	public:
		TBufferList();

		void Push(TOverlappedDataBuffer* pBuffer);
		TOverlappedDataBuffer* Pop();

		void Clear();

		size_t GetCount() const;
		bool IsEmpty() const;

		void SetExpectedBuffersCount(size_t stExpectedBuffers) { m_stExpectedBuffers = stExpectedBuffers; }
		HANDLE GetAllBuffersAccountedForEvent() const { return m_eventAllBuffersAccountedFor.Handle(); }

		boost::signals2::signal<void(bool bAdded)>& GetNotifier();

	private:
		void UpdateEvent();

	private:
		size_t m_stExpectedBuffers = 0;		// count of buffers there should be in m_listBuffers when no buffer is in use
		std::list<TOverlappedDataBuffer*> m_listBuffers;

		boost::signals2::signal<void(bool bAdded)> m_notifier;
		TEvent m_eventAllBuffersAccountedFor;
	};

	using TBufferListPtr = std::shared_ptr<TBufferList>;
}

#endif