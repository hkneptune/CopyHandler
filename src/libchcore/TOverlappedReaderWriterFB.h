// ============================================================================
//  Copyright (C) 2001-2014 by Jozef Starosczyk
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
#ifndef __TOVERLAPPEDREADERWRITERFB_H__
#define __TOVERLAPPEDREADERWRITER_H__

#include "../liblogger/TLogFileData.h"
#include "../liblogger/TLogger.h"
#include "TOverlappedMemoryPool.h"
#include "TOverlappedReaderFB.h"
#include "TOverlappedWriterFB.h"

namespace chcore
{
	class TOverlappedThreadPool;

	class TOverlappedReaderWriterFB
	{
	public:
		explicit TOverlappedReaderWriterFB(const IFilesystemPtr& spFilesystem,
			const IFeedbackHandlerPtr& spFeedbackHandler,
			TWorkerThreadController& rThreadController,
			TOverlappedThreadPool& rThreadPool,
			const TFileInfoPtr& spSrcFileInfo,
			const TSmartPath& pathDst,
			const TSubTaskStatsInfoPtr& spStats,
			const logger::TLogFileDataPtr& spLogFileData,
			const TOverlappedMemoryPoolPtr& spBuffers,
			unsigned long long ullResumePosition,
			DWORD dwChunkSize,
			bool bNoBuffering,
			bool bProtectReadOnlyFiles,
			bool bOnlyCreate,
			bool bUpdateFileAttributesAndTimes);
		TOverlappedReaderWriterFB(const TOverlappedReaderWriterFB&) = delete;
		~TOverlappedReaderWriterFB();

		TOverlappedReaderWriterFB& operator=(const TOverlappedReaderWriterFB&) = delete;

		TSubTaskBase::ESubOperationResult Process();

	private:
		logger::TLoggerPtr m_spLog;
		TOverlappedThreadPool& m_rThreadPool;
		TWorkerThreadController& m_rThreadController;

		TOverlappedProcessorRangePtr m_spRange;

		TOverlappedMemoryPoolPtr m_spMemoryPool;
		TOverlappedReaderFBPtr m_spReader;
		TOverlappedWriterFBPtr m_spWriter;
	};
}

#endif
