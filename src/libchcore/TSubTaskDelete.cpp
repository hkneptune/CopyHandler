// ============================================================================
//  Copyright (C) 2001-2009 by Jozef Starosczyk
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
/// @file  TSubTaskDelete.cpp
/// @date  2010/09/19
/// @brief Contains implementation of classes responsible for delete sub-operation.
// ============================================================================
#include "stdafx.h"
#include "TSubTaskDelete.h"
#include "TSubTaskContext.h"
#include "TBasicProgressInfo.h"
#include "TWorkerThreadController.h"
#include "TTaskConfiguration.h"
#include "TTaskDefinition.h"
//#include "FeedbackHandler.h"
#include "TLocalFilesystem.h"
#include "..\libicpf\log.h"
#include "FeedbackHandlerBase.h"
#include <boost\lexical_cast.hpp>

BEGIN_CHCORE_NAMESPACE

TSubTaskDelete::TSubTaskDelete(TSubTaskContext& rContext) : 
	TSubTaskBase(rContext)
{
}

TSubTaskBase::ESubOperationResult TSubTaskDelete::Exec()
{
	// log
	icpf::log_file& rLog = GetContext().GetLog();
	TFileInfoArray& rFilesCache = GetContext().GetFilesCache();
	TTaskDefinition& rTaskDefinition = GetContext().GetTaskDefinition();
	TTaskBasicProgressInfo& rBasicProgressInfo = GetContext().GetTaskBasicProgressInfo();
	TWorkerThreadController& rThreadController = GetContext().GetThreadController();
	IFeedbackHandler* piFeedbackHandler = GetContext().GetFeedbackHandler();

	// log
	rLog.logi(_T("Deleting files (DeleteFiles)..."));

	// current processed path
	BOOL bSuccess;
	TFileInfoPtr spFileInfo;
	TString strFormat;

	// index points to 0 or next item to process
	size_t stIndex = rBasicProgressInfo.GetCurrentIndex();
	while(stIndex < rFilesCache.GetSize())
	{
		// set index in pTask to currently deleted element
		rBasicProgressInfo.SetCurrentIndex(stIndex);

		// check for kill flag
		if(rThreadController.KillRequested())
		{
			// log
			rLog.logi(_T("Kill request while deleting files (Delete Files)"));
			return TSubTaskBase::eSubResult_KillRequest;
		}

		// current processed element
		spFileInfo = rFilesCache.GetAt(rFilesCache.GetSize() - stIndex - 1);
		if(!(spFileInfo->GetFlags() & FIF_PROCESSED))
		{
			++stIndex;
			continue;
		}

		// delete data
		if(spFileInfo->IsDirectory())
		{
			if(!GetTaskPropValue<eTO_ProtectReadOnlyFiles>(rTaskDefinition.GetConfiguration()))
				TLocalFilesystem::SetAttributes(spFileInfo->GetFullFilePath(), FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_DIRECTORY);
			bSuccess = TLocalFilesystem::RemoveDirectory(spFileInfo->GetFullFilePath());
		}
		else
		{
			// set files attributes to normal - it'd slow processing a bit, but it's better.
			if(!GetTaskPropValue<eTO_ProtectReadOnlyFiles>(rTaskDefinition.GetConfiguration()))
				TLocalFilesystem::SetAttributes(spFileInfo->GetFullFilePath(), FILE_ATTRIBUTE_NORMAL);
			bSuccess = TLocalFilesystem::DeleteFile(spFileInfo->GetFullFilePath());
		}

		// operation failed
		DWORD dwLastError = GetLastError();
		if(!bSuccess && dwLastError != ERROR_PATH_NOT_FOUND && dwLastError != ERROR_FILE_NOT_FOUND)
		{
			// log
			strFormat = _T("Error #%errno while deleting file/folder %path");
			strFormat.Replace(_T("%errno"), boost::lexical_cast<std::wstring>(dwLastError).c_str());
			strFormat.Replace(_T("%path"), spFileInfo->GetFullFilePath().ToString());
			rLog.loge(strFormat);

			FEEDBACK_FILEERROR ferr = { spFileInfo->GetFullFilePath().ToString(), NULL, eDeleteError, dwLastError };
			IFeedbackHandler::EFeedbackResult frResult = (IFeedbackHandler::EFeedbackResult)piFeedbackHandler->RequestFeedback(IFeedbackHandler::eFT_FileError, &ferr);
			switch(frResult)
			{
			case IFeedbackHandler::eResult_Cancel:
				rLog.logi(_T("Cancel request while deleting file."));
				return TSubTaskBase::eSubResult_CancelRequest;

			case IFeedbackHandler::eResult_Retry:
				continue;	// no stIndex bump, since we are trying again

			case IFeedbackHandler::eResult_Pause:
				return TSubTaskBase::eSubResult_PauseRequest;

			case IFeedbackHandler::eResult_Skip:
				break;		// just do nothing

			default:
				BOOST_ASSERT(FALSE);		// unknown result
				THROW_CORE_EXCEPTION(eErr_UnhandledCase);
			}
		}

		++stIndex;
	}//while

	// add 1 to current index
	rBasicProgressInfo.IncreaseCurrentIndex();

	// log
	rLog.logi(_T("Deleting files finished"));

	return TSubTaskBase::eSubResult_Continue;
}

END_CHCORE_NAMESPACE