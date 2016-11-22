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
#include "TWorkerThreadController.h"
#include "TOverlappedThreadPool.h"
#include "TCoreWin32Exception.h"

namespace chcore
{
	TOverlappedReaderWriterFB::TOverlappedReaderWriterFB(const IFilesystemPtr& spFilesystem,
		const IFeedbackHandlerPtr& spFeedbackHandler,
		TWorkerThreadController& rThreadController,
		TOverlappedThreadPool& rThreadPool,
		const TFileInfoPtr& spSrcFileInfo,
		const TSmartPath& pathDst,
		const TSubTaskStatsInfoPtr& spStats,
		const logger::TLogFileDataPtr& spLogFileData,
		const TOverlappedMemoryPoolPtr& spMemoryPool,
		unsigned long long ullResumePosition,
		DWORD dwChunkSize,
		bool bNoBuffering,
		bool bProtectReadOnlyFiles,
		bool bOnlyCreate) :

		m_spLog(logger::MakeLogger(spLogFileData, L"DataBuffer")),
		m_rThreadPool(rThreadPool),
		m_rThreadController(rThreadController),
		m_spRange(std::make_shared<TOverlappedProcessorRange>(ullResumePosition)),
		m_spMemoryPool(spMemoryPool),
		m_spReader(std::make_shared<TOverlappedReaderFB>(spFilesystem, spFeedbackHandler, rThreadController, spStats, spSrcFileInfo, spLogFileData, spMemoryPool->GetBufferList(), m_spRange, dwChunkSize, bNoBuffering, bProtectReadOnlyFiles)),
		m_spWriter(std::make_shared<TOverlappedWriterFB>(spFilesystem, spFeedbackHandler, rThreadController, spStats, spSrcFileInfo, pathDst, spLogFileData, m_spReader->GetFinishedQueue(), m_spRange, spMemoryPool->GetBufferList(), bOnlyCreate, bNoBuffering, bProtectReadOnlyFiles))
	{
		if(!spMemoryPool)
			throw TCoreException(eErr_InvalidArgument, L"spMemoryPool", LOCATION);
	}

	TOverlappedReaderWriterFB::~TOverlappedReaderWriterFB()
	{
	}

	TSubTaskBase::ESubOperationResult TOverlappedReaderWriterFB::WaitForMissingBuffersAndResetState()
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
				m_spWriter->OnWritePossible();
				break;
			}

			case WAIT_OBJECT_0 + eWriteFailed:
			{
				m_spWriter->OnWriteFailed();
				break;
			}

			case WAIT_OBJECT_0 + eWriteFinished:
			{
				bool bIgnoreStop = false;
				m_spWriter->OnWriteFinished(bIgnoreStop);
				break;
			}

			default:
				throw TCoreException(eErr_UnhandledCase, L"Unknown result from async waiting function", LOCATION);
			}
		}

		return eResult;
	}

	TSubTaskBase::ESubOperationResult TOverlappedReaderWriterFB::Start()
	{
		TSubTaskBase::ESubOperationResult eResult = m_spReader->Start();
		if(eResult != TSubTaskBase::eSubResult_Continue)
			return eResult;

		eResult = m_spWriter->Start();
		if(eResult != TSubTaskBase::eSubResult_Continue)
			return eResult;

		m_rThreadPool.QueueRead(m_spReader);

		// read data from file to buffer
		// NOTE: order is critical here:
		// - write finished is first, so that all the data that were already queued to be written, will be written and accounted for (in stats)
		// - kill request is second, so that we can stop processing as soon as all the data is written to destination location;
		//      that also means that we don't want to queue reads or writes anymore - all the data that were read until now, will be lost
		// - write possible - we're prioritizing write queuing here to empty buffers as soon as possible
		// - read possible - lowest priority - if we don't have anything to write or finalize , then read another part of source data
		enum
		{
			eKillThread, eDataSourceFinished, eWriteFinished, eWriteFailed, eWritePossible
		};

		TEvent unsignaledEvent(true, false);

		std::vector<HANDLE> vHandles = {
			m_rThreadController.GetKillThreadHandle(),
			m_spReader->GetEventDataSourceFinishedHandle(),
			m_spWriter->GetWriter()->GetEventWriteFinishedHandle(),
			m_spWriter->GetWriter()->GetEventWriteFailedHandle(),
			m_spWriter->GetWriter()->GetEventWritePossibleHandle()
		};

		bool bStopProcessing = false;
		while(!bStopProcessing && eResult == TSubTaskBase::eSubResult_Continue)
		{
			DWORD dwResult = WaitForMultipleObjectsEx(boost::numeric_cast<DWORD>(vHandles.size()), vHandles.data(), false, INFINITE, true);
			switch(dwResult)
			{
			case STATUS_USER_APC:
				break;

			case WAIT_OBJECT_0 + eKillThread:
				// log
				LOG_INFO(m_spLog) << L"Received kill request while copying file";

				eResult = TSubTaskBase::eSubResult_KillRequest;
				bStopProcessing = true;
				break;

			case WAIT_OBJECT_0 + eWritePossible:
				eResult = m_spWriter->OnWritePossible();
				break;

			case WAIT_OBJECT_0 + eWriteFailed:
				eResult = m_spWriter->OnWriteFailed();
				break;

			case WAIT_OBJECT_0 + eWriteFinished:
				eResult = m_spWriter->OnWriteFinished(bStopProcessing);
				break;

			case WAIT_OBJECT_0 + eDataSourceFinished:
				eResult = m_spReader->StopThreaded();
				vHandles[eDataSourceFinished] = unsignaledEvent.Handle();
				break;

			default:
				DWORD dwLastError = GetLastError();
				throw TCoreWin32Exception(eErr_UnhandledCase, dwLastError, L"Unknown result from async waiting function", LOCATION);
			}
		}

		WaitForMissingBuffersAndResetState();

		return eResult;
	}
}
