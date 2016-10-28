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
#ifndef __TWRITEBUFFERQUEUEWRAPPERWRAPPER_H__
#define __TWRITEBUFFERQUEUEWRAPPERWRAPPER_H__

#include "TEvent.h"
#include "TOrderedBufferQueue.h"
#include "TBufferList.h"
#include "TSimpleOrderedBufferQueue.h"

namespace chcore
{
	class TOverlappedDataBuffer;

	class TWriteBufferQueueWrapper
	{
	public:
		TWriteBufferQueueWrapper(const TOrderedBufferQueuePtr& spQueue);
		~TWriteBufferQueueWrapper();

		void Push(TOverlappedDataBuffer* pBuffer, bool /*bKeepPosition*/) { Push(pBuffer); }
		void Push(TOverlappedDataBuffer* pBuffer);
		TOverlappedDataBuffer* Pop();

		size_t GetCount() const;
		bool IsEmpty() const;

		HANDLE GetHasBuffersEvent() const;
		void ReleaseBuffers(const TBufferListPtr& spBuffers);

	private:
		bool IsBufferReady() const;
		void UpdateHasBuffers();
		void UpdateHasBuffers(bool /*bAdded*/) { UpdateHasBuffers(); }
		TOverlappedDataBuffer* InternalPop();

	private:
		TOrderedBufferQueuePtr m_spDataQueue;	// external queue of buffers to use
		boost::signals2::connection m_emptyBuffersQueueConnector;

		TSimpleOrderedBufferQueue m_tClaimedQueue;	// internal queue of claimed buffers

		TEvent m_eventHasBuffers;
	};
}

#endif
