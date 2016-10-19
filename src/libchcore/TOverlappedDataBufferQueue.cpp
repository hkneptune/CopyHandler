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
#include <array>

#define STATUS_END_OF_FILE 0xc0000011

namespace chcore
{
	///////////////////////////////////////////////////////////////////////////////////
	// class TOverlappedDataBuffer
	VOID CALLBACK OverlappedReadCompleted(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped)
	{
		_ASSERTE(dwNumberOfBytesTransfered == lpOverlapped->InternalHigh);

		TOverlappedDataBuffer* pBuffer = (TOverlappedDataBuffer*)lpOverlapped;
		TOverlappedDataBufferQueue* pQueue = (TOverlappedDataBufferQueue*)pBuffer->GetParam();

		// determine if this is the last packet
		bool bEof = (dwErrorCode == ERROR_HANDLE_EOF ||
			pBuffer->GetStatusCode() == STATUS_END_OF_FILE ||
			(dwErrorCode == ERROR_SUCCESS && dwNumberOfBytesTransfered != pBuffer->GetRequestedDataSize()));

		// reset status code and error code if they pointed out to EOF
		if(pBuffer->GetStatusCode() == STATUS_END_OF_FILE)
			pBuffer->SetStatusCode(0);

		pBuffer->SetErrorCode(dwErrorCode == ERROR_HANDLE_EOF ? ERROR_SUCCESS : dwErrorCode);

		pBuffer->SetRealDataSize(dwNumberOfBytesTransfered);
		pBuffer->SetLastPart(bEof);

		pQueue->AddFullBuffer(pBuffer);
	}

	VOID CALLBACK OverlappedWriteCompleted(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped)
	{
		_ASSERTE(dwNumberOfBytesTransfered == lpOverlapped->InternalHigh);

		TOverlappedDataBuffer* pBuffer = (TOverlappedDataBuffer*)lpOverlapped;
		TOverlappedDataBufferQueue* pQueue = (TOverlappedDataBufferQueue*)pBuffer->GetParam();

		pBuffer->SetErrorCode(dwErrorCode);

		pQueue->AddFinishedBuffer(pBuffer);
	}

	bool CompareBufferPositions::operator()(const TOverlappedDataBuffer* pBufferA, const TOverlappedDataBuffer* pBufferB)
	{
		return pBufferA->GetBufferOrder() < pBufferB->GetBufferOrder();
	}

	TOverlappedDataBufferQueue::TOverlappedDataBufferQueue(const logger::TLogFileDataPtr& spLogFileData) :
		m_spLog(logger::MakeLogger(spLogFileData, L"DataBuffer")),
		m_eventReadPossible(true, false),
		m_eventWritePossible(true, false),
		m_eventWriteFinished(true, false),
		m_eventAllBuffersAccountedFor(true, true),
		m_bDataSourceFinished(false),
		m_bDataWritingFinished(false),
		m_ullNextReadBufferOrder(0),
		m_ullNextWriteBufferOrder(0),
		m_ullNextFinishedBufferOrder(0)
	{
	}

	TOverlappedDataBufferQueue::TOverlappedDataBufferQueue(const logger::TLogFileDataPtr& spLogFileData, size_t stCount, size_t stBufferSize) :
		m_spLog(logger::MakeLogger(spLogFileData, L"DataBuffer")),
		m_eventReadPossible(true, false),
		m_eventWritePossible(true, false),
		m_eventWriteFinished(true, false),
		m_eventAllBuffersAccountedFor(true, false),
		m_bDataSourceFinished(false),
		m_bDataWritingFinished(false),
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
			m_eventAllBuffersAccountedFor.ResetEvent();

			return pBuffer;
		}

		return nullptr;
	}

	void TOverlappedDataBufferQueue::AddEmptyBuffer(TOverlappedDataBuffer* pBuffer)
	{
		if (!pBuffer)
			throw TCoreException(eErr_InvalidPointer, L"pBuffer", LOCATION);

		LOG_TRACE(m_spLog) << L"Queuing buffer as empty; buffer-order: " << pBuffer->GetBufferOrder();

		m_listEmptyBuffers.push_back(pBuffer);
		UpdateReadPossibleEvent();
		UpdateAllBuffersAccountedFor();
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

			if(pBuffer->GetErrorCode() == ERROR_SUCCESS)
			{
				// if this is the last part - mark that writing is finished, so that no other buffer will be written
				if(pBuffer->IsLastPart())
					m_bDataWritingFinished = true;

				++m_ullNextWriteBufferOrder;
			}

			UpdateWritePossibleEvent();
			m_eventAllBuffersAccountedFor.ResetEvent();

			return pBuffer;
		}

		return nullptr;
	}

	void TOverlappedDataBufferQueue::AddFullBuffer(TOverlappedDataBuffer* pBuffer)
	{
		if (!pBuffer)
			throw TCoreException(eErr_InvalidPointer, L"pBuffer", LOCATION);

		LOG_TRACE(m_spLog) << L"Queuing buffer as full; buffer-order: " << pBuffer->GetBufferOrder() <<
			L", requested-data-size: " << pBuffer->GetRequestedDataSize() <<
			L", real-data-size: " << pBuffer->GetRealDataSize() <<
			L", file-position: " << pBuffer->GetFilePosition() <<
			L", error-code: " << pBuffer->GetErrorCode() <<
			L", status-code: " << pBuffer->GetStatusCode() <<
			L", is-last-part: " << pBuffer->IsLastPart();

		std::pair<FullBuffersSet::iterator, bool> pairInsertInfo = m_setFullBuffers.insert(pBuffer);
		if (!pairInsertInfo.second)
			throw TCoreException(eErr_InvalidOverlappedPosition, L"Tried to re-insert same buffer into queue", LOCATION);

		if (pBuffer->IsLastPart())
		{
			m_bDataSourceFinished = true;
			UpdateReadPossibleEvent();
		}

		UpdateWritePossibleEvent();
		UpdateAllBuffersAccountedFor();
	}

	void TOverlappedDataBufferQueue::UpdateWritePossibleEvent()
	{
		if (m_bDataWritingFinished || m_setFullBuffers.empty())
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
		if (!m_setFinishedBuffers.empty())
		{
			TOverlappedDataBuffer* pBuffer = *m_setFinishedBuffers.begin();
			if (pBuffer->GetBufferOrder() != m_ullNextFinishedBufferOrder)
				return nullptr;

			m_setFinishedBuffers.erase(m_setFinishedBuffers.begin());

			m_eventWriteFinished.ResetEvent();	// faster than UpdateWriteFinishedEvent() and the final effect should be the same
			m_eventAllBuffersAccountedFor.ResetEvent();

			return pBuffer;
		}

		return nullptr;
	}

	void TOverlappedDataBufferQueue::MarkFinishedBufferAsComplete(TOverlappedDataBuffer* pBuffer)
	{
		if(!pBuffer)
			throw TCoreException(eErr_InvalidPointer, L"pBuffer", LOCATION);

		// allow next finished buffer to be processed
		++m_ullNextFinishedBufferOrder;
		UpdateWriteFinishedEvent();
	}

	void TOverlappedDataBufferQueue::AddFinishedBuffer(TOverlappedDataBuffer* pBuffer)
	{
		if (!pBuffer)
			throw TCoreException(eErr_InvalidPointer, L"pBuffer", LOCATION);

		LOG_TRACE(m_spLog) << L"Queuing buffer as finished; buffer-order: " << pBuffer->GetBufferOrder() <<
			L", requested-data-size: " << pBuffer->GetRequestedDataSize() <<
			L", real-data-size: " << pBuffer->GetRealDataSize() <<
			L", file-position: " << pBuffer->GetFilePosition() <<
			L", error-code: " << pBuffer->GetErrorCode() <<
			L", status-code: " << pBuffer->GetStatusCode() <<
			L", is-last-part: " << pBuffer->IsLastPart();

		std::pair<FullBuffersSet::iterator, bool> pairInsertInfo = m_setFinishedBuffers.insert(pBuffer);
		if (!pairInsertInfo.second)
			throw TCoreException(eErr_InvalidOverlappedPosition, L"Tried to re-insert same buffer into queue", LOCATION);

		UpdateWriteFinishedEvent();
		UpdateAllBuffersAccountedFor();
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

	void TOverlappedDataBufferQueue::UpdateAllBuffersAccountedFor()
	{
		size_t stCurrentBuffers = m_listEmptyBuffers.size() + m_setFullBuffers.size() + m_setFinishedBuffers.size();
		if (stCurrentBuffers == m_listAllBuffers.size())
			m_eventAllBuffersAccountedFor.SetEvent();
		else
			m_eventAllBuffersAccountedFor.ResetEvent();
	}

	void TOverlappedDataBufferQueue::ReinitializeBuffers(size_t stCount, size_t stBufferSize)
	{
		// sanity check - if any of the buffers are still in use, we can't change the sizes
		if (m_listAllBuffers.size() != m_listEmptyBuffers.size())
			throw TCoreException(eErr_InternalProblem, L"Some buffers are still in use", LOCATION);
		if (stBufferSize == 0)
			throw TCoreException(eErr_InvalidArgument, L"stBufferSize", LOCATION);

		if (stBufferSize != GetSingleBufferSize())
		{
			// buffer sizes increased - clear current buffers and proceed with creating new ones
			m_listAllBuffers.clear();
			m_listEmptyBuffers.clear();
		}
		else if (stCount == m_listAllBuffers.size())
			return;		// nothing really changed
		else if (stCount > m_listAllBuffers.size())
			stCount -= m_listAllBuffers.size();	// allocate only the missing buffers
		else if (stCount < m_listAllBuffers.size())
		{
			// there are too many buffers - reduce
			m_listEmptyBuffers.clear();

			size_t stCountToRemove = m_listAllBuffers.size() - stCount;

			m_listAllBuffers.erase(m_listAllBuffers.begin(), m_listAllBuffers.begin() + stCountToRemove);
			for (const auto& upElement : m_listAllBuffers)
			{
				m_listEmptyBuffers.push_back(upElement.get());
			}

			UpdateReadPossibleEvent();
			UpdateAllBuffersAccountedFor();
			return;
		}

		// allocate buffers
		while (stCount--)
		{
			auto upBuffer = std::make_unique<TOverlappedDataBuffer>(stBufferSize, this);
			m_listEmptyBuffers.push_back(upBuffer.get());
			m_listAllBuffers.push_back(std::move(upBuffer));
		}

		UpdateReadPossibleEvent();
		UpdateAllBuffersAccountedFor();
	}

	size_t TOverlappedDataBufferQueue::GetTotalBufferCount() const
	{
		return m_listAllBuffers.size();
	}

	size_t TOverlappedDataBufferQueue::GetSingleBufferSize() const
	{
		if (m_listAllBuffers.empty())
			return 0;

		return (*m_listAllBuffers.begin())->GetBufferSize();
	}

	void TOverlappedDataBufferQueue::DataSourceChanged()
	{
		CleanupBuffers();

		if (m_listAllBuffers.size() != m_listEmptyBuffers.size())
			throw TCoreException(eErr_InternalProblem, L"Some buffers are still in use", LOCATION);

		m_bDataSourceFinished = false;
		m_bDataWritingFinished = false;
		m_ullNextReadBufferOrder = 0;
		m_ullNextWriteBufferOrder = 0;
		m_ullNextFinishedBufferOrder = 0;

		UpdateReadPossibleEvent();
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

	void TOverlappedDataBufferQueue::WaitForMissingBuffersAndResetState(HANDLE hKillEvent)
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

		std::copy(m_setFullBuffers.begin(), m_setFullBuffers.end(), std::back_inserter(m_listEmptyBuffers));
		std::copy(m_setFinishedBuffers.begin(), m_setFinishedBuffers.end(), std::back_inserter(m_listEmptyBuffers));

		m_setFinishedBuffers.clear();
		m_setFullBuffers.clear();
	}
}
