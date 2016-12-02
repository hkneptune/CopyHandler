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
#include "TOverlappedReader.h"
#include "TOverlappedDataBuffer.h"
#include "TCoreException.h"
#include "ErrorCodes.h"

namespace chcore
{
	TOverlappedReader::TOverlappedReader(const logger::TLogFileDataPtr& spLogFileData,
		const TBufferListPtr& spEmptyBuffers,
		const TOverlappedProcessorRangePtr& spDataRange,
		DWORD dwChunkSize,
		size_t stMaxOtfBuffers, size_t stMaxReadAheadBuffers,
		TSharedCountPtr<size_t> spOtfBuffersCount) :
		m_spLog(logger::MakeLogger(spLogFileData, L"DataBuffer")),
		m_spFullBuffers(std::make_shared<TOrderedBufferQueue>(spEmptyBuffers, spDataRange ? spDataRange->GetResumePosition() : 0)),
		m_tInputBuffers(spEmptyBuffers, spDataRange ? spDataRange->GetResumePosition() : 0, dwChunkSize, stMaxOtfBuffers, stMaxReadAheadBuffers, spOtfBuffersCount, m_spFullBuffers->GetSharedCount())
	{
		if(!spLogFileData)
			throw TCoreException(eErr_InvalidArgument, L"spLogFileData is NULL", LOCATION);
		if(!spEmptyBuffers)
			throw TCoreException(eErr_InvalidArgument, L"spMemoryPool", LOCATION);
		if(!spDataRange)
			throw TCoreException(eErr_InvalidArgument, L"spDataRange is NULL", LOCATION);
		if(!spOtfBuffersCount)
			throw TCoreException(eErr_InvalidArgument, L"spOtfBuffersCount is NULL", LOCATION);

		m_dataRangeChanged = spDataRange->GetNotifier().connect(boost::bind(&TOverlappedReader::UpdateProcessingRange, this, _1));
	}

	TOverlappedReader::~TOverlappedReader()
	{
		m_dataRangeChanged.disconnect();
	}

	TOverlappedDataBuffer* TOverlappedReader::GetEmptyBuffer()
	{
		TOverlappedDataBuffer* pBuffer = m_tInputBuffers.Pop();
		if (pBuffer)
			pBuffer->SetParam(this);
		return pBuffer;
	}

	void TOverlappedReader::AddEmptyBuffer(TOverlappedDataBuffer* pBuffer)
	{
		if(!pBuffer)
			throw TCoreException(eErr_InvalidPointer, L"pBuffer", LOCATION);

		LOG_TRACE(m_spLog) << L"Queuing buffer " << pBuffer << L" as really-empty; buffer-order: " << pBuffer->GetFilePosition() <<
			L", requested-data-size: " << pBuffer->GetRequestedDataSize() <<
			L", real-data-size: " << pBuffer->GetRealDataSize() <<
			L", file-position: " << pBuffer->GetFilePosition() <<
			L", error-code: " << pBuffer->GetErrorCode() <<
			L", status-code: " << pBuffer->GetStatusCode() <<
			L", is-last-part: " << pBuffer->IsLastPart();

		m_tInputBuffers.PushEmpty(pBuffer);
	}

	void TOverlappedReader::AddRetryBuffer(TOverlappedDataBuffer* pBuffer)
	{
		if(!pBuffer)
			throw TCoreException(eErr_InvalidPointer, L"pBuffer", LOCATION);

		LOG_TRACE(m_spLog) << L"Queuing buffer " << pBuffer << L" as empty; buffer-order: " << pBuffer->GetFilePosition() <<
			L", requested-data-size: " << pBuffer->GetRequestedDataSize() <<
			L", real-data-size: " << pBuffer->GetRealDataSize() <<
			L", file-position: " << pBuffer->GetFilePosition() <<
			L", error-code: " << pBuffer->GetErrorCode() <<
			L", status-code: " << pBuffer->GetStatusCode() <<
			L", is-last-part: " << pBuffer->IsLastPart();

		m_tInputBuffers.Push(pBuffer);
	}

	void TOverlappedReader::AddFailedReadBuffer(TOverlappedDataBuffer* pBuffer)
	{
		if (!pBuffer)
			throw TCoreException(eErr_InvalidPointer, L"pBuffer", LOCATION);

		LOG_TRACE(m_spLog) << L"Queuing buffer " << pBuffer << L" as failed-read; buffer-order: " << pBuffer->GetFilePosition() <<
			L", requested-data-size: " << pBuffer->GetRequestedDataSize() <<
			L", real-data-size: " << pBuffer->GetRealDataSize() <<
			L", file-position: " << pBuffer->GetFilePosition() <<
			L", error-code: " << pBuffer->GetErrorCode() <<
			L", status-code: " << pBuffer->GetStatusCode() <<
			L", is-last-part: " << pBuffer->IsLastPart();

		m_spFullBuffers->PushError(pBuffer, m_tInputBuffers);
	}

	TOverlappedDataBuffer* TOverlappedReader::GetFailedReadBuffer()
	{
		TOverlappedDataBuffer* pBuffer = m_spFullBuffers->PopError();
		if (pBuffer)
			pBuffer->SetParam(this);

		return pBuffer;
	}

	void TOverlappedReader::AddFinishedReadBuffer(TOverlappedDataBuffer* pBuffer)
	{
		if (!pBuffer)
			throw TCoreException(eErr_InvalidPointer, L"pBuffer", LOCATION);

		LOG_TRACE(m_spLog) << L"Queuing buffer " << pBuffer << L" as finished-read; buffer-order: " << pBuffer->GetFilePosition() <<
			L", requested-data-size: " << pBuffer->GetRequestedDataSize() <<
			L", real-data-size: " << pBuffer->GetRealDataSize() <<
			L", file-position: " << pBuffer->GetFilePosition() <<
			L", error-code: " << pBuffer->GetErrorCode() <<
			L", status-code: " << pBuffer->GetStatusCode() <<
			L", is-last-part: " << pBuffer->IsLastPart();

		if(pBuffer->IsLastPart())
			m_tInputBuffers.SetDataSourceFinished(pBuffer);

		m_spFullBuffers->Push(pBuffer);
	}

	TOverlappedDataBuffer* TOverlappedReader::GetFinishedReadBuffer()
	{
		TOverlappedDataBuffer* pBuffer = m_spFullBuffers->Pop();
		if(pBuffer)
			pBuffer->SetParam(this);

		return pBuffer;
	}

	TOrderedBufferQueuePtr TOverlappedReader::GetFinishedQueue() const
	{
		return m_spFullBuffers;
	}

	bool TOverlappedReader::IsDataSourceFinished() const
	{
		return m_tInputBuffers.IsDataSourceFinished();
	}

	void TOverlappedReader::ClearBuffers()
	{
		m_tInputBuffers.ClearBuffers();
		// Do not clear full buffers as they might be in use
		//m_spFullBuffers->ClearBuffers(m_spEmptyBuffers);
	}

	void TOverlappedReader::UpdateProcessingRange(unsigned long long ullNewPosition)
	{
		m_tInputBuffers.UpdateProcessingRange(ullNewPosition);
		m_spFullBuffers->UpdateProcessingRange(ullNewPosition);
	}

	HANDLE TOverlappedReader::GetEventReadPossibleHandle() const
	{
		return m_tInputBuffers.GetHasBuffersEvent();
	}

	HANDLE TOverlappedReader::GetEventReadFailedHandle() const
	{
		return m_spFullBuffers->GetHasErrorEvent();
	}

	HANDLE TOverlappedReader::GetEventDataSourceFinishedHandle() const
	{
		return m_spFullBuffers->GetHasReadingFinished();
	}
}
