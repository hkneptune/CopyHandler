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
#include "TWriteBufferQueueWrapper.h"
#include "TOverlappedDataBuffer.h"
#include "TCoreException.h"

namespace chcore
{
	TWriteBufferQueueWrapper::TWriteBufferQueueWrapper(const TOrderedBufferQueuePtr& spQueue, size_t stMaxOtfBuffers, TSharedCountPtr<size_t> spOtfBuffersCount) :
		m_spDataQueue(spQueue),
		m_stMaxOtfBuffers(stMaxOtfBuffers),
		m_spOtfBuffersCount(spOtfBuffersCount),
		m_eventHasBuffers(true, false)
	{
		if (!spQueue)
			throw TCoreException(eErr_InvalidArgument, L"spQueue is NULL", LOCATION);
		if (!spOtfBuffersCount)
			throw TCoreException(eErr_InvalidArgument, L"spOtfBuffersCount is NULL", LOCATION);
		if (stMaxOtfBuffers == 0)
			throw TCoreException(eErr_InvalidArgument, L"stMaxOtfBuffers cannot be 0", LOCATION);

		UpdateHasBuffers();

		m_dataQueueConnector = m_spDataQueue->GetSharedCount()->GetNotifier().connect(boost::bind(&TWriteBufferQueueWrapper::UpdateHasBuffers, this));
		m_retryBuffersConnector = m_tRetryBuffers.GetSharedCount()->GetNotifier().connect(boost::bind(&TWriteBufferQueueWrapper::UpdateHasBuffers, this));
	}

	TWriteBufferQueueWrapper::~TWriteBufferQueueWrapper()
	{
		m_dataQueueConnector.disconnect();
		m_retryBuffersConnector.disconnect();
	}

	void TWriteBufferQueueWrapper::Push(TOverlappedDataBuffer* pBuffer)
	{
		if (!pBuffer)
			throw TCoreException(eErr_InvalidPointer, L"pBuffer", LOCATION);

		m_tRetryBuffers.Push(pBuffer);
		UpdateHasBuffers();
	}

	TOverlappedDataBuffer* TWriteBufferQueueWrapper::Pop()
	{
		TOverlappedDataBuffer* pBuffer = InternalPop();
		if(pBuffer)
		{
			pBuffer->InitForWrite();
			UpdateHasBuffers();
		}

		return pBuffer;
	}

	TOverlappedDataBuffer* TWriteBufferQueueWrapper::InternalPop()
	{
		TOverlappedDataBuffer* pBuffer = m_tRetryBuffers.Pop();
		if(!pBuffer)
			pBuffer = m_spDataQueue->Pop();

		return pBuffer;
	}

	size_t TWriteBufferQueueWrapper::GetCount() const
	{
		return m_spDataQueue->GetCount();
	}

	HANDLE TWriteBufferQueueWrapper::GetHasBuffersEvent() const
	{
		return m_eventHasBuffers.Handle();
	}

	void TWriteBufferQueueWrapper::ClearBuffers(const TBufferListPtr& spEmptyBuffers)
	{
		m_spDataQueue->ClearBuffers();
		m_tRetryBuffers.ClearBuffers(spEmptyBuffers);
	}

	bool TWriteBufferQueueWrapper::HasBuffersToProcess() const
	{
		if(m_spOtfBuffersCount->GetValue() >= m_stMaxOtfBuffers)
			return false;

		if(!m_tRetryBuffers.IsEmpty())
			return true;

		return m_spDataQueue->HasPoppableBuffer();
	}

	void TWriteBufferQueueWrapper::UpdateHasBuffers()
	{
		m_eventHasBuffers.SetEvent(HasBuffersToProcess());
	}
}
