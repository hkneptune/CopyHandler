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
/// @file  TSubTaskScanDirectory.cpp
/// @date  2010/09/18
/// @brief Contains implementation of classes related to scan directory subtask.
// ============================================================================
#include "stdafx.h"
#include "TSubTaskFastMove.h"
#include <boost\smart_ptr\make_shared.hpp>
#include "TSubTaskContext.h"
#include "TTaskConfiguration.h"
#include "TTaskDefinition.h"
#include "TLocalFilesystem.h"
#include "FeedbackHandlerBase.h"
#include "TBasePathData.h"
#include "TWorkerThreadController.h"
#include "TTaskLocalStats.h"
#include "..\libicpf\log.h"
#include "TFileInfo.h"
#include "TBasicProgressInfo.h"
#include <boost\lexical_cast.hpp>

BEGIN_CHCORE_NAMESPACE

TSubTaskFastMove::TSubTaskFastMove(TSubTaskContext& rContext) :
	TSubTaskBase(rContext)
{
}

TSubTaskFastMove::~TSubTaskFastMove()
{
}

TSubTaskFastMove::ESubOperationResult TSubTaskFastMove::Exec()
{
	// log
	icpf::log_file& rLog = GetContext().GetLog();
	TTaskDefinition& rTaskDefinition = GetContext().GetTaskDefinition();
	IFeedbackHandler* piFeedbackHandler = GetContext().GetFeedbackHandler();
	TWorkerThreadController& rThreadController = GetContext().GetThreadController();
	TTaskLocalStats& rTaskLocalStats = GetContext().GetTaskLocalStats();
	TTaskBasicProgressInfo& rProgressInfo = GetContext().GetTaskBasicProgressInfo();
	TBasePathDataContainer& rBasePathDataContainer = GetContext().GetBasePathDataContainer();

	rLog.logi(_T("Performing initial fast-move operation..."));

	// reset progress
	rTaskLocalStats.SetProcessedSize(0);
	rTaskLocalStats.SetTotalSize(0);

	// read filtering options
	TFileFiltersArray afFilters;
	GetTaskPropValue<eTO_Filters>(rTaskDefinition.GetConfiguration(), afFilters);

	//wchar_t wchDestinationDriveLetter = rTaskDefinition.GetDestinationPath().GetDriveLetter();

	bool bIgnoreDirs = GetTaskPropValue<eTO_IgnoreDirectories>(rTaskDefinition.GetConfiguration());
	bool bForceDirectories = GetTaskPropValue<eTO_CreateDirectoriesRelativeToRoot>(rTaskDefinition.GetConfiguration());

	// when using special options with move operation, we don't want to use fast-moving, since most probably
	// some searching and special processing needs to be done
	if(bIgnoreDirs || bForceDirectories)
		return eSubResult_Continue;

	// add everything
	TString strFormat;
	bool bRetry = true;
	bool bSkipInputPath = false;

	size_t stSize = rTaskDefinition.GetSourcePathCount();
	for(size_t stIndex = rProgressInfo.GetCurrentIndex(); stIndex < stSize ; stIndex++)
	{
		// store currently processed index
		rProgressInfo.SetCurrentIndex(stIndex);

		// retrieve base path data
		TBasePathDataPtr spBasePathData = rBasePathDataContainer.GetAt(stIndex);
		if(!spBasePathData)
			THROW_CORE_EXCEPTION(eErr_InvalidPointer);

		// check if we want to process this path at all
		if(spBasePathData->GetSkipFurtherProcessing())
			continue;

		TFileInfoPtr spFileInfo(boost::make_shared<TFileInfo>());
		bSkipInputPath = false;
		// try to get some info about the input path; let user know if the path does not exist.
		do
		{
			bRetry = false;

			// read attributes of src file/folder
			bool bExists = TLocalFilesystem::GetFileInfo(rTaskDefinition.GetSourcePathAt(stIndex), spFileInfo, stIndex, &rTaskDefinition.GetSourcePaths());
			if(!bExists)
			{
				FEEDBACK_FILEERROR ferr = { rTaskDefinition.GetSourcePathAt(stIndex).ToString(), NULL, eFastMoveError, ERROR_FILE_NOT_FOUND };
				IFeedbackHandler::EFeedbackResult frResult = (IFeedbackHandler::EFeedbackResult)piFeedbackHandler->RequestFeedback(IFeedbackHandler::eFT_FileError, &ferr);
				switch(frResult)
				{
				case IFeedbackHandler::eResult_Cancel:
					return eSubResult_CancelRequest;

				case IFeedbackHandler::eResult_Retry:
					bRetry = true;
					break;

				case IFeedbackHandler::eResult_Pause:
					return eSubResult_PauseRequest;

				case IFeedbackHandler::eResult_Skip:
					bSkipInputPath = true;
					break;		// just do nothing

				default:
					BOOST_ASSERT(FALSE);		// unknown result
					THROW_CORE_EXCEPTION(eErr_UnhandledCase);
				}
			}
		}
		while(bRetry);

		// if we have chosen to skip the input path then there's nothing to do
		if(bSkipInputPath)
			continue;

		// does it match the input filter?
		if(!spFileInfo->IsDirectory() && !afFilters.Match(spFileInfo))
		{
			spBasePathData->SetSkipFurtherProcessing(true);
			continue;
		}

		// try to fast move
		bRetry = true;
		bool bResult = true;
		do 
		{
			bResult = TLocalFilesystem::FastMove(rTaskDefinition.GetSourcePathAt(stIndex), CalculateDestinationPath(spFileInfo, rTaskDefinition.GetDestinationPath(), 0));
			if(!bResult)
			{
				DWORD dwLastError = GetLastError();

				//log
				strFormat = _T("Error %errno while calling fast move %srcpath -> %dstpath (TSubTaskFastMove)");
				strFormat.Replace(_T("%errno"), boost::lexical_cast<std::wstring>(dwLastError).c_str());
				strFormat.Replace(_T("%srcpath"), spFileInfo->GetFullFilePath().ToString());
				strFormat.Replace(_T("%dstpath"), rTaskDefinition.GetDestinationPath().ToString());
				rLog.loge(strFormat);

				FEEDBACK_FILEERROR ferr = { rTaskDefinition.GetSourcePathAt(stIndex).ToString(), rTaskDefinition.GetDestinationPath().ToString(), eFastMoveError, dwLastError };
				IFeedbackHandler::EFeedbackResult frResult = (IFeedbackHandler::EFeedbackResult)piFeedbackHandler->RequestFeedback(IFeedbackHandler::eFT_FileError, &ferr);
				switch(frResult)
				{
				case IFeedbackHandler::eResult_Cancel:
					return TSubTaskBase::eSubResult_CancelRequest;

				case IFeedbackHandler::eResult_Retry:
					continue;

				case IFeedbackHandler::eResult_Pause:
					return TSubTaskBase::eSubResult_PauseRequest;

				case IFeedbackHandler::eResult_Skip:
					//bSkipInputPath = true;		// not needed, since we will break the loop anyway and there is no other processing for this path either
					bRetry = false;
					break;		// just do nothing
				default:
					BOOST_ASSERT(FALSE);		// unknown result
					THROW_CORE_EXCEPTION(eErr_UnhandledCase);
				}
			}
			else
				spBasePathData->SetSkipFurtherProcessing(true);		// mark that this path should not be processed any further
		}
		while(!bResult && bRetry);

		// check for kill need
		if(rThreadController.KillRequested())
		{
			// log
			rLog.logi(_T("Kill request while adding data to files array (RecurseDirectories)"));
			return eSubResult_KillRequest;
		}
	}

	// log
	rLog.logi(_T("Fast moving finished"));

	return eSubResult_Continue;
}

END_CHCORE_NAMESPACE
