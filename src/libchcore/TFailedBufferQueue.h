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
#ifndef __TFAILEDBUFFERQUEUE_H__
#define __TFAILEDBUFFERQUEUE_H__

#include <set>
#include "TEvent.h"
#include "TOverlappedDataBuffer.h"
#include "TCoreException.h"
#include "TBufferList.h"

namespace chcore
{
	class TFailedBufferQueue
	{
	public:
		static const unsigned long long NoPosition = 0xffffffffffffffff;

	public:
		TFailedBufferQueue();

		template<class T>
		void PushWithFallback(TOverlappedDataBuffer* pBuffer, T& rRetryQueue)
		{
			if(pBuffer->HasError())
			{
				if(pBuffer->GetFilePosition() < m_ullErrorPosition)
				{
					// case: new buffer failed at even earlier position in file than the one that failed previously (should also work for numeric_limits::max())
					// - move existing buffers with errors to failed read buffers, add current one to full queue
					m_ullErrorPosition = pBuffer->GetFilePosition();

					BufferCollection newQueue;

					for(TOverlappedDataBuffer* pBuf : m_setBuffers)
					{
						if(pBuf->HasError())
							rRetryQueue.Push(pBuf, true);
						else
						{
							auto pairInsert = newQueue.insert(pBuf);
							if (!pairInsert.second)
								throw TCoreException(eErr_InvalidArgument, L"Tried to insert duplicate buffer into the collection", LOCATION);
						}
					}

					if(newQueue.size() != m_setBuffers.size())
						std::swap(m_setBuffers, newQueue);
				}
				else if(pBuffer->GetFilePosition() > m_ullErrorPosition)
				{
					// case: new buffer failed at position later than the one that failed before - add to failed buffers
					// for retry
					rRetryQueue.Push(pBuffer, true);
					return;
				}
				//else -> case: we've received the same buffer that failed before; add to normal full queue for user to handle that
			}
			else if(m_ullErrorPosition == pBuffer->GetFilePosition())
			{
				// case: adding correctly read buffer that previously failed to read; clear the error flag and add full buffer
				m_ullErrorPosition = NoPosition;
			}

			auto pairInsert = m_setBuffers.insert(pBuffer);
			if (!pairInsert.second)
				throw TCoreException(eErr_InvalidArgument, L"Tried to insert duplicate buffer into the collection", LOCATION);

			UpdateHasBuffers();
		}

		TOverlappedDataBuffer* Pop();
		const TOverlappedDataBuffer* const Peek() const;

		void Clear();

		size_t GetCount() const;
		bool IsEmpty() const;

		HANDLE GetHasBuffersEvent() const;
		void ReleaseBuffers(const TBufferListPtr& spBuffers);

	private:
		bool IsBufferReady() const;
		void UpdateHasBuffers();

	private:
		using BufferCollection = std::set<TOverlappedDataBuffer*, CompareBufferPositions>;

		BufferCollection m_setBuffers;
		TEvent m_eventHasBuffers;
		unsigned long long m_ullErrorPosition = NoPosition;
	};

	using TFailedBufferQueuePtr = std::shared_ptr<TFailedBufferQueue>;
}

#endif
