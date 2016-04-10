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
/// @file  TDataBuffer.cpp
/// @date  2012/03/04
/// @brief Contains class representing buffer for data.
// ============================================================================
#include "stdafx.h"
#include "TOverlappedDataBuffer.h"
#include <boost/bind.hpp>
#include "TCoreException.h"
#include "ErrorCodes.h"
#include "IOverlappedDataBufferQueue.h"
#include "RoundingFunctions.h"
#include <atltrace.h>
#include <boost/numeric/conversion/cast.hpp>

#define STATUS_END_OF_FILE 0xc0000011

namespace chcore
{
	///////////////////////////////////////////////////////////////////////////////////
	// class TOverlappedDataBuffer
	VOID CALLBACK OverlappedReadCompleted(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped)
	{
		TOverlappedDataBuffer* pBuffer = (TOverlappedDataBuffer*)lpOverlapped;

		// determine if this is the last packet
		bool bEof = (dwErrorCode == ERROR_HANDLE_EOF ||
			pBuffer->GetStatusCode() == STATUS_END_OF_FILE ||
			(dwErrorCode == ERROR_SUCCESS && pBuffer->GetBytesTransferred() != pBuffer->GetRequestedDataSize()));

		// reset status code and error code if they pointed out to EOF
		if (pBuffer->GetStatusCode() == STATUS_END_OF_FILE)
			pBuffer->SetStatusCode(0);

		pBuffer->SetErrorCode(dwErrorCode == ERROR_HANDLE_EOF ? ERROR_SUCCESS : dwErrorCode);

		pBuffer->SetRealDataSize(dwNumberOfBytesTransfered);
		pBuffer->SetLastPart(bEof);

		pBuffer->RequeueAsFull();
	}

	VOID CALLBACK OverlappedWriteCompleted(DWORD dwErrorCode, DWORD /*dwNumberOfBytesTransfered*/, LPOVERLAPPED lpOverlapped)
	{
		TOverlappedDataBuffer* pBuffer = (TOverlappedDataBuffer*)lpOverlapped;

		pBuffer->SetErrorCode(dwErrorCode);
		pBuffer->RequeueAsFinished();
	}

	TOverlappedDataBuffer::TOverlappedDataBuffer(size_t stBufferSize, IOverlappedDataBufferQueue* pQueue) :
		m_pQueue(pQueue)
	{
		if (!m_pQueue)
			throw TCoreException(eErr_InvalidPointer, L"m_pQueue", LOCATION);

		// initialize OVERLAPPED members
		Internal = 0;
		InternalHigh = 0;
		Offset = 0;
		OffsetHigh = 0;
		hEvent = NULL;

		// create buffer
		ReinitializeBuffer(stBufferSize);
	}

	TOverlappedDataBuffer::~TOverlappedDataBuffer()
	{
		ReleaseBuffer();
	}

	void TOverlappedDataBuffer::ReinitializeBuffer(size_t stNewBufferSize)
	{
		if (stNewBufferSize != m_stBufferSize)
		{
			ReleaseBuffer();

			m_pBuffer = VirtualAlloc(NULL, stNewBufferSize, MEM_COMMIT, PAGE_READWRITE);
			if (!m_pBuffer)
				throw TCoreException(eErr_CannotAllocateMemory, L"VirtualAlloc failed", LOCATION);
			m_stBufferSize = stNewBufferSize;
		}
	}

	void TOverlappedDataBuffer::ReleaseBuffer()
	{
		if (m_pBuffer)
		{
			VirtualFree(m_pBuffer, 0, MEM_RELEASE);
			m_stBufferSize = 0;
			m_pBuffer = nullptr;
		}
	}

	LPVOID TOverlappedDataBuffer::GetBufferPtr()
	{
		return m_pBuffer;
	}

	void TOverlappedDataBuffer::RequeueAsEmpty()
	{
		m_pQueue->AddEmptyBuffer(this);
	}

	void TOverlappedDataBuffer::RequeueAsFull()
	{
		m_pQueue->AddFullBuffer(this);
	}

	void TOverlappedDataBuffer::RequeueAsFinished()
	{
		m_pQueue->AddFinishedBuffer(this);
	}

	void TOverlappedDataBuffer::InitForRead(unsigned long long ullPosition, DWORD dwRequestedSize)
	{
		SetRequestedDataSize(dwRequestedSize);
		SetFilePosition(ullPosition);
		SetRealDataSize(0);
		SetLastPart(false);
		SetErrorCode(ERROR_SUCCESS);
		SetStatusCode(0);
		SetBytesTransferred(0);
	}

	void TOverlappedDataBuffer::InitForWrite()
	{
		SetErrorCode(ERROR_SUCCESS);
		SetStatusCode(0);
		SetBytesTransferred(0);
	}

	void TOverlappedDataBuffer::Reset()
	{
		SetRequestedDataSize(0);
		SetFilePosition(0);
		SetRealDataSize(0);
		SetLastPart(false);
		SetErrorCode(ERROR_SUCCESS);
		SetStatusCode(0);
		SetBytesTransferred(0);
	}
}
