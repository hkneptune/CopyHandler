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
#ifndef __TFILESYSTEMFEEDBACKWRAPPER_H__
#define __TFILESYSTEMFEEDBACKWRAPPER_H__

#include "IFilesystem.h"
#include "TSubTaskBase.h"
#include "../liblogger/TLogger.h"
#include "FeedbackManager.h"

namespace chcore {
	class TWorkerThreadController;
}

namespace chengine
{
	class TFilesystemFeedbackWrapper
	{
	public:
		TFilesystemFeedbackWrapper(const FeedbackManagerPtr& spFeedbackManager, const IFilesystemPtr& spFilesystem,
			const logger::TLogFileDataPtr& spLogFileData, chcore::TWorkerThreadController& rThreadController);
		TFilesystemFeedbackWrapper& operator=(const TFilesystemFeedbackWrapper&) = delete;

		TSubTaskBase::ESubOperationResult CreateDirectoryFB(const chcore::TSmartPath& pathDirectory);
		TSubTaskBase::ESubOperationResult CheckForFreeSpaceFB(const chcore::TSmartPath& pathFirstSrc, const chcore::TSmartPath& pathDestination,
			unsigned long long ullNeededSize);

		TSubTaskBase::ESubOperationResult RemoveDirectoryFB(const TFileInfoPtr& spFileInfo, bool bProtectReadOnlyFiles);
		TSubTaskBase::ESubOperationResult DeleteFileFB(const TFileInfoPtr& spFileInfo, bool bProtectReadOnlyFiles);

		TSubTaskBase::ESubOperationResult FastMoveFB(const TFileInfoPtr& spFileInfo, const chcore::TSmartPath& pathDestination,
			const TBasePathDataPtr& spBasePath);

		TSubTaskBase::ESubOperationResult GetFileInfoFB(const chcore::TSmartPath& pathCurrent,
			TFileInfoPtr& spFileInfo, const TBasePathDataPtr& spBasePath);

		TSubTaskBase::ESubOperationResult SetFileDirBasicInfo(const chcore::TSmartPath& pathFileDir, DWORD dwAttributes, const chcore::TFileTime& ftCreationTime,
			const chcore::TFileTime& ftLastAccessTime, const chcore::TFileTime& ftLastWriteTime);

	private:
		bool WasKillRequested(const TFeedbackResult& rFeedbackResult) const;

	private:
		FeedbackManagerPtr m_spFeedbackManager;
		IFilesystemPtr m_spFilesystem;
		logger::TLoggerPtr m_spLog;
		chcore::TWorkerThreadController& m_rThreadController;
	};
}

#endif
