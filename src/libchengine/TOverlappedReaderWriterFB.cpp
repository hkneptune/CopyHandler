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
#include "TOverlappedThreadPool.h"
#include "../libchcore/TCoreWin32Exception.h"

namespace chengine
{
	TOverlappedReaderWriterFB::TOverlappedReaderWriterFB(const IFilesystemPtr& spFilesystem,
		const FeedbackManagerPtr& spFeedbackManager,
		TWorkerThreadController& rThreadController,
		TOverlappedThreadPool& rThreadPool,
		const TFileInfoPtr& spSrcFileInfo,
		const TDestinationPathProvider& rDstPathProvider,
		const TSubTaskStatsInfoPtr& spStats,
		const logger::TLogFileDataPtr& spLogFileData,
		const TOverlappedMemoryPoolPtr& spMemoryPool,
		unsigned long long ullResumePosition,
		DWORD dwChunkSize,
		unsigned int uiMaxReadAhead,
		unsigned int uiMaxConcurrentReads,
		unsigned int uiMaxConcurrentWrites,
		bool bNoBuffering,
		bool bProtectReadOnlyFiles,
		bool bOnlyCreate,
		bool bUpdateFileAttributesAndTimes) :

		m_spLog(logger::MakeLogger(spLogFileData, L"DataBuffer")),
		m_rThreadPool(rThreadPool),
		m_rThreadController(rThreadController),
		m_spRange(std::make_shared<TOverlappedProcessorRange>(ullResumePosition)),
		m_spMemoryPool(spMemoryPool),
		m_spReader(std::make_shared<TOverlappedReaderFB>(spFilesystem, spFeedbackManager, rThreadController, spStats, spSrcFileInfo, spLogFileData, spMemoryPool ? spMemoryPool->GetBufferList() : TBufferListPtr(), m_spRange, dwChunkSize, uiMaxConcurrentReads, uiMaxReadAhead, bNoBuffering, bProtectReadOnlyFiles)),
		m_spWriter(std::make_shared<TOverlappedWriterFB>(spFilesystem, spFeedbackManager, rThreadController, spStats, spSrcFileInfo, rDstPathProvider, spLogFileData, m_spReader->GetFinishedQueue(), m_spRange, spMemoryPool ? spMemoryPool->GetBufferList() : TBufferListPtr(), uiMaxConcurrentWrites, bOnlyCreate, bNoBuffering, bProtectReadOnlyFiles, bUpdateFileAttributesAndTimes))
	{
	}

	TOverlappedReaderWriterFB::~TOverlappedReaderWriterFB()
	{
	}

	TSubTaskBase::ESubOperationResult TOverlappedReaderWriterFB::Process()
	{
		TSubTaskBase::ESubOperationResult eResult = m_spReader->Start();
		if(eResult != TSubTaskBase::eSubResult_Continue)
			return eResult;

		eResult = m_spWriter->Start();
		if(eResult != TSubTaskBase::eSubResult_Continue)
			return eResult;

		m_rThreadPool.QueueRead(m_spReader);
		m_rThreadPool.QueueWrite(m_spWriter);

		// read data from file to buffer
		// NOTE: order is critical here:
		// - write finished is first, so that all the data that were already queued to be written, will be written and accounted for (in stats)
		// - kill request is second, so that we can stop processing as soon as all the data is written to destination location;
		//      that also means that we don't want to queue reads or writes anymore - all the data that were read until now, will be lost
		// - write possible - we're prioritizing write queuing here to empty buffers as soon as possible
		// - read possible - lowest priority - if we don't have anything to write or finalize , then read another part of source data
		enum
		{
			eReadingFinished, eWritingFinished
		};

		TEvent unsignaledEvent(true, false);

		std::vector<HANDLE> vHandles = {
			m_spReader->GetEventProcessingFinishedHandle(),
			m_spWriter->GetEventProcessingFinishedHandle()
		};

		bool bStopProcessing = false;
		while(!bStopProcessing && eResult == TSubTaskBase::eSubResult_Continue)
		{
			DWORD dwResult = WaitForMultipleObjectsEx(boost::numeric_cast<DWORD>(vHandles.size()), vHandles.data(), false, INFINITE, FALSE);
			switch(dwResult)
			{
			case WAIT_OBJECT_0 + eWritingFinished:
				eResult = m_spWriter->StopThreaded();
				vHandles[eWritingFinished] = unsignaledEvent.Handle();
				bStopProcessing = true;
				break;

			case WAIT_OBJECT_0 + eReadingFinished:
				eResult = m_spReader->StopThreaded();
				vHandles[eReadingFinished] = unsignaledEvent.Handle();
				break;

			default:
				DWORD dwLastError = GetLastError();
				throw TCoreWin32Exception(eErr_UnhandledCase, dwLastError, L"Unknown result from async waiting function", LOCATION);
			}
		}

		// ensure both reader and writer are correctly stopped
		m_spReader->StopThreaded();
		m_spWriter->StopThreaded();

		// get rid of reader and writer - mostly to release the buffers being used
		m_spReader.reset();
		m_spWriter.reset();

		// ensure that no buffer was lost in the process
		if(!m_spMemoryPool->GetBufferList()->AreAllBuffersAccountedFor())
			throw TCoreException(eErr_InternalProblem, L"Some buffers were lost in action", LOCATION);

		return eResult;
	}
}
