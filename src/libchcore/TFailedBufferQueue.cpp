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
#include "TFailedBufferQueue.h"
#include "TOverlappedDataBuffer.h"

namespace chcore
{
	TFailedBufferQueue::TFailedBufferQueue() :
		m_eventHasBuffers(true, false)
	{
	}

	TOverlappedDataBuffer* TFailedBufferQueue::Pop()
	{
		if(!IsBufferReady())
			return nullptr;

		TOverlappedDataBuffer* pBuffer = *m_setBuffers.begin();
		m_setBuffers.erase(m_setBuffers.begin());

		if(pBuffer->GetFilePosition() == m_ullErrorPosition)
		{
			// removed the element marked as 'error', calculate the new error position
			m_ullErrorPosition = NoPosition;
			for(TOverlappedDataBuffer* pBuf : m_setBuffers)
			{
				if(pBuf->HasError())
				{
					m_ullErrorPosition = pBuf->GetFilePosition();
					break;
				}
			}
		}

		UpdateHasBuffers();

		return pBuffer;
	}

	const TOverlappedDataBuffer* const TFailedBufferQueue::Peek() const
	{
		if(!m_setBuffers.empty())
			return *m_setBuffers.begin();
		return nullptr;
	}

	bool TFailedBufferQueue::IsBufferReady() const
	{
		return !m_setBuffers.empty();
	}

	void TFailedBufferQueue::Clear()
	{
		m_setBuffers.clear();
		m_eventHasBuffers.ResetEvent();
	}

	size_t TFailedBufferQueue::GetCount() const
	{
		return m_setBuffers.size();
	}

	bool TFailedBufferQueue::IsEmpty() const
	{
		return m_setBuffers.empty();
	}

	HANDLE TFailedBufferQueue::GetHasBuffersEvent() const
	{
		return m_eventHasBuffers.Handle();
	}

	void TFailedBufferQueue::ReleaseBuffers(const TBufferListPtr& spBuffers)
	{
		for(TOverlappedDataBuffer* pBuffer : m_setBuffers)
		{
			spBuffers->Push(pBuffer);
		}
		m_setBuffers.clear();
	}

	void TFailedBufferQueue::UpdateHasBuffers()
	{
		if(IsBufferReady())
			m_eventHasBuffers.SetEvent();
		else
			m_eventHasBuffers.ResetEvent();
	}
}
