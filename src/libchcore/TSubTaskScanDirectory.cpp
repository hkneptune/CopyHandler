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
#include "TSubTaskScanDirectory.h"
#include "TSubTaskContext.h"
#include "TTaskConfiguration.h"
#include "TTaskDefinition.h"
//#include "FeedbackHandler.h"
#include "TLocalFilesystem.h"
#include "FeedbackHandlerBase.h"
#include "TBasePathData.h"
#include "TWorkerThreadController.h"
#include "TTaskLocalStats.h"
#include <boost\smart_ptr\make_shared.hpp>
#include "..\libicpf\log.h"
#include "TFileInfoArray.h"
#include "TFileInfo.h"

BEGIN_CHCORE_NAMESPACE

TSubTaskScanDirectories::TSubTaskScanDirectories(TSubTaskContext& rContext) :
	TSubTaskBase(rContext)
{
}

TSubTaskScanDirectories::~TSubTaskScanDirectories()
{
}

TSubTaskScanDirectories::ESubOperationResult TSubTaskScanDirectories::Exec()
{
	// log
	icpf::log_file& rLog = GetContext().GetLog();
	TFileInfoArray& rFilesCache = GetContext().GetFilesCache();
	TTaskDefinition& rTaskDefinition = GetContext().GetTaskDefinition();
	IFeedbackHandler* piFeedbackHandler = GetContext().GetFeedbackHandler();
	const TBasePathDataContainer& rarrSourcePathsInfo = GetContext().GetBasePathDataContainer();
	TWorkerThreadController& rThreadController = GetContext().GetThreadController();
	TTaskLocalStats& rTaskLocalStats = GetContext().GetTaskLocalStats();

	rLog.logi(_T("Searching for files..."));

	// reset progress
	rFilesCache.SetComplete(false);
	rTaskLocalStats.SetProcessedSize(0);
	rTaskLocalStats.SetTotalSize(0);

	// delete the content of rFilesCache
	rFilesCache.Clear();

	// read filtering options
	TFileFiltersArray afFilters;
	GetTaskPropValue<eTO_Filters>(rTaskDefinition.GetConfiguration(), afFilters);

	// enter some data to rFilesCache
	wchar_t wchDestinationDriveLetter = rTaskDefinition.GetDestinationPath().GetDriveLetter();

	bool bIgnoreDirs = GetTaskPropValue<eTO_IgnoreDirectories>(rTaskDefinition.GetConfiguration());
	bool bForceDirectories = GetTaskPropValue<eTO_CreateDirectoriesRelativeToRoot>(rTaskDefinition.GetConfiguration());
	bool bMove = rTaskDefinition.GetOperationType() == eOperation_Move;

	// add everything
	TString strFormat;
	bool bRetry = true;
	bool bSkipInputPath = false;

	size_t stSize = rTaskDefinition.GetSourcePathCount();
	for(size_t stIndex = 0; stIndex < stSize ; stIndex++)
	{
		TFileInfoPtr spFileInfo;

		bSkipInputPath = false;

		spFileInfo.reset(new TFileInfo());

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
					rFilesCache.Clear();
					return eSubResult_CancelRequest;

				case IFeedbackHandler::eResult_Retry:
					bRetry = true;
					break;

				case IFeedbackHandler::eResult_Pause:
					rFilesCache.Clear();
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

		// log
		strFormat = _T("Adding file/folder (clipboard) : %path ...");
		strFormat.Replace(_T("%path"), rTaskDefinition.GetSourcePathAt(stIndex).ToString());
		rLog.logi(strFormat);

		// found file/folder - check if the dest name has been generated
		if(!rarrSourcePathsInfo.GetAt(stIndex)->IsDestinationPathSet())
		{
			// generate something - if dest folder == src folder - search for copy
			if(rTaskDefinition.GetDestinationPath() == spFileInfo->GetFullFilePath().GetFileRoot())
			{
				TSmartPath pathSubst = FindFreeSubstituteName(spFileInfo->GetFullFilePath(), rTaskDefinition.GetDestinationPath());
				rarrSourcePathsInfo.GetAt(stIndex)->SetDestinationPath(pathSubst);
			}
			else
				rarrSourcePathsInfo.GetAt(stIndex)->SetDestinationPath(spFileInfo->GetFullFilePath().GetFileName());
		}

		wchar_t wchSourceDriveLetter = spFileInfo->GetFullFilePath().GetDriveLetter();

		// add if needed
		if(spFileInfo->IsDirectory())
		{
			// add if folder's aren't ignored
			if(!bIgnoreDirs && !bForceDirectories)
			{
				// add directory info; it is not to be filtered with afFilters
				rFilesCache.AddFileInfo(spFileInfo);

				// log
				strFormat = _T("Added folder %path");
				strFormat.Replace(_T("%path"), spFileInfo->GetFullFilePath().ToString());
				rLog.logi(strFormat);
			}

			// don't add folder contents when moving inside one disk boundary
			if(bIgnoreDirs || !bMove || wchDestinationDriveLetter == L'\0' || wchDestinationDriveLetter != wchSourceDriveLetter ||
				TLocalFilesystem::PathExist(CalculateDestinationPath(spFileInfo, rTaskDefinition.GetDestinationPath(), ((int)bForceDirectories) << 1)) )
			{
				// log
				strFormat = _T("Recursing folder %path");
				strFormat.Replace(_t("%path"), spFileInfo->GetFullFilePath().ToString());
				rLog.logi(strFormat);

				// no movefile possibility - use CustomCopyFileFB
				rarrSourcePathsInfo.GetAt(stIndex)->SetMove(false);

				ScanDirectory(spFileInfo->GetFullFilePath(), stIndex, true, !bIgnoreDirs || bForceDirectories, afFilters);
			}

			// check for kill need
			if(rThreadController.KillRequested())
			{
				// log
				rLog.logi(_T("Kill request while adding data to files array (RecurseDirectories)"));
				rFilesCache.Clear();
				return eSubResult_KillRequest;
			}
		}
		else
		{
			if(bMove && wchDestinationDriveLetter != L'\0' && wchDestinationDriveLetter == wchSourceDriveLetter &&
				!TLocalFilesystem::PathExist(CalculateDestinationPath(spFileInfo, rTaskDefinition.GetDestinationPath(), ((int)bForceDirectories) << 1)) )
			{
				// if moving within one partition boundary set the file size to 0 so the overall size will
				// be ok
				spFileInfo->SetLength64(0);
			}
			else
				rarrSourcePathsInfo.GetAt(stIndex)->SetMove(false);	// no MoveFile

			// add file info if passes filters
			if(afFilters.Match(spFileInfo))
				rFilesCache.AddFileInfo(spFileInfo);

			// log
			strFormat = _T("Added file %path");
			strFormat.Replace(_T("%path"), spFileInfo->GetFullFilePath().ToString());
			rLog.logi(strFormat);
		}
	}

	// calc size of all files
	rTaskLocalStats.SetTotalSize(rFilesCache.CalculateTotalSize());
	rFilesCache.SetComplete(true);

	// log
	rLog.logi(_T("Searching for files finished"));

	return eSubResult_Continue;
}

int TSubTaskScanDirectories::ScanDirectory(TSmartPath pathDirName, size_t stSrcIndex, bool bRecurse, bool bIncludeDirs, TFileFiltersArray& afFilters)
{
	TFileInfoArray& rFilesCache = GetContext().GetFilesCache();
	TTaskDefinition& rTaskDefinition = GetContext().GetTaskDefinition();
	TWorkerThreadController& rThreadController = GetContext().GetThreadController();

	TLocalFilesystemFind finder = TLocalFilesystem::CreateFinderObject(pathDirName, PathFromString(_T("*")));
	TFileInfoPtr spFileInfo(boost::make_shared<TFileInfo>());

	while(finder.FindNext(spFileInfo))
	{
		if(rThreadController.KillRequested())
			break;

		if(!spFileInfo->IsDirectory())
		{
			if(afFilters.Match(spFileInfo))
			{
				spFileInfo->SetParentObject(stSrcIndex, &rTaskDefinition.GetSourcePaths());
				rFilesCache.AddFileInfo(spFileInfo);
				spFileInfo = boost::make_shared<TFileInfo>();
			}
		}
		else
		{
			TSmartPath pathCurrent = spFileInfo->GetFullFilePath();
			if(bIncludeDirs)
			{
				spFileInfo->SetParentObject(stSrcIndex, &rTaskDefinition.GetSourcePaths());
				rFilesCache.AddFileInfo(spFileInfo);
				spFileInfo = boost::make_shared<TFileInfo>();
			}

			if(bRecurse)
				ScanDirectory(pathCurrent, stSrcIndex, bRecurse, bIncludeDirs, afFilters);
		}
	}

	return 0;
}

END_CHCORE_NAMESPACE
