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
#ifndef __TFILESYSTEMFILEFEEDBACKWRAPPER_H__
#define __TFILESYSTEMFILEFEEDBACKWRAPPER_H__

#include "IFilesystemFile.h"
#include "TSubTaskBase.h"
#include "IFeedbackHandler.h"
#include "IFilesystem.h"
#include "../liblogger/TLogger.h"

namespace chcore
{
	class TWorkerThreadController;
}

namespace chengine
{
	class TFilesystemFileFeedbackWrapper
	{
	public:
		TFilesystemFileFeedbackWrapper(const IFilesystemFilePtr& spFile, const IFeedbackHandlerPtr& spFeedbackHandler,
			const logger::TLogFileDataPtr& spLogFileData, chcore::TWorkerThreadController& rThreadController,
			const IFilesystemPtr& spFilesystem);
		TFilesystemFileFeedbackWrapper& operator=(const TFilesystemFileFeedbackWrapper&) = delete;

		TSubTaskBase::ESubOperationResult HandleFileAlreadyExistsFB(const TFileInfoPtr& spSrcFileInfo, bool& bShouldAppend);

		TSubTaskBase::ESubOperationResult TruncateFileFB(file_size_t fsNewSize);

		TSubTaskBase::ESubOperationResult ReadFileFB(TOverlappedDataBuffer& rBuffer);
		TSubTaskBase::ESubOperationResult WriteFileFB(TOverlappedDataBuffer& rBuffer);

		TSubTaskBase::ESubOperationResult FinalizeFileFB(TOverlappedDataBuffer& rBuffer);

		TSubTaskBase::ESubOperationResult CancelIo();

		TSubTaskBase::ESubOperationResult HandleReadError(TOverlappedDataBuffer& rBuffer);
		TSubTaskBase::ESubOperationResult HandleWriteError(TOverlappedDataBuffer& rBuffer);

		TSubTaskBase::ESubOperationResult IsFreshlyCreated(bool& bIsFreshlyCreated) const;

		chcore::TSmartPath GetFilePath() const;
		TSubTaskBase::ESubOperationResult GetFileSize(file_size_t& fsSize, bool bSilent = false) const;
		file_size_t GetSeekPositionForResume(file_size_t fsLastAvailablePosition);

		TSubTaskBase::ESubOperationResult SetBasicInfo(DWORD dwAttributes, const chcore::TFileTime& ftCreationTime, const chcore::TFileTime& ftLastAccessTime, const chcore::TFileTime& ftLastWriteTime);

		bool IsOpen() const { return m_spFile->IsOpen(); }
		void Close() { m_spFile->Close(); }

	private:
		bool WasKillRequested(const TFeedbackResult& rFeedbackResult) const;

	private:
		IFilesystemFilePtr m_spFile;
		IFeedbackHandlerPtr m_spFeedbackHandler;
		IFilesystemPtr m_spFilesystem;
		logger::TLoggerPtr m_spLog;
		chcore::TWorkerThreadController& m_rThreadController;
	};

	using TFilesystemFileFeedbackWrapperPtr = std::shared_ptr<TFilesystemFileFeedbackWrapper>;
}

#endif
