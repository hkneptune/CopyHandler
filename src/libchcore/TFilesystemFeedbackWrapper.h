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
#include "../common/TLogger.h"
#include "../common/TLoggerFactory.h"

namespace chcore
{
	class TWorkerThreadController;

	class TFilesystemFeedbackWrapper
	{
	public:
		TFilesystemFeedbackWrapper(const IFeedbackHandlerPtr& spFeedbackHandler, const IFilesystemPtr& spFilesystem, const TLoggerFactoryPtr& spLogFactory, TWorkerThreadController& rThreadController);
		TFilesystemFeedbackWrapper& operator=(const TFilesystemFeedbackWrapper&) = delete;

		TSubTaskBase::ESubOperationResult CreateDirectoryFB(const TSmartPath& pathDirectory);
		TSubTaskBase::ESubOperationResult CheckForFreeSpaceFB(const TSmartPath& pathFirstSrc, const TSmartPath& pathDestination, unsigned long long ullNeededSize);

		TSubTaskBase::ESubOperationResult RemoveDirectoryFB(const TFileInfoPtr& spFileInfo, bool bProtectReadOnlyFiles);
		TSubTaskBase::ESubOperationResult DeleteFileFB(const TFileInfoPtr& spFileInfo, bool bProtectReadOnlyFiles);

		TSubTaskBase::ESubOperationResult FastMoveFB(const TFileInfoPtr& spFileInfo, const TSmartPath& pathDestination,
			const TBasePathDataPtr& spBasePath, bool& bSkip);

		TSubTaskBase::ESubOperationResult GetFileInfoFB(const TSmartPath& pathCurrent,
			TFileInfoPtr& spFileInfo, const TBasePathDataPtr& spBasePath, bool& bSkip);

	private:
		bool WasKillRequested(const TFeedbackResult& rFeedbackResult) const;

	private:
		IFeedbackHandlerPtr m_spFeedbackHandler;
		IFilesystemPtr m_spFilesystem;
		TLoggerPtr m_spLog;
		TWorkerThreadController& m_rThreadController;
	};
}

#endif
