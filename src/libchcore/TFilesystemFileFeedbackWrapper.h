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

namespace chcore
{
	class TFilesystemFileFeedbackWrapper
	{
	public:
		TFilesystemFileFeedbackWrapper(icpf::log_file& rLog);

		TSubTaskBase::ESubOperationResult OpenSourceFileFB(const IFeedbackHandlerPtr& spFeedbackHandler, const IFilesystemFilePtr& fileSrc);
		TSubTaskBase::ESubOperationResult OpenExistingDestinationFileFB(const IFeedbackHandlerPtr& spFeedbackHandler, const IFilesystemFilePtr& fileDst);

		TSubTaskBase::ESubOperationResult TruncateFileFB(const IFeedbackHandlerPtr& spFeedbackHandler, const IFilesystemFilePtr& spFile, file_size_t fsNewSize,
			const TSmartPath& pathFile, bool& bSkip);

		TSubTaskBase::ESubOperationResult ReadFileFB(const IFeedbackHandlerPtr& spFeedbackHandler, const IFilesystemFilePtr& spFile,
			TOverlappedDataBuffer& rBuffer, const TSmartPath& pathFile, bool& bSkip);
		TSubTaskBase::ESubOperationResult WriteFileFB(const IFeedbackHandlerPtr& spFeedbackHandler, const IFilesystemFilePtr& spFile,
			TOverlappedDataBuffer& rBuffer, const TSmartPath& pathFile, bool& bSkip);

		TSubTaskBase::ESubOperationResult FinalizeFileFB(const IFeedbackHandlerPtr& spFeedbackHandler, const IFilesystemFilePtr& spFile,
			TOverlappedDataBuffer& rBuffer, const TSmartPath& pathFile, bool& bSkip);

	private:
		icpf::log_file& m_rLog;
	};
}

#endif
