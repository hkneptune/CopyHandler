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
		m_spUnorderedQueue(spUnorderedQueue),
		m_eventHasBuffers(true, false),
		m_ullNextReadPosition(ullNextReadPosition),
		m_dwChunkSize(dwChunkSize)
	{
		if(!spUnorderedQueue)
			throw TCoreException(eErr_InvalidArgument, L"spUnorderedQueue is NULL", LOCATION);
		if(dwChunkSize == 0)
			throw TCoreException(eErr_InvalidArgument, L"dwChunkSize cannot be 0", LOCATION);

		m_emptyBuffersQueueConnector = m_spUnorderedQueue->GetNotifier().connect(boost::bind(&TReadBufferQueueWrapper::UpdateHasBuffers, this, _1));

		UpdateHasBuffers();
	}

	TReadBufferQueueWrapper::~TReadBufferQueueWrapper()
	{
		m_emptyBuffersQueueConnector.disconnect();
	}

	void TReadBufferQueueWrapper::Push(TOverlappedDataBuffer* pBuffer, bool bKeepPosition)
	{
		if (!pBuffer)
			throw TCoreException(eErr_InvalidPointer, L"pBuffer", LOCATION);

		if(!bKeepPosition)
			m_spUnorderedQueue->Push(pBuffer);
		else if(IsDataSourceFinished())
		{
			if(!pBuffer->IsLastPart())
			{
				if(pBuffer->GetFilePosition() > m_ullDataSourceFinishedPos)
					throw TCoreException(eErr_InvalidArgument, L"Adding regular buffer after the queue was marked as finished", LOCATION);

				m_tClaimedQueue.Push(pBuffer);
			}
			else
				m_spUnorderedQueue->Push(pBuffer);
		}
		else
			m_tClaimedQueue.Push(pBuffer);
		
		UpdateHasBuffers();
	}

	TOverlappedDataBuffer* TReadBufferQueueWrapper::Pop()
	{
		if(!IsBufferReady())
			return nullptr;

		// always return retry buffers first
		TOverlappedDataBuffer* pBuffer = m_tClaimedQueue.Pop();
		if(!pBuffer)
		{
			pBuffer = m_spUnorderedQueue->Pop();
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
			return !m_tClaimedQueue.empty();

		return !m_tClaimedQueue.empty() || !m_spUnorderedQueue->IsEmpty();
	}

	size_t TReadBufferQueueWrapper::GetCount() const
	{
		return m_tClaimedQueue.size();
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
			auto iterFind = std::find_if(m_tClaimedQueue.begin(), m_tClaimedQueue.end(), [](TOverlappedDataBuffer* pBuffer) { return pBuffer->IsLastPart(); });
			if(iterFind == m_tClaimedQueue.end() || ++iterFind == m_tClaimedQueue.end())
			{
				UpdateHasBuffers();
				return;
			}

			auto iterInvalidParts = std::find_if(iterFind, m_tClaimedQueue.end(), [](TOverlappedDataBuffer* pBuffer) { return !pBuffer->IsLastPart(); });
			if(iterInvalidParts != m_tClaimedQueue.end())
				throw TCoreException(eErr_InvalidArgument, L"Found non-last-parts after last-part", LOCATION);

			for(auto iter = iterFind; iter != m_tClaimedQueue.end(); ++iter)
			{
				m_spUnorderedQueue->Push(*iter);
			}
			m_tClaimedQueue.erase(iterFind, m_tClaimedQueue.end());

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

	void TReadBufferQueueWrapper::UpdateHasBuffers(bool /*bAdded*/)
	{
		UpdateHasBuffers();
	}

	void TReadBufferQueueWrapper::ReleaseBuffers(const TBufferListPtr& spBuffers)
	{
		m_tClaimedQueue.ReleaseBuffers(spBuffers);
	}
}
