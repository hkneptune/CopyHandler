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
	TOverlappedReaderWriter::TOverlappedReaderWriter(const logger::TLogFileDataPtr& spLogFileData, const TOverlappedMemoryPoolPtr& spMemoryPool,
		file_size_t ullFilePos, DWORD dwChunkSize) :
		m_spLog(logger::MakeLogger(spLogFileData, L"DataBuffer")),
		m_spMemoryPool(spMemoryPool),
		m_eventReadPossible(true, true),
		m_eventWritePossible(true, false),
		m_eventWriteFinished(true, false),
		m_eventAllBuffersAccountedFor(true, true),
		m_bDataSourceFinished(false),
		m_bDataWritingFinished(false),
		m_dwDataChunkSize(dwChunkSize),
		m_ullNextReadBufferOrder(ullFilePos),
		m_ullNextWriteBufferOrder(0),
		m_ullNextFinishedBufferOrder(0)
	{
		if(!spMemoryPool)
			throw TCoreException(eErr_InvalidArgument, L"spMemoryPool", LOCATION);
	}

	TOverlappedReaderWriter::~TOverlappedReaderWriter()
	{
	}

	TOverlappedDataBuffer* TOverlappedReaderWriter::GetEmptyBuffer()
	{
		TOverlappedDataBuffer* pBuffer = nullptr;

		// return buffers to re-read if exists
		if(!m_setEmptyBuffers.empty())
			pBuffer = m_setEmptyBuffers.pop_front();
		else
		{
			// get empty buffer and initialize
			pBuffer = m_spMemoryPool->GetBuffer();
			if(pBuffer)
			{
				pBuffer->SetParam(this);
				pBuffer->InitForRead(m_ullNextReadBufferOrder, m_dwDataChunkSize);

				m_ullNextReadBufferOrder += m_dwDataChunkSize;
			}
		}

		// reset the accounted-for event only if we managed to get the pointer, otherwise nothing is changing
		if(pBuffer)
			m_eventAllBuffersAccountedFor.ResetEvent();

		UpdateReadPossibleEvent();	// update read-possible always - if we're getting null with read-possible event set (which we should not), we need to reset it

		return pBuffer;
	}

	void TOverlappedReaderWriter::AddFailedReadBuffer(TOverlappedDataBuffer* pBuffer)
	{
		if (!pBuffer)
			throw TCoreException(eErr_InvalidPointer, L"pBuffer", LOCATION);

		LOG_TRACE(m_spLog) << L"Queuing buffer for re-read; buffer-order: " << pBuffer->GetFilePosition();

		m_setEmptyBuffers.insert(pBuffer);

		m_eventReadPossible.SetEvent();
		UpdateAllBuffersAccountedFor();
	}

	void TOverlappedReaderWriter::AddEmptyBuffer(TOverlappedDataBuffer* pBuffer)
	{
		if(!pBuffer)
			throw TCoreException(eErr_InvalidPointer, L"pBuffer", LOCATION);

		LOG_TRACE(m_spLog) << L"Releasing empty buffer; buffer-order: " << pBuffer->GetFilePosition();

		m_spMemoryPool->AddBuffer(pBuffer);

		UpdateReadPossibleEvent();
		UpdateAllBuffersAccountedFor();
	}

	void TOverlappedReaderWriter::UpdateReadPossibleEvent()
	{
		if(!m_setEmptyBuffers.empty() || (!m_bDataSourceFinished && m_spMemoryPool->HasBuffers()))
			m_eventReadPossible.SetEvent();
		else
			m_eventReadPossible.ResetEvent();
	}

	TOverlappedDataBuffer* TOverlappedReaderWriter::GetFullBuffer()
	{
		if (!m_setFullBuffers.empty())
		{
			TOverlappedDataBuffer* pBuffer = *m_setFullBuffers.begin();
			if (pBuffer->GetFilePosition() != m_ullNextWriteBufferOrder)
				return nullptr;

			m_setFullBuffers.erase(m_setFullBuffers.begin());

			if(pBuffer->GetErrorCode() == ERROR_SUCCESS)
			{
				// if this is the last part - mark that writing is finished, so that no other buffer will be written
				if(pBuffer->IsLastPart())
					m_bDataWritingFinished = true;

				m_ullNextWriteBufferOrder += m_dwDataChunkSize;
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

		LOG_TRACE(m_spLog) << L"Queuing buffer as full; buffer-order: " << pBuffer->GetFilePosition() <<
			L", requested-data-size: " << pBuffer->GetRequestedDataSize() <<
			L", real-data-size: " << pBuffer->GetRealDataSize() <<
			L", file-position: " << pBuffer->GetFilePosition() <<
			L", error-code: " << pBuffer->GetErrorCode() <<
			L", status-code: " << pBuffer->GetStatusCode() <<
			L", is-last-part: " << pBuffer->IsLastPart();

		auto pairInsertInfo = m_setFullBuffers.insert(pBuffer);
		if (!pairInsertInfo.second)
			throw TCoreException(eErr_InvalidOverlappedPosition, L"Tried to re-insert same buffer into queue", LOCATION);

		if (pBuffer->IsLastPart())
			m_bDataSourceFinished = true;

		UpdateWritePossibleEvent();
		UpdateAllBuffersAccountedFor();
	}

	void TOverlappedReaderWriter::AddFailedFullBuffer(TOverlappedDataBuffer* pBuffer)
	{
		if(!pBuffer)
			throw TCoreException(eErr_InvalidPointer, L"pBuffer", LOCATION);

		LOG_TRACE(m_spLog) << L"Queuing buffer as full (failed); buffer-order: " << pBuffer->GetFilePosition() <<
			L", requested-data-size: " << pBuffer->GetRequestedDataSize() <<
			L", real-data-size: " << pBuffer->GetRealDataSize() <<
			L", file-position: " << pBuffer->GetFilePosition() <<
			L", error-code: " << pBuffer->GetErrorCode() <<
			L", status-code: " << pBuffer->GetStatusCode() <<
			L", is-last-part: " << pBuffer->IsLastPart();

		// overwrite error code (to avoid treating the buffer as failed read)
		pBuffer->SetErrorCode(ERROR_SUCCESS);

		auto pairInsertInfo = m_setFullBuffers.insert(pBuffer);
		if(!pairInsertInfo.second)
			throw TCoreException(eErr_InvalidOverlappedPosition, L"Tried to re-insert same buffer into queue", LOCATION);

		if(pBuffer->IsLastPart())
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
			if (pFirstBuffer->GetFilePosition() == m_ullNextWriteBufferOrder)
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
			if (pBuffer->GetFilePosition() != m_ullNextFinishedBufferOrder)
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
		m_ullNextFinishedBufferOrder += m_dwDataChunkSize;
		UpdateWriteFinishedEvent();
	}

	void TOverlappedReaderWriter::AddFinishedBuffer(TOverlappedDataBuffer* pBuffer)
	{
		if (!pBuffer)
			throw TCoreException(eErr_InvalidPointer, L"pBuffer", LOCATION);

		LOG_TRACE(m_spLog) << L"Queuing buffer as finished; buffer-order: " << pBuffer->GetFilePosition() <<
			L", requested-data-size: " << pBuffer->GetRequestedDataSize() <<
			L", real-data-size: " << pBuffer->GetRealDataSize() <<
			L", file-position: " << pBuffer->GetFilePosition() <<
			L", error-code: " << pBuffer->GetErrorCode() <<
			L", status-code: " << pBuffer->GetStatusCode() <<
			L", is-last-part: " << pBuffer->IsLastPart();

		auto pairInsertInfo = m_setFinishedBuffers.insert(pBuffer);
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
			if (pFirstBuffer->GetFilePosition() == m_ullNextFinishedBufferOrder)
				m_eventWriteFinished.SetEvent();
			else
				m_eventWriteFinished.ResetEvent();
		}
	}

	void TOverlappedReaderWriter::UpdateAllBuffersAccountedFor()
	{
		size_t stCurrentBuffers = m_spMemoryPool->GetAvailableBufferCount() + m_setFullBuffers.size() + m_setFinishedBuffers.size() + m_setEmptyBuffers.size();
		if (stCurrentBuffers == m_spMemoryPool->GetTotalBufferCount())
			m_eventAllBuffersAccountedFor.SetEvent();
		else
			m_eventAllBuffersAccountedFor.ResetEvent();
	}

	void TOverlappedReaderWriter::DataSourceChanged()
	{
		CleanupBuffers();

		if (!m_spMemoryPool->AreAllBuffersAccountedFor())
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
					m_spMemoryPool->AddBuffer(*iterCurrent);
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

		auto funcAdd = [&](TOverlappedDataBuffer* pBuffer) { m_spMemoryPool->AddBuffer(pBuffer); };

		std::for_each(m_setFullBuffers.begin(), m_setFullBuffers.end(), funcAdd);
		std::for_each(m_setFinishedBuffers.begin(), m_setFinishedBuffers.end(), funcAdd);

		m_setFinishedBuffers.clear();
		m_setFullBuffers.clear();
	}
}
