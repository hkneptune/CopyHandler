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
#include "TReadBufferQueueWrapper.h"
#include "TOverlappedDataBuffer.h"

using namespace chcore;

namespace chengine
{
	TReadBufferQueueWrapper::TReadBufferQueueWrapper(const TBufferListPtr& spEmptyBuffers,
		unsigned long long ullNextReadPosition, DWORD dwChunkSize,
		size_t stMaxOtfBuffers, size_t stMaxReadAheadBuffers,
		TSharedCountPtr<size_t> spOtfBuffersCount, TSharedCountMTPtr<size_t> spCurrentReadAheadBuffers) :
		m_spEmptyBuffers(spEmptyBuffers),
		m_ullNextReadPosition(ullNextReadPosition),
		m_dwChunkSize(dwChunkSize),
		m_stMaxOtfBuffers(stMaxOtfBuffers),
		m_stMaxReadAheadBuffers(stMaxReadAheadBuffers),
		m_spOtfBuffersCount(spOtfBuffersCount),
		m_spCurrentReadAheadBuffers(spCurrentReadAheadBuffers),
		m_eventHasBuffers(false, true)
	{
		if(!spEmptyBuffers)
			throw TCoreException(eErr_InvalidArgument, L"spEmptyBuffers is NULL", LOCATION);
		if(!spOtfBuffersCount)
			throw TCoreException(eErr_InvalidArgument, L"spOtfBuffersCount is NULL", LOCATION);
		if(!spCurrentReadAheadBuffers)
			throw TCoreException(eErr_InvalidArgument, L"spCurrentReadAheadBuffers is NULL", LOCATION);
		if(dwChunkSize == 0)
			throw TCoreException(eErr_InvalidArgument, L"dwChunkSize cannot be 0", LOCATION);
		if(stMaxOtfBuffers == 0)
			throw TCoreException(eErr_InvalidArgument, L"stMaxOtfBuffers cannot be 0", LOCATION);
		if(stMaxReadAheadBuffers == 0)
			throw TCoreException(eErr_InvalidArgument, L"stMaxReadAheadBuffers cannot be 0", LOCATION);

		m_emptyBuffersQueueConnector = m_spEmptyBuffers->GetSharedCount()->GetNotifier().connect(boost::bind(&TReadBufferQueueWrapper::UpdateHasBuffers, this));
		m_currentReadAheadConnector = m_spCurrentReadAheadBuffers->GetNotifier().connect(boost::bind(&TReadBufferQueueWrapper::UpdateHasBuffers, this));
		m_retryBuffersConnector = m_tRetryBuffers.GetSharedCount()->GetNotifier().connect(boost::bind(&TReadBufferQueueWrapper::UpdateHasBuffers, this));
		m_otfBuffersConnector = m_spOtfBuffersCount->GetNotifier().connect(boost::bind(&TReadBufferQueueWrapper::UpdateHasBuffers, this));
	}

	TReadBufferQueueWrapper::~TReadBufferQueueWrapper()
	{
		m_emptyBuffersQueueConnector.disconnect();
		m_currentReadAheadConnector.disconnect();
		m_retryBuffersConnector.disconnect();
		m_otfBuffersConnector.disconnect();
	}

	void TReadBufferQueueWrapper::Push(TOverlappedDataBuffer* pBuffer)
	{
		if (!pBuffer)
			throw TCoreException(eErr_InvalidPointer, L"pBuffer", LOCATION);

		if(IsDataSourceFinished())
		{
			if(!pBuffer->IsLastPart())
			{
				if(pBuffer->GetFilePosition() > m_ullDataSourceFinishedPos)
					throw TCoreException(eErr_InvalidArgument, L"Adding regular buffer after the queue was marked as finished", LOCATION);

				m_tRetryBuffers.Push(pBuffer);
			}
			else
				m_spEmptyBuffers->Push(pBuffer);
		}
		else
			m_tRetryBuffers.Push(pBuffer);
	}

	void TReadBufferQueueWrapper::PushEmpty(TOverlappedDataBuffer* pBuffer)
	{
		if (!pBuffer)
			throw TCoreException(eErr_InvalidPointer, L"pBuffer", LOCATION);

		m_spEmptyBuffers->Push(pBuffer);
	}

	TOverlappedDataBuffer* TReadBufferQueueWrapper::Pop()
	{
		if(m_spOtfBuffersCount->GetValue() >= m_stMaxOtfBuffers)
			return nullptr;

		if(m_spCurrentReadAheadBuffers->GetValue() >= m_stMaxReadAheadBuffers)
			return nullptr;

		TOverlappedDataBuffer* pBuffer = m_tRetryBuffers.Pop();
		if(!pBuffer && !IsDataSourceFinished())
		{
			pBuffer = m_spEmptyBuffers->Pop();
			if(pBuffer)
			{
				pBuffer->InitForRead(m_ullNextReadPosition, m_dwChunkSize);
				m_ullNextReadPosition += m_dwChunkSize;
			}
		}

		if(pBuffer)
			m_eventHasBuffers.SetEvent();

		return pBuffer;
	}

	void TReadBufferQueueWrapper::SetDataSourceFinished(TOverlappedDataBuffer* pBuffer)
	{
		if(!pBuffer)
			throw TCoreException(eErr_InvalidArgument, L"pBuffer is NULL", LOCATION);
		if(!pBuffer->IsLastPart())
			throw TCoreException(eErr_InvalidArgument, L"Trying to set the end of data using unfinished buffer", LOCATION);

		if(pBuffer->GetFilePosition() < m_ullDataSourceFinishedPos)
		{
			m_ullDataSourceFinishedPos = pBuffer->GetFilePosition();
			UpdateHasBuffers();
		}
	}

	bool TReadBufferQueueWrapper::IsDataSourceFinished() const
	{
		return m_ullDataSourceFinishedPos != NoPosition;
	}

	HANDLE TReadBufferQueueWrapper::GetHasBuffersEvent() const
	{
		return m_eventHasBuffers.Handle();
	}

	void TReadBufferQueueWrapper::UpdateHasBuffers()
	{
		m_eventHasBuffers.SetEvent();
	}

	void TReadBufferQueueWrapper::ClearBuffers()
	{
		m_tRetryBuffers.ClearBuffers(m_spEmptyBuffers);
	}

	void TReadBufferQueueWrapper::UpdateProcessingRange(unsigned long long ullNewPosition)
	{
		if(!m_tRetryBuffers.IsEmpty())
			throw TCoreException(eErr_InvalidData, L"Cannot update processing range when processing already started", LOCATION);
		m_ullNextReadPosition = ullNewPosition;
	}
}
