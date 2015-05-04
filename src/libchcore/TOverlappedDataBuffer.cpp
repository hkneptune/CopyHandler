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

#define STATUS_END_OF_FILE 0xc0000011

BEGIN_CHCORE_NAMESPACE

///////////////////////////////////////////////////////////////////////////////////
// class TOverlappedDataBuffer
VOID CALLBACK OverlappedReadCompleted(DWORD dwErrorCode, DWORD /*dwNumberOfBytesTransfered*/, LPOVERLAPPED lpOverlapped)
{
	TOverlappedDataBuffer* pBuffer = (TOverlappedDataBuffer*) lpOverlapped;

	bool bEof = (dwErrorCode == ERROR_HANDLE_EOF ||
		pBuffer->GetStatusCode() == STATUS_END_OF_FILE ||
		(dwErrorCode == ERROR_SUCCESS && pBuffer->GetBytesTransferred() != pBuffer->GetRequestedDataSize()));

	if (pBuffer->GetStatusCode() == STATUS_END_OF_FILE)
		pBuffer->SetStatusCode(0);
	pBuffer->SetErrorCode(dwErrorCode == ERROR_HANDLE_EOF ? ERROR_SUCCESS : dwErrorCode);

	if (dwErrorCode != ERROR_SUCCESS)
		ATLTRACE(_T("OverlappedReadCompleted error: %lu, status code: %I64u\n"), dwErrorCode, pBuffer->GetStatusCode());

	pBuffer->SetLastPart(bEof);
	pBuffer->RequeueAsFull();
}

VOID CALLBACK OverlappedWriteCompleted(DWORD dwErrorCode, DWORD /*dwNumberOfBytesTransfered*/, LPOVERLAPPED lpOverlapped)
{
	TOverlappedDataBuffer* pBuffer = (TOverlappedDataBuffer*) lpOverlapped;
	if (dwErrorCode != ERROR_SUCCESS)
		ATLTRACE(_T("OverlappedWriteCompleted error: %lu, status code: %I64u\n"), dwErrorCode, pBuffer->GetStatusCode());

	pBuffer->RequeueAsFinished();
}

TOverlappedDataBuffer::TOverlappedDataBuffer(size_t stBufferSize, IOverlappedDataBufferQueue* pQueue) :
	m_pBuffer(NULL),
	m_stBufferSize(0),
	m_bLastPart(false),
	m_pQueue(pQueue),
	m_dwRequestedDataSize(0)
{
	if (!m_pQueue)
		THROW_CORE_EXCEPTION(eErr_InvalidPointer);

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
	if (stNewBufferSize > m_stBufferSize)
	{
		ReleaseBuffer();

		m_pBuffer = VirtualAlloc(NULL, stNewBufferSize, MEM_COMMIT, PAGE_READWRITE);
		if (!m_pBuffer)
			THROW_CORE_EXCEPTION(eErr_CannotAllocateMemory);
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

END_CHCORE_NAMESPACE
