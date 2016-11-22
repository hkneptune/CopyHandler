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
#ifndef __TOVERLAPPEDREADERFB_H__
#define __TOVERLAPPEDREADERFB_H__

#include "TOverlappedReader.h"
#include "TFilesystemFileFeedbackWrapper.h"
#include "TOverlappedProcessorRange.h"
#include <boost/thread/thread.hpp>
#include "TThreadedQueueRunner.h"

namespace chcore
{
	class TFilesystemFileFeedbackWrapper;

	class TOverlappedReaderFB
	{
	public:
		TOverlappedReaderFB(const IFilesystemPtr& spFilesystem,
			const IFeedbackHandlerPtr& spFeedbackHandler,
			TWorkerThreadController& rThreadController,
			const TSubTaskStatsInfoPtr& spStats,
			const TFileInfoPtr& spSrcFileInfo,
			const logger::TLogFileDataPtr& spLogFileData,
			const TBufferListPtr& spEmptyBuffers,
			const TOverlappedProcessorRangePtr& spDataRange,
			DWORD dwChunkSize,
			bool bNoBuffering,
			bool bProtectReadOnlyFiles);
		TOverlappedReaderFB(const TOverlappedReaderFB& rSrc) = delete;

		~TOverlappedReaderFB();

		TOverlappedReaderFB& operator=(const TOverlappedReaderFB& rSrc) = delete;

		TSubTaskBase::ESubOperationResult Start();
		
		void StartThreaded();
		TSubTaskBase::ESubOperationResult StopThreaded();

		TOrderedBufferQueuePtr GetFinishedQueue() const;
		void SetReleaseMode();

		HANDLE GetEventDataSourceFinishedHandle() const;

	private:
		TSubTaskBase::ESubOperationResult UpdateFileStats();

		TSubTaskBase::ESubOperationResult OnReadPossible();
		TSubTaskBase::ESubOperationResult OnReadFailed();

		TOverlappedReaderPtr GetReader() const;

	private:
		TOverlappedReaderPtr m_spReader;
		TEvent m_eventDataSourceFinished;

		IFilesystemPtr m_spFilesystem;
		TFilesystemFileFeedbackWrapperPtr m_spSrcFile;
		TSubTaskStatsInfoPtr m_spStats;
		TFileInfoPtr m_spSrcFileInfo;

		TWorkerThreadController& m_rThreadController;
		TSubTaskBase::ESubOperationResult m_eThreadResult = TSubTaskBase::eSubResult_Continue;
	};

	using TOverlappedReaderFBPtr = std::shared_ptr<TOverlappedReaderFB>;
}

#endif
