// ============================================================================
//  Copyright (C) 2001-2015 by Jozef Starosczyk
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
#include "TOverlappedDataBufferQueue.h"
#include "TOverlappedDataBuffer.h"
#include "TCoreException.h"
#include "ErrorCodes.h"

BEGIN_CHCORE_NAMESPACE

bool CompareBufferPositions::operator()(const TOverlappedDataBuffer* pBufferA, const TOverlappedDataBuffer* pBufferB)
{
	return pBufferA->GetBufferOrder() < pBufferB->GetBufferOrder();
}

TOverlappedDataBufferQueue::TOverlappedDataBufferQueue() :
	m_eventReadPossible(true, true),
	m_eventWritePossible(true, false),
	m_eventWriteFinished(true, false),
	m_stBufferSize(0),
	m_ullNextExpectedWritePosition(0),
	m_bDataSourceFinished(false),
	m_ullNextReadBufferOrder(0),
	m_ullNextWriteBufferOrder(0),
	m_ullNextFinishedBufferOrder(0)
{
}

TOverlappedDataBufferQueue::TOverlappedDataBufferQueue(size_t stCount, size_t stBufferSize) :
	m_eventReadPossible(true, true),
	m_eventWritePossible(true, false),
	m_eventWriteFinished(true, false),
	m_stBufferSize(0),
	m_ullNextExpectedWritePosition(0),
	m_bDataSourceFinished(false),
	m_ullNextReadBufferOrder(0),
	m_ullNextWriteBufferOrder(0),
	m_ullNextFinishedBufferOrder(0)
{
	ReinitializeBuffers(stCount, stBufferSize);
}

TOverlappedDataBufferQueue::~TOverlappedDataBufferQueue()
{
}

TOverlappedDataBuffer* TOverlappedDataBufferQueue::GetEmptyBuffer()
{
	if (!m_listEmptyBuffers.empty())
	{
		TOverlappedDataBuffer* pBuffer = m_listEmptyBuffers.front();
		m_listEmptyBuffers.pop_front();

		pBuffer->SetBufferOrder(m_ullNextReadBufferOrder++);

		UpdateReadPossibleEvent();

		return pBuffer;
	}

	return nullptr;
}

void TOverlappedDataBufferQueue::AddEmptyBuffer(TOverlappedDataBuffer* pBuffer)
{
	if (!pBuffer)
		THROW_CORE_EXCEPTION(eErr_InvalidPointer);

	m_listEmptyBuffers.push_back(pBuffer);
	UpdateReadPossibleEvent();
}

void TOverlappedDataBufferQueue::UpdateReadPossibleEvent()
{
	if (!m_listEmptyBuffers.empty() && !m_bDataSourceFinished)
		m_eventReadPossible.SetEvent();
	else
		m_eventReadPossible.ResetEvent();
}

TOverlappedDataBuffer* TOverlappedDataBufferQueue::GetFullBuffer()
{
	if (!m_setFullBuffers.empty())
	{
		TOverlappedDataBuffer* pBuffer = *m_setFullBuffers.begin();
		if (pBuffer->GetBufferOrder() != m_ullNextWriteBufferOrder)
			return nullptr;

		m_setFullBuffers.erase(m_setFullBuffers.begin());
		m_ullNextExpectedWritePosition += pBuffer->GetBytesTransferred();

		++m_ullNextWriteBufferOrder;

		UpdateWritePossibleEvent();

		return pBuffer;
	}

	return nullptr;
}

void TOverlappedDataBufferQueue::AddFullBuffer(TOverlappedDataBuffer* pBuffer)
{
	if (!pBuffer)
		THROW_CORE_EXCEPTION(eErr_InvalidPointer);

	// special case - if we already know that there was an end of file and the new packet arrived with the same information and no data
	// then it can be treated as an empty buffer
	if (pBuffer->IsLastPart() && m_bDataSourceFinished && pBuffer->GetBytesTransferred() == 0)
	{
		// not using AddEmptyBuffer() as we there is no need for changing the signals (they are already set correctly)
		m_listEmptyBuffers.push_back(pBuffer);
		return;
	}

	m_setFullBuffers.insert(pBuffer);

	if(pBuffer->IsLastPart())
		m_bDataSourceFinished = true;

	UpdateWritePossibleEvent();
}

void TOverlappedDataBufferQueue::UpdateWritePossibleEvent()
{
	if (m_setFullBuffers.empty())
		m_eventWritePossible.ResetEvent();
	else
	{
		TOverlappedDataBuffer* pFirstBuffer = *m_setFullBuffers.begin();
		if (pFirstBuffer->GetBufferOrder() == m_ullNextWriteBufferOrder)
			m_eventWritePossible.SetEvent();
		else
			m_eventWritePossible.ResetEvent();
	}
}

TOverlappedDataBuffer* TOverlappedDataBufferQueue::GetFinishedBuffer()
{
	if(!m_setFinishedBuffers.empty())
	{
		TOverlappedDataBuffer* pBuffer = *m_setFinishedBuffers.begin();
		if(pBuffer->GetBufferOrder() != m_ullNextFinishedBufferOrder)
			return nullptr;

		m_setFinishedBuffers.erase(m_setFinishedBuffers.begin());

		++m_ullNextFinishedBufferOrder;

		UpdateWriteFinishedEvent();

		return pBuffer;
	}

	return nullptr;
}

void TOverlappedDataBufferQueue::AddFinishedBuffer(TOverlappedDataBuffer* pBuffer)
{
	if (!pBuffer)
		THROW_CORE_EXCEPTION(eErr_InvalidPointer);

	m_setFinishedBuffers.insert(pBuffer);

	UpdateWriteFinishedEvent();
}

void TOverlappedDataBufferQueue::UpdateWriteFinishedEvent()
{
	if (m_setFinishedBuffers.empty())
		m_eventWriteFinished.ResetEvent();
	else
	{
		TOverlappedDataBuffer* pFirstBuffer = *m_setFinishedBuffers.begin();
		if (pFirstBuffer->GetBufferOrder() == m_ullNextFinishedBufferOrder)
			m_eventWriteFinished.SetEvent();
		else
			m_eventWriteFinished.ResetEvent();
	}
}

void TOverlappedDataBufferQueue::ReinitializeBuffers(size_t stCount, size_t stBufferSize)
{
	// sanity check - if any of the buffers are still in use, we can't change the sizes
	if (m_listAllBuffers.size() != m_listEmptyBuffers.size())
		THROW_CORE_EXCEPTION(eErr_InternalProblem);

	if (stBufferSize > m_stBufferSize)
	{
		// buffer sizes increased - clear current buffers and proceed with creating new ones
		m_listAllBuffers.clear();
		m_listEmptyBuffers.clear();
		m_setFullBuffers.clear();
	}
	else if (stCount == m_listAllBuffers.size())
		return;		// nothing really changed
	else if (stCount < m_listAllBuffers.size())
		stCount = m_listAllBuffers.size() - stCount;	// allocate only the missing buffers
	else if (stCount > m_listAllBuffers.size())
	{
		// there are too many buffers - reduce
		m_listEmptyBuffers.clear();
		m_setFullBuffers.clear();

		size_t stCountToRemove = stCount - m_listAllBuffers.size();

		m_listAllBuffers.erase(m_listAllBuffers.begin(), m_listAllBuffers.begin() + stCountToRemove);
		for (const auto& upElement : m_listAllBuffers)
		{
			m_listEmptyBuffers.push_back(upElement.get());
		}

		return;
	}

	// allocate buffers
	while (stCount--)
	{
		auto upBuffer = std::make_unique<TOverlappedDataBuffer>(stBufferSize, this);
		m_listEmptyBuffers.push_back(upBuffer.get());
		m_listAllBuffers.push_back(std::move(upBuffer));
	}

	m_stBufferSize = stCount;
}

void TOverlappedDataBufferQueue::DataSourceChanged()
{
	CleanupBuffers();

	if (m_listAllBuffers.size() != m_listEmptyBuffers.size())
		THROW_CORE_EXCEPTION(eErr_InternalProblem);

	m_bDataSourceFinished = false;
	m_ullNextReadBufferOrder = 0;
	m_ullNextWriteBufferOrder = 0;
	m_ullNextFinishedBufferOrder = 0;

	m_eventReadPossible.SetEvent();
	m_eventWritePossible.ResetEvent();
	m_eventWriteFinished.ResetEvent();
}

void TOverlappedDataBufferQueue::CleanupBuffers()
{
	// function sanitizes the buffer locations (empty/full/finished) - i.e. when there is full buffer that have no data, is marked eof and we are in the eof state
	// then this buffer is really the empty one
	if (m_bDataSourceFinished && !m_setFullBuffers.empty())
	{
		auto iterCurrent = m_setFullBuffers.begin();
		while (iterCurrent != m_setFullBuffers.end())
		{
			if ((*iterCurrent)->IsLastPart())
			{
				m_listEmptyBuffers.push_back(*iterCurrent);
				iterCurrent = m_setFullBuffers.erase(iterCurrent);
			}
			else
				++iterCurrent;
		}
	}
}

END_CHCORE_NAMESPACE
