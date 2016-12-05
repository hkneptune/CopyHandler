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
#include "TOverlappedDataBuffer.h"
#include "TCoreException.h"
#include "ErrorCodes.h"

namespace chcore
{
	bool CompareBufferPositions::operator()(const TOverlappedDataBuffer* pBufferA, const TOverlappedDataBuffer* pBufferB) const
	{
		if(!pBufferA)
			throw TCoreException(eErr_InvalidArgument, L"pBufferA", LOCATION);
		if(!pBufferB)
			throw TCoreException(eErr_InvalidArgument, L"pBufferB", LOCATION);

		return pBufferA->GetFilePosition() < pBufferB->GetFilePosition();
	}

	TOverlappedDataBuffer::TOverlappedDataBuffer(size_t stBufferSize, void* pParam) :
		m_pParam(pParam)
	{
		// initialize OVERLAPPED members
		Internal = 0;
		InternalHigh = 0;
		Offset = 0;
		OffsetHigh = 0;
		hEvent = nullptr;

		// create buffer
		ReinitializeBuffer(stBufferSize);
	}

	TOverlappedDataBuffer::~TOverlappedDataBuffer()
	{
		ReleaseBuffer();
	}

	void TOverlappedDataBuffer::ReinitializeBuffer(size_t stNewBufferSize)
	{
		if (stNewBufferSize == 0)
			throw TCoreException(eErr_InvalidArgument, L"Cannot create 0-sized buffer", LOCATION);

		if (stNewBufferSize != m_stBufferSize)
		{
			ReleaseBuffer();

			m_pBuffer = VirtualAlloc(nullptr, stNewBufferSize, MEM_COMMIT, PAGE_READWRITE);
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
		SetParam(nullptr);
	}
}
