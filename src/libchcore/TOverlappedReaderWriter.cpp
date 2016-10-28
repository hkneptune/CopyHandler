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
#include <atltrace.h>

namespace chcore
{
	TOverlappedReaderWriter::TOverlappedReaderWriter(const logger::TLogFileDataPtr& spLogFileData, const TOverlappedMemoryPoolPtr& spMemoryPool,
		unsigned long long ullFilePos, DWORD dwChunkSize) :
		m_spLog(logger::MakeLogger(spLogFileData, L"DataBuffer")),
		m_spMemoryPool(spMemoryPool),
		m_spReader(std::make_shared<TOverlappedReader>(spLogFileData, spMemoryPool->GetBufferList(), ullFilePos, dwChunkSize)),
		m_spWriter(std::make_shared<TOverlappedWriter>(spLogFileData, m_spReader->GetFinishedQueue(), ullFilePos, spMemoryPool->GetBufferList()))
	{
		if(!spMemoryPool)
			throw TCoreException(eErr_InvalidArgument, L"spMemoryPool", LOCATION);
	}

	TOverlappedReaderWriter::~TOverlappedReaderWriter()
	{
	}

	void TOverlappedReaderWriter::WaitForMissingBuffersAndResetState(HANDLE hKillEvent)
	{
		m_spReader->ReleaseBuffers();
		m_spWriter->ReleaseBuffers();

		enum { eKillThread = 0, eAllBuffersReturned, eHandleCount };
		std::array<HANDLE, eHandleCount> arrHandles = { hKillEvent, m_spMemoryPool->GetBufferList()->GetAllBuffersAccountedForEvent() };

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
	}
}
