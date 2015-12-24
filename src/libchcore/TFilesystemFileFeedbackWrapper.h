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

#include "libchcore.h"
#include "IFilesystemFile.h"
#include "..\libicpf\log.h"
#include "TSubTaskBase.h"
#include "IFeedbackHandler.h"
#include "IFilesystem.h"

namespace chcore
{
	class TWorkerThreadController;

	class TFilesystemFileFeedbackWrapper
	{
	public:
		TFilesystemFileFeedbackWrapper(const IFeedbackHandlerPtr& spFeedbackHandler, icpf::log_file& rLog, TWorkerThreadController& rThreadController);
		TFilesystemFileFeedbackWrapper& operator=(const TFilesystemFileFeedbackWrapper&) = delete;

		TSubTaskBase::ESubOperationResult OpenSourceFileFB(const IFilesystemFilePtr& fileSrc);
		TSubTaskBase::ESubOperationResult OpenExistingDestinationFileFB(const IFilesystemFilePtr& fileDst);
		TSubTaskBase::ESubOperationResult OpenDestinationFileFB(const IFilesystemFilePtr& fileDst, const TFileInfoPtr& spSrcFileInfo,
			unsigned long long& ullSeekTo, bool& bFreshlyCreated, bool& bSkip);

		TSubTaskBase::ESubOperationResult TruncateFileFB(const IFilesystemFilePtr& spFile, file_size_t fsNewSize,
			const TSmartPath& pathFile, bool& bSkip);

		TSubTaskBase::ESubOperationResult ReadFileFB(const IFilesystemFilePtr& spFile,
			TOverlappedDataBuffer& rBuffer, const TSmartPath& pathFile, bool& bSkip);
		TSubTaskBase::ESubOperationResult WriteFileFB(const IFilesystemFilePtr& spFile,
			TOverlappedDataBuffer& rBuffer, const TSmartPath& pathFile, bool& bSkip);

		TSubTaskBase::ESubOperationResult FinalizeFileFB(const IFilesystemFilePtr& spFile,
			TOverlappedDataBuffer& rBuffer, const TSmartPath& pathFile, bool& bSkip);

	private:
		bool WasKillRequested(const TFeedbackResult& rFeedbackResult) const;

	private:
		IFeedbackHandlerPtr m_spFeedbackHandler;
		IFilesystemPtr m_spFilesystem;
		icpf::log_file& m_rLog;
		TWorkerThreadController& m_rThreadController;
	};
}

#endif
