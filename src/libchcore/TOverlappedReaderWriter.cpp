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
#include "TOverlappedReaderWriter.h"
#include "TOverlappedDataBuffer.h"
#include "TCoreException.h"
#include "ErrorCodes.h"
#include <array>

namespace chcore
{
	bool CompareBufferPositions::operator()(const TOverlappedDataBuffer* pBufferA, const TOverlappedDataBuffer* pBufferB)
	{
		return pBufferA->GetBufferOrder() < pBufferB->GetBufferOrder();
	}

	TOverlappedReaderWriter::TOverlappedReaderWriter(const logger::TLogFileDataPtr& spLogFileData, const TOverlappedDataBufferQueuePtr& spBuffers) :
		m_spLog(logger::MakeLogger(spLogFileData, L"DataBuffer")),
		m_spBuffers(spBuffers),
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

	TOverlappedReaderWriter::~TOverlappedReaderWriter()
	{
	}

	TOverlappedDataBuffer* TOverlappedReaderWriter::GetEmptyBuffer()
	{
		TOverlappedDataBuffer* pBuffer = m_spBuffers->GetBuffer();
		if(pBuffer)
		{
			pBuffer->SetParam(this);
			pBuffer->SetBufferOrder(m_ullNextReadBufferOrder++);

			m_eventAllBuffersAccountedFor.ResetEvent();
		}

		return pBuffer;
	}

	void TOverlappedReaderWriter::AddEmptyBuffer(TOverlappedDataBuffer* pBuffer)
	{
		if (!pBuffer)
			throw TCoreException(eErr_InvalidPointer, L"pBuffer", LOCATION);

		LOG_TRACE(m_spLog) << L"Queuing buffer as empty; buffer-order: " << pBuffer->GetBufferOrder();

		pBuffer->SetParam(nullptr);
		m_spBuffers->AddBuffer(pBuffer);
		UpdateAllBuffersAccountedFor();
	}

	TOverlappedDataBuffer* TOverlappedReaderWriter::GetFullBuffer()
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

	void TOverlappedReaderWriter::AddFullBuffer(TOverlappedDataBuffer* pBuffer)
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
			m_bDataSourceFinished = true;

		UpdateWritePossibleEvent();
		UpdateAllBuffersAccountedFor();
	}

	void TOverlappedReaderWriter::UpdateWritePossibleEvent()
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

	TOverlappedDataBuffer* TOverlappedReaderWriter::GetFinishedBuffer()
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

	void TOverlappedReaderWriter::MarkFinishedBufferAsComplete(TOverlappedDataBuffer* pBuffer)
	{
		if(!pBuffer)
			throw TCoreException(eErr_InvalidPointer, L"pBuffer", LOCATION);

		// allow next finished buffer to be processed
		++m_ullNextFinishedBufferOrder;
		UpdateWriteFinishedEvent();
	}

	void TOverlappedReaderWriter::AddFinishedBuffer(TOverlappedDataBuffer* pBuffer)
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

	void TOverlappedReaderWriter::UpdateWriteFinishedEvent()
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

	void TOverlappedReaderWriter::UpdateAllBuffersAccountedFor()
	{
		size_t stCurrentBuffers = m_spBuffers->GetAvailableBufferCount() + m_setFullBuffers.size() + m_setFinishedBuffers.size();
		if (stCurrentBuffers == m_spBuffers->GetTotalBufferCount())
			m_eventAllBuffersAccountedFor.SetEvent();
		else
			m_eventAllBuffersAccountedFor.ResetEvent();
	}

	void TOverlappedReaderWriter::DataSourceChanged()
	{
		CleanupBuffers();

		if (!m_spBuffers->AreAllBuffersAccountedFor())
			throw TCoreException(eErr_InternalProblem, L"Some buffers are still in use", LOCATION);

		m_bDataSourceFinished = false;
		m_bDataWritingFinished = false;
		m_ullNextReadBufferOrder = 0;
		m_ullNextWriteBufferOrder = 0;
		m_ullNextFinishedBufferOrder = 0;

		m_eventWritePossible.ResetEvent();
		m_eventWriteFinished.ResetEvent();
	}

	void TOverlappedReaderWriter::CleanupBuffers()
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
					m_spBuffers->AddBuffer(*iterCurrent);
					iterCurrent = m_setFullBuffers.erase(iterCurrent);
				}
				else
					++iterCurrent;
			}
		}
	}

	void TOverlappedReaderWriter::WaitForMissingBuffersAndResetState(HANDLE hKillEvent)
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

		auto funcAdd = [&](TOverlappedDataBuffer* pBuffer) { m_spBuffers->AddBuffer(pBuffer); };

		std::for_each(m_setFullBuffers.begin(), m_setFullBuffers.end(), funcAdd);
		std::for_each(m_setFinishedBuffers.begin(), m_setFinishedBuffers.end(), funcAdd);

		m_setFinishedBuffers.clear();
		m_setFullBuffers.clear();
	}
}
