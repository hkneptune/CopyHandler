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
#ifndef __TUNORDEREDBUFFERQUEUEWRAPPERWRAPPER_H__
#define __TUNORDEREDBUFFERQUEUEWRAPPERWRAPPER_H__

#include "TEvent.h"
#include "TBufferList.h"
#include "TOrderedBufferQueue.h"

namespace chcore
{
	class TOverlappedDataBuffer;

	class TReadBufferQueueWrapper
	{
	public:
		static const unsigned long long NoPosition = 0xffffffffffffffff;

	public:
		TReadBufferQueueWrapper(const TBufferListPtr& spUnorderedQueue, unsigned long long ullNextReadPosition, DWORD dwChunkSize);

		void Push(TOverlappedDataBuffer* pBuffer, bool bKeepPosition);
		TOverlappedDataBuffer* Pop();

		size_t GetCount() const;

		void SetDataSourceFinished(TOverlappedDataBuffer* pBuffer);
		bool IsDataSourceFinished() const;

		HANDLE GetHasBuffersEvent() const;
		void ReleaseBuffers(const TBufferListPtr& spBuffers);

	private:
		bool IsBufferReady() const;
		void UpdateHasBuffers();

	private:
		TBufferListPtr m_spUnorderedQueue;		// external queue of buffers to use
		TOrderedBufferQueue m_tClaimedQueue;	// internal queue of claimed buffers

		TEvent m_eventHasBuffers;

		unsigned long long m_ullNextReadPosition = 0;	// next position for read buffers
		DWORD m_dwChunkSize = 0;

		unsigned long long m_ullDataSourceFinishedPos = NoPosition;
	};
}

#endif
