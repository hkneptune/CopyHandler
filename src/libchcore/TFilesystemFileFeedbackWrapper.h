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

	class TFilesystemFileFeedbackWrapper
	{
	public:
		TFilesystemFileFeedbackWrapper(const IFilesystemFilePtr& spFile, const IFeedbackHandlerPtr& spFeedbackHandler,
			const logger::TLogFileDataPtr& spLogFileData, TWorkerThreadController& rThreadController,
			const IFilesystemPtr& spFilesystem);
		TFilesystemFileFeedbackWrapper& operator=(const TFilesystemFileFeedbackWrapper&) = delete;

		TSubTaskBase::ESubOperationResult OpenSourceFileFB();
		TSubTaskBase::ESubOperationResult OpenExistingDestinationFileFB(bool bProtectReadOnlyFiles);
		TSubTaskBase::ESubOperationResult OpenDestinationFileFB(const TFileInfoPtr& spSrcFileInfo,
			unsigned long long& ullSeekTo, bool& bFreshlyCreated, bool& bSkip, bool bProtectReadOnlyFiles);

		TSubTaskBase::ESubOperationResult TruncateFileFB(file_size_t fsNewSize,
			const TSmartPath& pathFile, bool& bSkip);

		TSubTaskBase::ESubOperationResult ReadFileFB(TOverlappedDataBuffer& rBuffer, const TSmartPath& pathFile, bool& bSkip);
		TSubTaskBase::ESubOperationResult WriteFileFB(TOverlappedDataBuffer& rBuffer, const TSmartPath& pathFile, bool& bSkip);

		TSubTaskBase::ESubOperationResult FinalizeFileFB(TOverlappedDataBuffer& rBuffer, const TSmartPath& pathFile, bool& bSkip);

		TSmartPath GetFilePath() const { return m_spFile->GetFilePath(); }
		file_size_t GetFileSize() const { return m_spFile->GetFileSize(); }
		file_size_t GetSeekPositionForResume(file_size_t fsLastAvailablePosition) { return m_spFile->GetSeekPositionForResume(fsLastAvailablePosition); }

		bool IsOpen() const { return m_spFile->IsOpen(); }

	private:
		bool WasKillRequested(const TFeedbackResult& rFeedbackResult) const;

	private:
		IFilesystemFilePtr m_spFile;
		IFeedbackHandlerPtr m_spFeedbackHandler;
		IFilesystemPtr m_spFilesystem;
		logger::TLoggerPtr m_spLog;
		TWorkerThreadController& m_rThreadController;
	};
}

#endif
