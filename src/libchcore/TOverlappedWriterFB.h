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
#ifndef __TOVERLAPPEDWRITERFB_H__
#define __TOVERLAPPEDWRITERFB_H__

#include "TOverlappedWriter.h"
#include "TFilesystemFileFeedbackWrapper.h"
#include "TOverlappedProcessorRange.h"
#include "TThreadedQueueRunner.h"
#include "TEventCounter.h"

namespace chcore
{
	class TOverlappedWriterFB
	{
	public:
		TOverlappedWriterFB(const IFilesystemPtr& spFilesystem,
			const IFeedbackHandlerPtr& spFeedbackHandler,
			TWorkerThreadController& rThreadController,
			const TSubTaskStatsInfoPtr& spStats,
			const TFileInfoPtr& spSrcFileInfo,
			const TSmartPath& pathDst,
			const logger::TLogFileDataPtr& spLogFileData,
			const TOrderedBufferQueuePtr& spBuffersToWrite,
			const TOverlappedProcessorRangePtr& spRange,
			const TBufferListPtr& spEmptyBuffers,
			bool bOnlyCreate,
			bool bNoBuffering,
			bool bProtectReadOnlyFiles,
			bool bUpdateFileAttributesAndTimes);

		TOverlappedWriterFB(const TOverlappedWriterFB& rSrc) = delete;
		~TOverlappedWriterFB();

		TOverlappedWriterFB& operator=(const TOverlappedWriterFB& rSrc) = delete;

		TSubTaskBase::ESubOperationResult Start();

		void StartThreaded();

		TSubTaskBase::ESubOperationResult StopThreaded();
		bool WereAttributesAndTimesSet() const;

		HANDLE GetEventWritingFinishedHandle() const;
		HANDLE GetEventProcessingFinishedHandle() const;

		void QueueProcessedBuffer(TOverlappedDataBuffer* pBuffer);

	private:
		void AdjustProcessedSize(file_size_t fsWritten);
		TSubTaskBase::ESubOperationResult AdjustFinalSize();
		void WaitForOnTheFlyBuffers();
		void ClearBuffers();

		TSubTaskBase::ESubOperationResult OnWritePossible();
		TSubTaskBase::ESubOperationResult OnWriteFailed();
		TSubTaskBase::ESubOperationResult OnWriteFinished(bool& bStopProcessing);

		void UpdateCurrentItemStatsFromFileSize(bool bFileWritingFinished);

	private:
		TOverlappedWriterPtr m_spWriter;
		TFilesystemFileFeedbackWrapperPtr m_spDstFile;
		TSubTaskStatsInfoPtr m_spStats;
		TFileInfoPtr m_spSrcFileInfo;
		TOverlappedProcessorRangePtr m_spDataRange;
		bool m_bOnlyCreate = false;
		bool m_bUpdateFileAttributesAndTimes = false;

		TEvent m_eventProcessingFinished;
		TEvent m_eventWritingFinished;
		TEvent m_eventLocalKill;

		TEventCounter<unsigned int, EEventCounterMode::eSetIfEqual, 0> m_counterOnTheFly;

		TWorkerThreadController& m_rThreadController;
		TSubTaskBase::ESubOperationResult m_eThreadResult = TSubTaskBase::eSubResult_Continue;

		logger::TLoggerPtr m_spLog;
	};

	using TOverlappedWriterFBPtr = std::shared_ptr<TOverlappedWriterFB>;
}

#endif
