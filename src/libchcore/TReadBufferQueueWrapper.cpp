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
#include "TCoreException.h"

namespace chcore
{
	TReadBufferQueueWrapper::TReadBufferQueueWrapper(const TBufferListPtr& spUnorderedQueue, unsigned long long ullNextReadPosition, DWORD dwChunkSize) :
		m_spEmptyBuffers(spUnorderedQueue),
		m_eventHasBuffers(true, false),
		m_ullNextReadPosition(ullNextReadPosition),
		m_dwChunkSize(dwChunkSize)
	{
		if(!spUnorderedQueue)
			throw TCoreException(eErr_InvalidArgument, L"spUnorderedQueue is NULL", LOCATION);
		if(dwChunkSize == 0)
			throw TCoreException(eErr_InvalidArgument, L"dwChunkSize cannot be 0", LOCATION);

		m_emptyBuffersQueueConnector = m_spEmptyBuffers->GetNotifier().connect(boost::bind(&TReadBufferQueueWrapper::UpdateHasBuffers, this));

		UpdateHasBuffers();
	}

	TReadBufferQueueWrapper::~TReadBufferQueueWrapper()
	{
		m_emptyBuffersQueueConnector.disconnect();
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
		
		UpdateHasBuffers();
	}

	void TReadBufferQueueWrapper::PushEmpty(TOverlappedDataBuffer* pBuffer)
	{
		if (!pBuffer)
			throw TCoreException(eErr_InvalidPointer, L"pBuffer", LOCATION);

		m_spEmptyBuffers->Push(pBuffer);
		
		//UpdateHasBuffers();		// already updated using notifier
	}

	TOverlappedDataBuffer* TReadBufferQueueWrapper::Pop()
	{
		if(!IsBufferReady())
			return nullptr;

		// always return retry buffers first
		TOverlappedDataBuffer* pBuffer = m_tRetryBuffers.Pop();
		if(!pBuffer)
		{
			pBuffer = m_spEmptyBuffers->Pop();
			if(pBuffer)
			{
				pBuffer->InitForRead(m_ullNextReadPosition, m_dwChunkSize);
				m_ullNextReadPosition += m_dwChunkSize;
			}
		}

		if(pBuffer)
			UpdateHasBuffers();

		return pBuffer;
	}

	bool TReadBufferQueueWrapper::IsBufferReady() const
	{
		if(IsDataSourceFinished())
			return !m_tRetryBuffers.empty();

		return !m_tRetryBuffers.empty() || !m_spEmptyBuffers->IsEmpty();
	}

	size_t TReadBufferQueueWrapper::GetCount() const
	{
		return m_tRetryBuffers.size();
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

			// release superfluous finished buffers
			auto iterFind = std::find_if(m_tRetryBuffers.begin(), m_tRetryBuffers.end(), [](TOverlappedDataBuffer* pBuffer) { return pBuffer->IsLastPart(); });
			if(iterFind == m_tRetryBuffers.end() || ++iterFind == m_tRetryBuffers.end())
			{
				UpdateHasBuffers();
				return;
			}

			auto iterInvalidParts = std::find_if(iterFind, m_tRetryBuffers.end(), [](TOverlappedDataBuffer* pBuffer) { return !pBuffer->IsLastPart(); });
			if(iterInvalidParts != m_tRetryBuffers.end())
				throw TCoreException(eErr_InvalidArgument, L"Found non-last-parts after last-part", LOCATION);

			for(auto iter = iterFind; iter != m_tRetryBuffers.end(); ++iter)
			{
				m_spEmptyBuffers->Push(*iter);
			}
			m_tRetryBuffers.erase(iterFind, m_tRetryBuffers.end());

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
		m_eventHasBuffers.SetEvent(IsBufferReady());
	}

	void TReadBufferQueueWrapper::ReleaseBuffers()
	{
		m_tRetryBuffers.ReleaseBuffers(m_spEmptyBuffers);
	}
}
