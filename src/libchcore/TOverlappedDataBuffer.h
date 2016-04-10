// ============================================================================
//  Copyright (C) 2001-2012 by Jozef Starosczyk
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
/// @file  TDataBuffer.h
/// @date  2012/03/04
/// @brief Contains class representing buffer for data.
// ============================================================================
#ifndef __TDATABUFFER_H__
#define __TDATABUFFER_H__

#include "libchcore.h"

namespace chcore
{
	class IOverlappedDataBufferQueue;

	VOID CALLBACK OverlappedReadCompleted(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped);
	VOID CALLBACK OverlappedWriteCompleted(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped);

	class TOverlappedDataBuffer : public OVERLAPPED
	{
	public:
		// construction/destruction
		TOverlappedDataBuffer(size_t stBufferSize, IOverlappedDataBufferQueue* pQueue);
		TOverlappedDataBuffer(const TOverlappedDataBuffer&) = delete;
		TOverlappedDataBuffer(TOverlappedDataBuffer&& rSrc) = delete;

		~TOverlappedDataBuffer();

		// operators
		TOverlappedDataBuffer& operator=(const TOverlappedDataBuffer&) = delete;
		TOverlappedDataBuffer& operator=(TOverlappedDataBuffer&& rSrc) = delete;

		// interface methods
		// buffer size management
		void ReinitializeBuffer(size_t stNewBufferSize);
		LPVOID GetBufferPtr();

		size_t GetBufferSize() const { return m_stBufferSize; }

		// members
		DWORD GetRequestedDataSize() const { return m_dwRequestedDataSize; }
		void SetRequestedDataSize(DWORD dwRequestedSize) { m_dwRequestedDataSize = dwRequestedSize; }

		DWORD GetRealDataSize() const { return m_dwRealDataSize; }
		void SetRealDataSize(DWORD dwRealDataSize) { m_dwRealDataSize = dwRealDataSize; }

		void SetLastPart(bool bLastPart) { m_bLastPart = bLastPart; }
		bool IsLastPart() const { return m_bLastPart; }

		unsigned long long GetBufferOrder() const { return m_ullBufferOrder; }
		void SetBufferOrder(unsigned long long ullOrder) { m_ullBufferOrder = ullOrder; }

		DWORD GetErrorCode() const { return m_dwErrorCode; }
		void SetErrorCode(DWORD dwErrorCode) { m_dwErrorCode = dwErrorCode; }

		// OVERLAPPED interface
		ULONG_PTR GetStatusCode() const { return Internal; }
		void SetStatusCode(ULONG_PTR ulStatusCode) { Internal = ulStatusCode; }

		void SetBytesTransferred(ULONG_PTR ulBytes) { InternalHigh = ulBytes; }
		ULONG_PTR GetBytesTransferred() const { return InternalHigh; }

		unsigned long long GetFilePosition() const { return (unsigned long long)OffsetHigh << 32 | Offset; }
		void SetFilePosition(unsigned long long ullPosition) { OffsetHigh = (DWORD)(ullPosition >> 32); Offset = (DWORD)ullPosition; }

		// queue management
		void RequeueAsEmpty();
		void RequeueAsFull();
		void RequeueAsFinished();

		// composite initialization
		void InitForRead(unsigned long long ullPosition, DWORD dwRequestedSize);
		void InitForWrite();
		void Reset();

	private:
		void ReleaseBuffer();

	private:
		LPVOID m_pBuffer = nullptr;				// pointer to the allocated buffer
		size_t m_stBufferSize = 0;			// total buffer size
		DWORD m_dwRequestedDataSize = 0;	// part of the buffer that is to be used for data transfer (<= m_stBufferSize)
		DWORD m_dwRealDataSize = 0;			// data size as reported by read operation
		DWORD m_dwErrorCode = 0;			// win32 error code
		bool m_bLastPart = false;				// marks the last part of the file
		unsigned long long m_ullBufferOrder = 0;	// marks the order of this buffer

		IOverlappedDataBufferQueue* m_pQueue = nullptr;	// pointer to the queue where this object resides
	};
}

#endif
