// ============================================================================
//  Copyright (C) 2001-2014 by Jozef Starosczyk
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
#ifndef __TOVERLAPPEDDATABUFFERQUEUE_H__
#define __TOVERLAPPEDDATABUFFERQUEUE_H__

#include <deque>
#include "TEvent.h"

namespace chcore
{
	class TOverlappedDataBuffer;

	class TOverlappedDataBufferQueue
	{
	public:
		TOverlappedDataBufferQueue();
		TOverlappedDataBufferQueue(size_t stCount, size_t stBufferSize);
		TOverlappedDataBufferQueue(const TOverlappedDataBufferQueue&) = delete;
		~TOverlappedDataBufferQueue();

		TOverlappedDataBufferQueue& operator=(const TOverlappedDataBufferQueue&) = delete;

		void ReinitializeBuffers(size_t stCount, size_t stBufferSize);
		size_t GetTotalBufferCount() const;
		size_t GetAvailableBufferCount() const;
		size_t GetSingleBufferSize() const;

		// buffer management
		void AddBuffer(TOverlappedDataBuffer* pBuffer);
		TOverlappedDataBuffer* GetBuffer();

		bool AreAllBuffersAccountedFor() const;

		// event access
		HANDLE GetEventHasBuffers() const { return m_eventHasBuffers.Handle(); }
		HANDLE GetEventAllBuffersAccountedFor() const { return m_eventAllBuffersAccountedFor.Handle(); }

		void WaitForMissingBuffers(HANDLE hKillEvent) const;

	private:
		void UpdateAllBuffersAccountedFor();
		void UpdateHasBuffers();

	private:
		std::vector<std::unique_ptr<TOverlappedDataBuffer>> m_listAllBuffers;

		std::deque<TOverlappedDataBuffer*> m_dequeBuffers;

		TEvent m_eventHasBuffers;
		TEvent m_eventAllBuffersAccountedFor;
	};

	using TOverlappedDataBufferQueuePtr = std::shared_ptr<TOverlappedDataBufferQueue>;
}

#endif
