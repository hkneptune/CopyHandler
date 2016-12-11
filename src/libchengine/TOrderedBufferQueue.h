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
#ifndef __TORDEREDBUFFERQUEUE_H__
#define __TORDEREDBUFFERQUEUE_H__

#include <set>
#include "TEvent.h"
#include "TOverlappedDataBuffer.h"
#include "TBufferList.h"
#include <boost/thread/recursive_mutex.hpp>

namespace chengine
{
	class TOrderedBufferQueue
	{
	public:
		static const unsigned long long NoPosition = 0xffffffffffffffff;

	public:
		explicit TOrderedBufferQueue(const TBufferListPtr& spEmptyBuffers, unsigned long long ullExpectedPosition);
		~TOrderedBufferQueue();

		void Push(TOverlappedDataBuffer* pBuffer);

		template<class T>
		void PushError(TOverlappedDataBuffer* pBuffer, T& rRetryQueue);

		TOverlappedDataBuffer* Pop();
		TOverlappedDataBuffer* PopError();

		const TOverlappedDataBuffer* Peek() const;

		size_t GetCount() const;
		bool IsEmpty() const;
		bool HasPoppableBuffer() const;

		HANDLE GetHasBuffersEvent() const;
		HANDLE GetHasErrorEvent() const;
		HANDLE GetHasReadingFinished() const;

		void ClearBuffers();

		void UpdateProcessingRange(unsigned long long ullNewPosition);

		TSharedCountMTPtr<size_t> GetSharedCount();

	private:
		void UpdateHasBuffers();
		void UpdateHasErrors();
		void UpdateReadingFinished();

		bool InternalHasPoppableBuffer() const;

	private:
		using BufferCollection = std::set<TOverlappedDataBuffer*, CompareBufferPositions>;
		BufferCollection m_setBuffers;
		TSharedCountMTPtr<size_t> m_spBuffersCount;

		TBufferListPtr m_spEmptyBuffers;

		mutable boost::recursive_mutex m_mutex;

		TOverlappedDataBuffer* m_pFirstErrorBuffer = nullptr;
		unsigned long long m_ullErrorPosition = NoPosition;

		TEvent m_eventHasBuffers;
		TEvent m_eventHasError;
		TEvent m_eventHasReadingFinished;

		unsigned long long m_ullExpectedBufferPosition = 0;
		bool m_bDataSourceFinished = false;
	};

	template<class T>
	void TOrderedBufferQueue::PushError(TOverlappedDataBuffer* pBuffer, T& rRetryQueue)
	{
		if(!pBuffer)
			throw TCoreException(eErr_InvalidArgument, L"pBuffer is NULL", LOCATION);
		if(!pBuffer->HasError())
			throw TCoreException(eErr_InvalidArgument, L"Cannot push successful buffer to failed queue", LOCATION);

		boost::unique_lock<boost::recursive_mutex> lock(m_mutex);

		if(!m_pFirstErrorBuffer && m_ullErrorPosition == NoPosition)
		{
			m_pFirstErrorBuffer = pBuffer;
			m_ullErrorPosition = pBuffer->GetFilePosition();
			UpdateHasErrors();
			return;
		}

		if(pBuffer->GetFilePosition() < m_ullErrorPosition)
		{
			if(m_pFirstErrorBuffer)
			{
				// if there is no ptr set then it is being processed somewhere and will be handled separately
				m_pFirstErrorBuffer->SetErrorCode(ERROR_SUCCESS);
				rRetryQueue.Push(m_pFirstErrorBuffer);
			}
			m_pFirstErrorBuffer = pBuffer;
			m_ullErrorPosition = pBuffer->GetFilePosition();
		}
		else if(pBuffer->GetFilePosition() > m_ullErrorPosition)
		{
			pBuffer->SetErrorCode(ERROR_SUCCESS);
			rRetryQueue.Push(pBuffer);
		}
		else if(!m_pFirstErrorBuffer)
			m_pFirstErrorBuffer = pBuffer;		// restore the buffer 

		UpdateHasErrors();
	}

	using TOrderedBufferQueuePtr = std::shared_ptr<TOrderedBufferQueue>;
}

#endif
