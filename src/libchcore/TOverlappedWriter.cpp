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
#include "TOverlappedWriter.h"
#include "TOverlappedDataBuffer.h"
#include "TCoreException.h"
#include "ErrorCodes.h"

namespace chcore
{
	TOverlappedWriter::TOverlappedWriter(const logger::TLogFileDataPtr& spLogFileData, const TOrderedBufferQueuePtr& spBuffersToWrite,
		const TOverlappedProcessorRangePtr& spRange, const TBufferListPtr& spEmptyBuffers, size_t stMaxOtfBuffers, TSharedCountPtr<size_t> spOtfBuffersCount) :
		m_spLog(logger::MakeLogger(spLogFileData, L"DataBuffer")),
		m_spEmptyBuffers(spEmptyBuffers),
		m_tBuffersToWrite(spBuffersToWrite, stMaxOtfBuffers, spOtfBuffersCount),
		m_tFinishedBuffers(spEmptyBuffers, spRange != nullptr ? spRange->GetResumePosition() : 0)
	{
		if(!spLogFileData)
			throw TCoreException(eErr_InvalidArgument, L"spLogFileData is NULL", LOCATION);
		if(!spBuffersToWrite)
			throw TCoreException(eErr_InvalidArgument, L"spBuffersToWrite is NULL", LOCATION);
		if(!spEmptyBuffers)
			throw TCoreException(eErr_InvalidArgument, L"spEmptyBuffers is NULL", LOCATION);
		if(!spRange)
			throw TCoreException(eErr_InvalidArgument, L"spRange is NULL", LOCATION);
		if(!spOtfBuffersCount)
			throw TCoreException(eErr_InvalidArgument, L"spOtfBuffersCount is NULL", LOCATION);

		m_dataRangeChanged = spRange->GetNotifier().connect(boost::bind(&TOverlappedWriter::UpdateProcessingRange, this, _1));
	}

	TOverlappedWriter::~TOverlappedWriter()
	{
		m_dataRangeChanged.disconnect();
	}

	void TOverlappedWriter::AddEmptyBuffer(TOverlappedDataBuffer* pBuffer)
	{
		m_spEmptyBuffers->Push(pBuffer);
	}

	void TOverlappedWriter::AddRetryBuffer(TOverlappedDataBuffer* pBuffer)
	{
		if(!pBuffer)
			throw TCoreException(eErr_InvalidPointer, L"pBuffer", LOCATION);

		LOG_TRACE(m_spLog) << L"Queuing buffer " << pBuffer << L" as write-retry; buffer-order: " << pBuffer->GetFilePosition() <<
			L", requested-data-size: " << pBuffer->GetRequestedDataSize() <<
			L", real-data-size: " << pBuffer->GetRealDataSize() <<
			L", file-position: " << pBuffer->GetFilePosition() <<
			L", error-code: " << pBuffer->GetErrorCode() <<
			L", status-code: " << pBuffer->GetStatusCode() <<
			L", is-last-part: " << pBuffer->IsLastPart();

		m_tBuffersToWrite.Push(pBuffer);
	}

	TOverlappedDataBuffer* TOverlappedWriter::GetWriteBuffer()
	{
		TOverlappedDataBuffer* pBuffer = m_tBuffersToWrite.Pop();
		if (pBuffer)
			pBuffer->SetParam(this);

		return pBuffer;
	}

	void TOverlappedWriter::AddFailedWriteBuffer(TOverlappedDataBuffer* pBuffer)
	{
		if(!pBuffer)
			throw TCoreException(eErr_InvalidPointer, L"pBuffer", LOCATION);

		LOG_TRACE(m_spLog) << L"Queuing buffer " << pBuffer << L" as failed-write; buffer-order: " << pBuffer->GetFilePosition() <<
			L", requested-data-size: " << pBuffer->GetRequestedDataSize() <<
			L", real-data-size: " << pBuffer->GetRealDataSize() <<
			L", file-position: " << pBuffer->GetFilePosition() <<
			L", error-code: " << pBuffer->GetErrorCode() <<
			L", status-code: " << pBuffer->GetStatusCode() <<
			L", is-last-part: " << pBuffer->IsLastPart();

		m_tFinishedBuffers.PushError(pBuffer, m_tBuffersToWrite);
	}

	TOverlappedDataBuffer* TOverlappedWriter::GetFailedWriteBuffer()
	{
		TOverlappedDataBuffer* pBuffer = m_tFinishedBuffers.PopError();
		if (pBuffer)
			pBuffer->SetParam(this);

		return pBuffer;
	}

	TOverlappedDataBuffer* TOverlappedWriter::GetFinishedBuffer()
	{
		TOverlappedDataBuffer* pBuffer = m_tFinishedBuffers.Pop();

		if (pBuffer)
		{
			if(pBuffer->IsLastPart())
			{
				if (m_pLastPartBuffer != nullptr)
					throw TCoreException(eErr_InternalProblem, L"Encountered another 'last-part' finished buffer", LOCATION);
				m_pLastPartBuffer = pBuffer;
			}

			pBuffer->SetParam(this);
		}

		return pBuffer;
	}

	void TOverlappedWriter::MarkAsFinalized(TOverlappedDataBuffer* pBuffer)
	{
		if(!pBuffer)
			throw TCoreException(eErr_InvalidPointer, L"pBuffer", LOCATION);

		if (pBuffer != m_pLastPartBuffer)
			throw TCoreException(eErr_InvalidArgument, L"Trying to mark different buffer as finalized", LOCATION);

		LOG_TRACE(m_spLog) << L"Marking buffer " << pBuffer << L" as finalized-write; buffer-order: " << pBuffer->GetFilePosition() <<
			L", requested-data-size: " << pBuffer->GetRequestedDataSize() <<
			L", real-data-size: " << pBuffer->GetRealDataSize() <<
			L", file-position: " << pBuffer->GetFilePosition() <<
			L", error-code: " << pBuffer->GetErrorCode() <<
			L", status-code: " << pBuffer->GetStatusCode() <<
			L", is-last-part: " << pBuffer->IsLastPart();

		m_pLastPartBuffer = nullptr;
	}

	HANDLE TOverlappedWriter::GetEventWritePossibleHandle() const
	{
		return m_tBuffersToWrite.GetHasBuffersEvent();
	}

	HANDLE TOverlappedWriter::GetEventWriteFailedHandle() const
	{
		return m_tFinishedBuffers.GetHasErrorEvent();
	}

	HANDLE TOverlappedWriter::GetEventWriteFinishedHandle() const
	{
		return m_tFinishedBuffers.GetHasBuffersEvent();
	}

	void TOverlappedWriter::UpdateProcessingRange(unsigned long long ullNewPosition)
	{
		m_tFinishedBuffers.UpdateProcessingRange(ullNewPosition);
	}

	void TOverlappedWriter::ClearBuffers()
	{
		m_tBuffersToWrite.ClearBuffers(m_spEmptyBuffers);
		m_tFinishedBuffers.ClearBuffers();
	}

	void TOverlappedWriter::AddFinishedBuffer(TOverlappedDataBuffer* pBuffer)
	{
		if (!pBuffer)
			throw TCoreException(eErr_InvalidPointer, L"pBuffer", LOCATION);

		LOG_TRACE(m_spLog) << L"Queuing buffer " << pBuffer << L" as finished-write; buffer-order: " << pBuffer->GetFilePosition() <<
			L", requested-data-size: " << pBuffer->GetRequestedDataSize() <<
			L", real-data-size: " << pBuffer->GetRealDataSize() <<
			L", file-position: " << pBuffer->GetFilePosition() <<
			L", error-code: " << pBuffer->GetErrorCode() <<
			L", status-code: " << pBuffer->GetStatusCode() <<
			L", is-last-part: " << pBuffer->IsLastPart();

		m_tFinishedBuffers.Push(pBuffer);
	}
}
