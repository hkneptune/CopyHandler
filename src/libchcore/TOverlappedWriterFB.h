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

namespace chcore
{
	class TOverlappedWriterFB
	{
	public:
		TOverlappedWriterFB(const TFilesystemFileFeedbackWrapperPtr& spSrcFile, const TFilesystemFileFeedbackWrapperPtr& spDstFile, const TSubTaskStatsInfoPtr& spStats,
			const TFileInfoPtr& spSrcFileInfo,
			const logger::TLogFileDataPtr& spLogFileData, const TOrderedBufferQueuePtr& spBuffersToWrite,
			unsigned long long ullFilePos, const TBufferListPtr& spEmptyBuffers);
		~TOverlappedWriterFB();

		TOverlappedWriterPtr GetWriter() const { return m_spWriter; }

		void SetReleaseMode() { m_bReleaseMode = true; }

		TSubTaskBase::ESubOperationResult OnWritePossible();
		TSubTaskBase::ESubOperationResult OnWriteFailed();
		TSubTaskBase::ESubOperationResult OnWriteFinished(bool& bStopProcessing);

	private:
		TSubTaskBase::ESubOperationResult AdjustProcessedSize(file_size_t fsWritten);
		TSubTaskBase::ESubOperationResult AdjustFinalSize();

	private:
		TOverlappedWriterPtr m_spWriter;
		TFilesystemFileFeedbackWrapperPtr m_spSrcFile;
		TFilesystemFileFeedbackWrapperPtr m_spDstFile;
		TSubTaskStatsInfoPtr m_spStats;
		TFileInfoPtr m_spSrcFileInfo;
		TFileInfoPtr m_spDstFileInfo;
		bool m_bReleaseMode = false;
	};

	using TOverlappedWriterFBPtr = std::shared_ptr<TOverlappedWriterFB>;
}

#endif
