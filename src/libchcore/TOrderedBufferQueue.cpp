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
#include "TOrderedBufferQueue.h"
#include "TOverlappedDataBuffer.h"
#include "TCoreException.h"
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/locks.hpp>

namespace chcore
{
	TOrderedBufferQueue::TOrderedBufferQueue(unsigned long long ullExpectedPosition) :
		m_eventHasBuffers(true, false),
		m_eventHasError(true, false),
		m_eventHasReadingFinished(true, false),
		m_ullExpectedBufferPosition(ullExpectedPosition)
	{
	}

	void TOrderedBufferQueue::Push(TOverlappedDataBuffer* pBuffer)
	{
		if(!pBuffer)
			throw TCoreException(eErr_InvalidArgument, L"pBuffer is NULL", LOCATION);
		if(pBuffer->HasError())
			throw TCoreException(eErr_InvalidArgument, L"Cannot push buffer with error", LOCATION);

		boost::unique_lock<boost::shared_mutex> lock(m_mutex);

		auto pairInsert = m_setBuffers.insert(pBuffer);
		if (!pairInsert.second)
			throw TCoreException(eErr_InvalidArgument, L"Tried to insert duplicate buffer into the collection", LOCATION);

		if(pBuffer->IsLastPart())
			m_bDataSourceFinished = true;

		if(m_bDataSourceFinished)
			UpdateReadingFinished();

		if(pBuffer->GetFilePosition() == m_ullErrorPosition)
		{
			if(m_pFirstErrorBuffer != nullptr)
				throw TCoreException(eErr_InternalProblem, L"Buffer with error was not retrieved prior to adding same-by-position buffer without error", LOCATION);
			m_ullErrorPosition = NoPosition;
			UpdateHasErrors();
		}

		UpdateHasBuffers();
	}

	TOverlappedDataBuffer* TOrderedBufferQueue::Pop()
	{
		boost::unique_lock<boost::shared_mutex> lock(m_mutex);

		if(!InternalHasPoppableBuffer())
			return nullptr;

		TOverlappedDataBuffer* pBuffer = *m_setBuffers.begin();
		m_setBuffers.erase(m_setBuffers.begin());

		m_ullExpectedBufferPosition += pBuffer->GetRequestedDataSize();

		UpdateHasBuffers();

		return pBuffer;
	}

	TOverlappedDataBuffer* TOrderedBufferQueue::PopError()
	{
		boost::unique_lock<boost::shared_mutex> lock(m_mutex);

		if(!m_pFirstErrorBuffer)
			return nullptr;

		TOverlappedDataBuffer* pBuffer = m_pFirstErrorBuffer;
		m_pFirstErrorBuffer = nullptr;
		UpdateHasErrors();

		return pBuffer;
	}

	const TOverlappedDataBuffer* const TOrderedBufferQueue::Peek() const
	{
		boost::shared_lock<boost::shared_mutex> lock(m_mutex);

		if(!m_setBuffers.empty())
			return *m_setBuffers.begin();
		return nullptr;
	}

	size_t TOrderedBufferQueue::GetCount() const
	{
		boost::shared_lock<boost::shared_mutex> lock(m_mutex);
		return m_setBuffers.size() + (m_pFirstErrorBuffer ? 1 : 0);
	}

	bool TOrderedBufferQueue::IsEmpty() const
	{
		boost::shared_lock<boost::shared_mutex> lock(m_mutex);
		return m_setBuffers.empty();
	}

	bool TOrderedBufferQueue::HasPoppableBuffer() const
	{
		boost::shared_lock<boost::shared_mutex> lock(m_mutex);

		return InternalHasPoppableBuffer();
	}

	bool TOrderedBufferQueue::InternalHasPoppableBuffer() const
	{
		if(m_setBuffers.empty())
			return false;

		TOverlappedDataBuffer* pBuffer = *m_setBuffers.begin();
		return pBuffer->GetFilePosition() == m_ullExpectedBufferPosition;
	}

	HANDLE TOrderedBufferQueue::GetHasBuffersEvent() const
	{
		return m_eventHasBuffers.Handle();
	}

	HANDLE TOrderedBufferQueue::GetHasErrorEvent() const
	{
		return m_eventHasError.Handle();
	}

	HANDLE TOrderedBufferQueue::GetHasReadingFinished() const
	{
		return m_eventHasReadingFinished.Handle();
	}

	void TOrderedBufferQueue::ReleaseBuffers(const TBufferListPtr& spBuffers)
	{
		if(!spBuffers)
			throw TCoreException(eErr_InvalidArgument, L"spBuffers is NULL", LOCATION);

		boost::unique_lock<boost::shared_mutex> lock(m_mutex);

		for(TOverlappedDataBuffer* pBuffer : m_setBuffers)
		{
			spBuffers->Push(pBuffer);
		}
		m_setBuffers.clear();

		if(m_pFirstErrorBuffer)
		{
			spBuffers->Push(m_pFirstErrorBuffer);
			m_pFirstErrorBuffer = nullptr;
			m_ullErrorPosition = NoPosition;
		}

		UpdateHasBuffers();
		UpdateHasErrors();
	}

	void TOrderedBufferQueue::UpdateHasBuffers()
	{
		if(InternalHasPoppableBuffer())
		{
			m_eventHasBuffers.SetEvent();
			m_notifier(true);
		}
		else
		{
			m_eventHasBuffers.ResetEvent();
			m_notifier(false);
		}
	}

	void TOrderedBufferQueue::UpdateHasErrors()
	{
		m_eventHasError.SetEvent(m_pFirstErrorBuffer != nullptr);
	}

	void TOrderedBufferQueue::UpdateReadingFinished()
	{
		bool bFullSequence = true;
		unsigned long long ullExpected = m_ullExpectedBufferPosition;
		for(TOverlappedDataBuffer* pBuffer : m_setBuffers)
		{
			if(pBuffer->GetFilePosition() != ullExpected)
			{
				bFullSequence = false;
				break;
			}

			ullExpected += pBuffer->GetRequestedDataSize();
		}

		m_eventHasReadingFinished.SetEvent(bFullSequence);
	}

	boost::signals2::signal<void(bool)>& TOrderedBufferQueue::GetNotifier()
	{
		return m_notifier;
	}

	void TOrderedBufferQueue::UpdateProcessingRange(unsigned long long ullNewPosition)
	{
		boost::unique_lock<boost::shared_mutex> lock(m_mutex);

		if(!m_setBuffers.empty())
			throw TCoreException(eErr_InvalidData, L"Cannot update processing range when processing already started", LOCATION);

		m_ullExpectedBufferPosition = ullNewPosition;
	}
}
