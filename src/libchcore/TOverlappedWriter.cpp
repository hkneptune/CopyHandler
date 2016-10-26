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
		unsigned long long ullFilePos) :
		m_spLog(logger::MakeLogger(spLogFileData, L"DataBuffer")),
		m_tBuffersToWrite(spBuffersToWrite),
		m_tFailedWriteBuffers(),
		m_tFinishedBuffers(ullFilePos),
		m_bDataWritingFinished(false)
	{
		if(!spBuffersToWrite)
			throw TCoreException(eErr_InvalidArgument, L"spBuffersToWrite", LOCATION);
	}

	TOverlappedWriter::~TOverlappedWriter()
	{
	}

	TOverlappedDataBuffer* TOverlappedWriter::GetWriteBuffer()
	{
		return m_tBuffersToWrite.Pop();
	}

	void TOverlappedWriter::AddFailedWriteBuffer(TOverlappedDataBuffer* pBuffer)
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

		m_tFailedWriteBuffers.PushWithFallback(pBuffer, m_tBuffersToWrite);
	}

	TOverlappedDataBuffer* TOverlappedWriter::GetFailedWriteBuffer()
	{
		return m_tFailedWriteBuffers.Pop();
	}

	TOverlappedDataBuffer* TOverlappedWriter::GetFinishedBuffer()
	{
		TOverlappedDataBuffer* pBuffer = m_tFinishedBuffers.Pop();

		if (pBuffer && pBuffer->IsLastPart())
		{
			if (m_pLastPartBuffer != nullptr)
				throw TCoreException(eErr_InternalProblem, L"Encountered another 'last-part' finished buffer", LOCATION);
			m_pLastPartBuffer = pBuffer;
		}

		return pBuffer;
	}

	void TOverlappedWriter::MarkAsFinalized(TOverlappedDataBuffer* pBuffer)
	{
		if (!pBuffer)
			throw TCoreException(eErr_InvalidPointer, L"pBuffer", LOCATION);

		if (pBuffer != m_pLastPartBuffer)
			throw TCoreException(eErr_InvalidArgument, L"Trying to mark different buffer as finalized", LOCATION);

		m_bDataWritingFinished = true;
		m_pLastPartBuffer = nullptr;
	}

	void TOverlappedWriter::AddFinishedBuffer(TOverlappedDataBuffer* pBuffer)
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

		m_tFinishedBuffers.Push(pBuffer);
	}

	size_t TOverlappedWriter::GetBufferCount() const
	{
		return m_tFailedWriteBuffers.GetCount() + m_tFinishedBuffers.GetCount();
	}

	void TOverlappedWriter::ReleaseBuffers(const TBufferListPtr& spList)
	{
		m_tBuffersToWrite.ReleaseBuffers(spList);
		m_tFailedWriteBuffers.ReleaseBuffers(spList);
		m_tFinishedBuffers.ReleaseBuffers(spList);
	}
}
