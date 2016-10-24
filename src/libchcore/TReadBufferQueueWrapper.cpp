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
		m_ullNextReadPosition(ullNextReadPosition),
		m_dwChunkSize(dwChunkSize),
		m_eventHasBuffers(true, false)
	{
		UpdateHasBuffers();
	}

	void TReadBufferQueueWrapper::Push(TOverlappedDataBuffer* pBuffer, bool bKeepPosition)
	{
		if (!pBuffer)
			throw TCoreException(eErr_InvalidPointer, L"pBuffer", LOCATION);

		if(!bKeepPosition)
		{
			if(IsDataSourceFinished())
				m_spUnorderedQueue->Push(pBuffer);
			else
			{
				pBuffer->InitForRead(m_ullNextReadPosition, m_dwChunkSize);
				m_ullNextReadPosition += m_dwChunkSize;
				m_tClaimedQueue.Push(pBuffer);
			}
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
		{
			if(m_tClaimedQueue.IsEmpty())
				return false;

			const TOverlappedDataBuffer* const pFirstBuffer = m_tClaimedQueue.Peek();
			if(pFirstBuffer->GetFilePosition() <= m_ullDataSourceFinishedPos)
				return true;

			return false;
		}
		else
			return !m_tClaimedQueue.IsEmpty() || !m_spUnorderedQueue->IsEmpty();
	}

	void TReadBufferQueueWrapper::Clear()
	{
		m_spUnorderedQueue->Clear();
		m_ullNextReadPosition = 0;
		m_dwChunkSize = 0;
		m_ullDataSourceFinishedPos = NoPosition;
		m_eventHasBuffers.ResetEvent();
	}

	size_t TReadBufferQueueWrapper::GetCount() const
	{
		return m_tClaimedQueue.GetCount();
	}

	bool TReadBufferQueueWrapper::IsEmpty() const
	{
		return m_spUnorderedQueue->IsEmpty();
	}

	void TReadBufferQueueWrapper::SetDataSourceFinished(TOverlappedDataBuffer* pBuffer)
	{
		if(pBuffer->IsLastPart())
		{
			if(pBuffer->GetFilePosition() < m_ullDataSourceFinishedPos)
			{
				m_ullDataSourceFinishedPos = pBuffer->GetFilePosition();
				// #todo: release excessive claimed buffers
				UpdateHasBuffers();
			}
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

	void TReadBufferQueueWrapper::ReleaseBuffers(const TBufferListPtr& spBuffers)
	{
		m_tClaimedQueue.ReleaseBuffers(spBuffers);
	}
}
