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
	TOverlappedReader::TOverlappedReader(const logger::TLogFileDataPtr& spLogFileData, const TBufferListPtr& spEmptyBuffers,
		unsigned long long ullFilePos, DWORD dwChunkSize) :
		m_spLog(logger::MakeLogger(spLogFileData, L"DataBuffer")),
		m_spEmptyBuffers(spEmptyBuffers),
		m_tInputBuffers(spEmptyBuffers, ullFilePos, dwChunkSize),
		m_spFullBuffers(std::make_shared<TOrderedBufferQueue>(ullFilePos))
	{
		if(!spLogFileData)
			throw TCoreException(eErr_InvalidArgument, L"spLogFileData is NULL", LOCATION);
		if(!spEmptyBuffers)
			throw TCoreException(eErr_InvalidArgument, L"spMemoryPool", LOCATION);
	}

	TOverlappedReader::~TOverlappedReader()
	{
	}

	TOverlappedDataBuffer* TOverlappedReader::GetEmptyBuffer()
	{
		if(m_bReleaseMode)
			return nullptr;

		TOverlappedDataBuffer* pBuffer = m_tInputBuffers.Pop();
		if (pBuffer)
			pBuffer->SetParam(this);
		return pBuffer;
	}

	void TOverlappedReader::AddEmptyBuffer(TOverlappedDataBuffer* pBuffer, bool bKeepPosition)
	{
		if(!pBuffer)
			throw TCoreException(eErr_InvalidPointer, L"pBuffer", LOCATION);

		if(m_bReleaseMode)
			m_tInputBuffers.Push(pBuffer, false);
		else
		{
			LOG_TRACE(m_spLog) << L"Queuing buffer " << pBuffer << L" as empty; buffer-order: " << pBuffer->GetFilePosition() <<
				L", requested-data-size: " << pBuffer->GetRequestedDataSize() <<
				L", real-data-size: " << pBuffer->GetRealDataSize() <<
				L", file-position: " << pBuffer->GetFilePosition() <<
				L", error-code: " << pBuffer->GetErrorCode() <<
				L", status-code: " << pBuffer->GetStatusCode() <<
				L", is-last-part: " << pBuffer->IsLastPart();

			m_tInputBuffers.Push(pBuffer, bKeepPosition);
		}
	}

	void TOverlappedReader::AddFailedReadBuffer(TOverlappedDataBuffer* pBuffer)
	{
		if (!pBuffer)
			throw TCoreException(eErr_InvalidPointer, L"pBuffer", LOCATION);

		if(m_bReleaseMode)
			m_tInputBuffers.Push(pBuffer, false);
		else
		{
			LOG_TRACE(m_spLog) << L"Queuing buffer " << pBuffer << L" as failed-read; buffer-order: " << pBuffer->GetFilePosition() <<
				L", requested-data-size: " << pBuffer->GetRequestedDataSize() <<
				L", real-data-size: " << pBuffer->GetRealDataSize() <<
				L", file-position: " << pBuffer->GetFilePosition() <<
				L", error-code: " << pBuffer->GetErrorCode() <<
				L", status-code: " << pBuffer->GetStatusCode() <<
				L", is-last-part: " << pBuffer->IsLastPart();

			m_spFullBuffers->PushError(pBuffer, m_tInputBuffers);
		}
	}

	TOverlappedDataBuffer* TOverlappedReader::GetFailedReadBuffer()
	{
		if(m_bReleaseMode)
			return nullptr;

		TOverlappedDataBuffer* pBuffer = m_spFullBuffers->PopError();
		if (pBuffer)
			pBuffer->SetParam(this);

		return pBuffer;
	}

	void TOverlappedReader::AddFinishedReadBuffer(TOverlappedDataBuffer* pBuffer)
	{
		if (!pBuffer)
			throw TCoreException(eErr_InvalidPointer, L"pBuffer", LOCATION);

		if(m_bReleaseMode)
		{
			m_tInputBuffers.Push(pBuffer, false);
		}
		else
		{
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
	}

	TOverlappedDataBuffer* TOverlappedReader::GetFinishedReadBuffer()
	{
		if(m_bReleaseMode)
			return nullptr;

		TOverlappedDataBuffer* pBuffer = m_spFullBuffers->Pop();
		if(pBuffer)
			pBuffer->SetParam(this);

		return pBuffer;
	}

	TOrderedBufferQueuePtr TOverlappedReader::GetFinishedQueue() const
	{
		return m_spFullBuffers;
	}

	void TOverlappedReader::ReleaseBuffers()
	{
		m_bReleaseMode = true;
		m_tInputBuffers.ReleaseBuffers(m_spEmptyBuffers);
		m_spFullBuffers->ReleaseBuffers(m_spEmptyBuffers);
	}
}
