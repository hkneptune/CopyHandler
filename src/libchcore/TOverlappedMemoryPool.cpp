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
#include "TOverlappedMemoryPool.h"
#include "TOverlappedDataBuffer.h"
#include "TCoreException.h"
#include "ErrorCodes.h"
#include <array>

#define STATUS_END_OF_FILE 0xc0000011

namespace chcore
{
	TOverlappedMemoryPool::TOverlappedMemoryPool() :
		m_eventHasBuffers(true, false),
		m_eventAllBuffersAccountedFor(true, true)
	{
	}

	TOverlappedMemoryPool::TOverlappedMemoryPool(size_t stCount, size_t stBufferSize) :
		TOverlappedMemoryPool()
	{
		ReinitializeBuffers(stCount, stBufferSize);
	}

	TOverlappedMemoryPool::~TOverlappedMemoryPool()
	{
	}

	TOverlappedDataBuffer* TOverlappedMemoryPool::GetBuffer()
	{
		if (!m_dequeBuffers.empty())
		{
			TOverlappedDataBuffer* pBuffer = m_dequeBuffers.front();
			m_dequeBuffers.pop_front();

			UpdateHasBuffers();
			UpdateAllBuffersAccountedFor();

			return pBuffer;
		}

		return nullptr;
	}

	bool TOverlappedMemoryPool::AreAllBuffersAccountedFor() const
	{
		return m_dequeBuffers.size() == m_listAllBuffers.size();
	}

	void TOverlappedMemoryPool::AddBuffer(TOverlappedDataBuffer* pBuffer)
	{
		if (!pBuffer)
			throw TCoreException(eErr_InvalidPointer, L"pBuffer", LOCATION);

		m_dequeBuffers.push_back(pBuffer);
		UpdateHasBuffers();
		UpdateAllBuffersAccountedFor();
	}

	void TOverlappedMemoryPool::UpdateAllBuffersAccountedFor()
	{
		if (AreAllBuffersAccountedFor())
			m_eventAllBuffersAccountedFor.SetEvent();
		else
			m_eventAllBuffersAccountedFor.ResetEvent();
	}

	void TOverlappedMemoryPool::UpdateHasBuffers()
	{
		if(!m_dequeBuffers.empty())
			m_eventHasBuffers.SetEvent();
		else
			m_eventHasBuffers.ResetEvent();
	}

	void TOverlappedMemoryPool::ReinitializeBuffers(size_t stCount, size_t stBufferSize)
	{
		// sanity check - if any of the buffers are still in use, we can't change the sizes
		if (m_listAllBuffers.size() != m_dequeBuffers.size())
			throw TCoreException(eErr_InternalProblem, L"Some buffers are still in use", LOCATION);
		if (stBufferSize == 0)
			throw TCoreException(eErr_InvalidArgument, L"stBufferSize", LOCATION);

		if (stBufferSize != GetSingleBufferSize())
		{
			// buffer sizes increased - clear current buffers and proceed with creating new ones
			m_listAllBuffers.clear();
			m_dequeBuffers.clear();
		}
		else if (stCount == m_listAllBuffers.size())
			return;		// nothing really changed
		else if (stCount > m_listAllBuffers.size())
			stCount -= m_listAllBuffers.size();	// allocate only the missing buffers
		else if (stCount < m_listAllBuffers.size())
		{
			// there are too many buffers - reduce
			m_dequeBuffers.clear();

			size_t stCountToRemove = m_listAllBuffers.size() - stCount;

			m_listAllBuffers.erase(m_listAllBuffers.begin(), m_listAllBuffers.begin() + stCountToRemove);
			for (const auto& upElement : m_listAllBuffers)
			{
				m_dequeBuffers.push_back(upElement.get());
			}

			UpdateHasBuffers();
			UpdateAllBuffersAccountedFor();
			return;
		}

		// allocate buffers
		while (stCount--)
		{
			auto upBuffer = std::make_unique<TOverlappedDataBuffer>(stBufferSize, nullptr);
			m_dequeBuffers.push_back(upBuffer.get());
			m_listAllBuffers.push_back(std::move(upBuffer));
		}

		UpdateHasBuffers();
		UpdateAllBuffersAccountedFor();
	}

	size_t TOverlappedMemoryPool::GetTotalBufferCount() const
	{
		return m_listAllBuffers.size();
	}

	size_t TOverlappedMemoryPool::GetAvailableBufferCount() const
	{
		return m_dequeBuffers.size();
	}
	
	size_t TOverlappedMemoryPool::GetSingleBufferSize() const
	{
		if (m_listAllBuffers.empty())
			return 0;

		return (*m_listAllBuffers.begin())->GetBufferSize();
	}

	void TOverlappedMemoryPool::WaitForMissingBuffers(HANDLE hKillEvent) const
	{
		enum { eKillThread = 0, eAllBuffersReturned, eHandleCount };
		std::array<HANDLE, eHandleCount> arrHandles = { hKillEvent, m_eventAllBuffersAccountedFor.Handle() };

		bool bExit = false;
		while (!bExit)
		{
			DWORD dwResult = WaitForMultipleObjectsEx(eHandleCount, arrHandles.data(), false, INFINITE, true);
			switch (dwResult)
			{
			case STATUS_USER_APC:
				break;

			case WAIT_OBJECT_0 + eAllBuffersReturned:
				bExit = true;
				break;

			case WAIT_OBJECT_0 + eKillThread:
				bExit = true;
				break;
			}
		}
	}
}
