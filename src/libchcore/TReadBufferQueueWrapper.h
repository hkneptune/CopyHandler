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
#include "TSimpleOrderedBufferQueue.h"

namespace chcore
{
	class TOverlappedDataBuffer;

	class TReadBufferQueueWrapper
	{
	public:
		static const unsigned long long NoPosition = 0xffffffffffffffff;

	public:
		TReadBufferQueueWrapper(const TBufferListPtr& spEmptyBuffers,
			unsigned long long ullNextReadPosition, DWORD dwChunkSize,
			size_t stMaxOtfBuffers, size_t stMaxReadAheadBuffers,
			TSharedCountPtr<size_t> spOtfBuffersCount, TSharedCountMTPtr<size_t> spCurrentReadAheadBuffers);
		~TReadBufferQueueWrapper();

		void Push(TOverlappedDataBuffer* pBuffer);
		void PushEmpty(TOverlappedDataBuffer* pBuffer);

		TOverlappedDataBuffer* Pop();

		void SetDataSourceFinished(TOverlappedDataBuffer* pBuffer);
		bool IsDataSourceFinished() const;

		HANDLE GetHasBuffersEvent() const;
		void ClearBuffers();

		void UpdateProcessingRange(unsigned long long ullNewPosition);

	private:
		void UpdateHasBuffers();

	private:
		// external buffers
		TBufferListPtr m_spEmptyBuffers;		// external queue of buffers to use
		boost::signals2::connection m_emptyBuffersQueueConnector;

		// retry buffers
		TSimpleOrderedBufferQueue m_tRetryBuffers;	// internal queue of claimed buffers
		boost::signals2::connection m_retryBuffersConnector;

		// input
		unsigned long long m_ullNextReadPosition = 0;	// next position for read buffers
		DWORD m_dwChunkSize = 0;

		// config
		size_t m_stMaxOtfBuffers = 0;
		size_t m_stMaxReadAheadBuffers = 0;

		// internal state
		unsigned long long m_ullDataSourceFinishedPos = NoPosition;

		// external state
		TSharedCountPtr<size_t> m_spOtfBuffersCount;
		boost::signals2::connection m_otfBuffersConnector;

		TSharedCountMTPtr<size_t> m_spCurrentReadAheadBuffers;
		boost::signals2::connection m_currentReadAheadConnector;

		// events
		TEvent m_eventHasBuffers;
	};
}

#endif
