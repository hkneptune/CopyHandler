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
#include "TOverlappedReaderWriterFB.h"
#include "TCoreException.h"
#include "ErrorCodes.h"
#include <array>
#include <atltrace.h>

namespace chcore
{
	TOverlappedReaderWriterFB::TOverlappedReaderWriterFB(const TFilesystemFileFeedbackWrapperPtr& spSrcFile, const TFileInfoPtr& spSrcFileInfo,
		const TFilesystemFileFeedbackWrapperPtr& spDstFile,
		const TSubTaskStatsInfoPtr& spStats,
		const logger::TLogFileDataPtr& spLogFileData, const TOverlappedMemoryPoolPtr& spMemoryPool, unsigned long long ullFilePos, DWORD dwChunkSize) :

		m_spLog(logger::MakeLogger(spLogFileData, L"DataBuffer")),
		m_spMemoryPool(spMemoryPool),
		m_spReader(std::make_shared<TOverlappedReaderFB>(spSrcFile, spStats, spSrcFileInfo, spLogFileData, spMemoryPool->GetBufferList(), ullFilePos, dwChunkSize)),
		m_spWriter(std::make_shared<TOverlappedWriterFB>(spSrcFile, spDstFile, spStats, spSrcFileInfo, spLogFileData, m_spReader->GetReader()->GetFinishedQueue(), ullFilePos, spMemoryPool->GetBufferList()))
	{
		if(!spMemoryPool)
			throw TCoreException(eErr_InvalidArgument, L"spMemoryPool", LOCATION);
	}

	TOverlappedReaderWriterFB::~TOverlappedReaderWriterFB()
	{
	}

	TSubTaskBase::ESubOperationResult TOverlappedReaderWriterFB::WaitForMissingBuffersAndResetState(bool& bProcessed)
	{
		m_spReader->SetReleaseMode();
		m_spWriter->SetReleaseMode();

		enum
		{
			eAllBuffersAccountedFor, eWriteFinished, eWriteFailed, eWritePossible, eHandleCount
		};
		std::array<HANDLE, eHandleCount> arrHandles = {
			m_spMemoryPool->GetBufferList()->GetAllBuffersAccountedForEvent(),
			m_spWriter->GetWriter()->GetEventWriteFinishedHandle(),
			m_spWriter->GetWriter()->GetEventWriteFailedHandle(),
			m_spWriter->GetWriter()->GetEventWritePossibleHandle()
		};

		TSubTaskBase::ESubOperationResult eResult = TSubTaskBase::eSubResult_Continue;
		bool bStopProcessing = false;
		while(!bStopProcessing)
		{
			bool bIgnoreStop = false;

			DWORD dwResult = WaitForMultipleObjectsEx(eHandleCount, arrHandles.data(), false, INFINITE, true);
			switch(dwResult)
			{
			case STATUS_USER_APC:
				break;

			case WAIT_OBJECT_0 + eAllBuffersAccountedFor:
			{
				LOG_DEBUG(m_spLog) << L"All buffer accounted for.";

				eResult = TSubTaskBase::eSubResult_KillRequest;
				bStopProcessing = true;
				break;
			}

			case WAIT_OBJECT_0 + eWritePossible:
			{
				eResult = m_spWriter->OnWritePossible(bIgnoreStop, bProcessed);
				break;
			}

			case WAIT_OBJECT_0 + eWriteFailed:
			{
				eResult = m_spWriter->OnWriteFailed(bIgnoreStop, bProcessed);
				break;
			}

			case WAIT_OBJECT_0 + eWriteFinished:
			{
				eResult = m_spWriter->OnWriteFinished(bIgnoreStop, bProcessed);
				break;
			}

			default:
				throw TCoreException(eErr_UnhandledCase, L"Unknown result from async waiting function", LOCATION);
			}
		}

		return eResult;
	}

	TSubTaskBase::ESubOperationResult TOverlappedReaderWriterFB::Start(HANDLE hKill, bool& bProcessed)
	{
		// read data from file to buffer
		// NOTE: order is critical here:
		// - write finished is first, so that all the data that were already queued to be written, will be written and accounted for (in stats)
		// - kill request is second, so that we can stop processing as soon as all the data is written to destination location;
		//      that also means that we don't want to queue reads or writes anymore - all the data that were read until now, will be lost
		// - write possible - we're prioritizing write queuing here to empty buffers as soon as possible
		// - read possible - lowest priority - if we don't have anything to write or finalize , then read another part of source data
		enum
		{
			eKillThread, eWriteFinished, eWriteFailed, eWritePossible, eReadFailed, eReadPossible, eHandleCount
		};
		std::array<HANDLE, eHandleCount> arrHandles = {
			hKill,
			m_spWriter->GetWriter()->GetEventWriteFinishedHandle(),
			m_spWriter->GetWriter()->GetEventWriteFailedHandle(),
			m_spWriter->GetWriter()->GetEventWritePossibleHandle(),
			m_spReader->GetReader()->GetEventReadFailedHandle(),
			m_spReader->GetReader()->GetEventReadPossibleHandle()
		};

		TSubTaskBase::ESubOperationResult eResult = TSubTaskBase::eSubResult_Continue;
		bool bStopProcessing = false;
		while(!bStopProcessing)
		{
			DWORD dwResult = WaitForMultipleObjectsEx(eHandleCount, arrHandles.data(), false, INFINITE, true);
			switch(dwResult)
			{
			case STATUS_USER_APC:
				break;

			case WAIT_OBJECT_0 + eKillThread:
			{
				// log
				LOG_INFO(m_spLog) << L"Received kill request while copying file";

				eResult = TSubTaskBase::eSubResult_KillRequest;
				bStopProcessing = true;
				break;
			}

			case WAIT_OBJECT_0 + eReadPossible:
			{
				eResult = m_spReader->OnReadPossible(bStopProcessing, bProcessed);
				break;
			}
			case WAIT_OBJECT_0 + eReadFailed:
			{
				eResult = m_spReader->OnReadFailed(bStopProcessing, bProcessed);
				break;
			}
			case WAIT_OBJECT_0 + eWritePossible:
			{
				eResult = m_spWriter->OnWritePossible(bStopProcessing, bProcessed);
				break;
			}

			case WAIT_OBJECT_0 + eWriteFailed:
			{
				eResult = m_spWriter->OnWriteFailed(bStopProcessing, bProcessed);
				break;
			}

			case WAIT_OBJECT_0 + eWriteFinished:
			{
				eResult = m_spWriter->OnWriteFinished(bStopProcessing, bProcessed);
				break;
			}

			default:
				throw TCoreException(eErr_UnhandledCase, L"Unknown result from async waiting function", LOCATION);
			}
		}

		WaitForMissingBuffersAndResetState(bProcessed);

		return eResult;
	}
}
