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

namespace chcore
{
	TOrderedBufferQueue::TOrderedBufferQueue() :
		m_eventHasBuffers(true, false)
	{
	}

	TOrderedBufferQueue::TOrderedBufferQueue(unsigned long long ullExpectedPosition) :
		m_eventHasBuffers(true, false),
		m_ullExpectedBufferPosition(ullExpectedPosition)
	{
	}

	void TOrderedBufferQueue::Push(TOverlappedDataBuffer* pBuffer)
	{
		auto pairInsert = m_setBuffers.insert(pBuffer);
		if (!pairInsert.second)
			throw TCoreException(eErr_InvalidArgument, L"Tried to insert duplicate buffer into the collection", LOCATION);

		UpdateHasBuffers();
	}

	TOverlappedDataBuffer* TOrderedBufferQueue::Pop()
	{
		if(!IsBufferReady())
			return nullptr;

		TOverlappedDataBuffer* pBuffer = *m_setBuffers.begin();
		m_setBuffers.erase(m_setBuffers.begin());

		if(!pBuffer->HasError() && m_ullExpectedBufferPosition != NoPosition)
			m_ullExpectedBufferPosition += pBuffer->GetRequestedDataSize();

		UpdateHasBuffers();

		return pBuffer;
	}

	const TOverlappedDataBuffer* const TOrderedBufferQueue::Peek() const
	{
		if(!m_setBuffers.empty())
			return *m_setBuffers.begin();
		return nullptr;
	}

	bool TOrderedBufferQueue::IsBufferReady() const
	{
		return (!m_setBuffers.empty() && (m_ullExpectedBufferPosition == NoPosition || (*m_setBuffers.begin())->GetFilePosition() == m_ullExpectedBufferPosition));
	}

	void TOrderedBufferQueue::Clear()
	{
		m_setBuffers.clear();
		m_ullExpectedBufferPosition = NoPosition;
		m_eventHasBuffers.ResetEvent();
	}

	size_t TOrderedBufferQueue::GetCount() const
	{
		return m_setBuffers.size();
	}

	bool TOrderedBufferQueue::IsEmpty() const
	{
		return m_setBuffers.empty();
	}

	HANDLE TOrderedBufferQueue::GetHasBuffersEvent() const
	{
		return m_eventHasBuffers.Handle();
	}

	void TOrderedBufferQueue::ReleaseBuffers(const TBufferListPtr& spBuffers)
	{
		for(TOverlappedDataBuffer* pBuffer : m_setBuffers)
		{
			spBuffers->Push(pBuffer);
		}
		m_setBuffers.clear();
	}

	void TOrderedBufferQueue::UpdateHasBuffers()
	{
		if(!m_setBuffers.empty() && (m_ullExpectedBufferPosition == NoPosition || Peek()->GetFilePosition() == m_ullExpectedBufferPosition))
			m_eventHasBuffers.SetEvent();
		else
			m_eventHasBuffers.ResetEvent();
	}
}
