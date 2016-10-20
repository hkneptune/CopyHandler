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
#include "OverlappedCallbacks.h"
#include "TOverlappedDataBuffer.h"
#include "TOverlappedReaderWriter.h"

#define STATUS_END_OF_FILE 0xc0000011

namespace chcore
{
	VOID CALLBACK OverlappedReadCompleted(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped)
	{
		TOverlappedDataBuffer* pBuffer = (TOverlappedDataBuffer*)lpOverlapped;
		TOverlappedReaderWriter* pQueue = (TOverlappedReaderWriter*)pBuffer->GetParam();

		// determine if this is the last packet
		bool bEof = (dwErrorCode == ERROR_HANDLE_EOF ||
			pBuffer->GetStatusCode() == STATUS_END_OF_FILE ||
			(dwErrorCode == ERROR_SUCCESS && dwNumberOfBytesTransfered != pBuffer->GetRequestedDataSize()));

		// reset status code and error code if they pointed out to EOF
		if(pBuffer->GetStatusCode() == STATUS_END_OF_FILE)
			pBuffer->SetStatusCode(0);

		pBuffer->SetErrorCode(dwErrorCode == ERROR_HANDLE_EOF ? ERROR_SUCCESS : dwErrorCode);

		pBuffer->SetRealDataSize(dwNumberOfBytesTransfered);
		pBuffer->SetLastPart(bEof);
		// NOTE: updating the bytes transferred as system does not update lpOverlapped with correct value
		// in case of error (e.g end-of-file error triggers the difference and dwNumberOfBytesTransfered contains more up-to-date information)
		pBuffer->SetBytesTransferred(dwNumberOfBytesTransfered);

		pQueue->AddFullBuffer(pBuffer);
	}

	VOID CALLBACK OverlappedWriteCompleted(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped)
	{
		TOverlappedDataBuffer* pBuffer = (TOverlappedDataBuffer*)lpOverlapped;
		TOverlappedReaderWriter* pQueue = (TOverlappedReaderWriter*)pBuffer->GetParam();

		pBuffer->SetErrorCode(dwErrorCode);
		pBuffer->SetBytesTransferred(dwNumberOfBytesTransfered);

		pQueue->AddFinishedBuffer(pBuffer);
	}
}
