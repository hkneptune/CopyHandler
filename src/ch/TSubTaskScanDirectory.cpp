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
#include "FeedbackHandler.h"
#include "TLocalFilesystem.h"
#include "..\libchcore\FeedbackHandlerBase.h"
#include "TBasePathData.h"
#include "TWorkerThreadController.h"
#include "TTaskLocalStats.h"

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
	CFileInfoArray& rFilesCache = GetContext().GetFilesCache();
	TTaskDefinition& rTaskDefinition = GetContext().GetTaskDefinition();
	chcore::IFeedbackHandler* piFeedbackHandler = GetContext().GetFeedbackHandler();
	const TBasePathDataContainer& rarrSourcePathsInfo = GetContext().GetBasePathDataContainer();
	TWorkerThreadController& rThreadController = GetContext().GetThreadController();

	rLog.logi(_T("Searching for files..."));

	// delete the content of rFilesCache
	rFilesCache.Clear();

	// read filtering options
	CFiltersArray afFilters;
	GetTaskPropValue<eTO_Filters>(rTaskDefinition.GetConfiguration(), afFilters);

	// enter some data to rFilesCache
	int iDestDrvNumber = 0;
	TLocalFilesystem::GetDriveData(rTaskDefinition.GetDestinationPath(), &iDestDrvNumber, NULL);

	bool bIgnoreDirs = GetTaskPropValue<eTO_IgnoreDirectories>(rTaskDefinition.GetConfiguration());
	bool bForceDirectories = GetTaskPropValue<eTO_CreateDirectoriesRelativeToRoot>(rTaskDefinition.GetConfiguration());
	bool bMove = rTaskDefinition.GetOperationType() == eOperation_Move;

	// add everything
	ictranslate::CFormat fmt;
	bool bRetry = true;
	bool bSkipInputPath = false;

	size_t stSize = rTaskDefinition.GetSourcePathCount();
	for(size_t stIndex = 0; stIndex < stSize ; stIndex++)
	{
		CFileInfoPtr spFileInfo;

		bSkipInputPath = false;

		spFileInfo.reset(new CFileInfo());
		spFileInfo->SetClipboard(&rTaskDefinition.GetSourcePaths());

		// try to get some info about the input path; let user know if the path does not exist.
		do
		{
			bRetry = false;

			// read attributes of src file/folder
			bool bExists = spFileInfo->Create(rTaskDefinition.GetSourcePathAt(stIndex), stIndex);
			if(!bExists)
			{
				FEEDBACK_FILEERROR ferr = { rTaskDefinition.GetSourcePathAt(stIndex).ToString(), NULL, eFastMoveError, ERROR_FILE_NOT_FOUND };
				CFeedbackHandler::EFeedbackResult frResult = (CFeedbackHandler::EFeedbackResult)piFeedbackHandler->RequestFeedback(CFeedbackHandler::eFT_FileError, &ferr);
				switch(frResult)
				{
				case CFeedbackHandler::eResult_Cancel:
					rFilesCache.Clear();
					return eSubResult_CancelRequest;

				case CFeedbackHandler::eResult_Retry:
					bRetry = true;
					break;

				case CFeedbackHandler::eResult_Pause:
					rFilesCache.Clear();
					return eSubResult_PauseRequest;

				case CFeedbackHandler::eResult_Skip:
					bSkipInputPath = true;
					break;		// just do nothing

				default:
					BOOST_ASSERT(FALSE);		// unknown result
					THROW(_T("Unhandled case"), 0, 0, 0);
				}
			}
		}
		while(bRetry);

		// if we have chosen to skip the input path then there's nothing to do
		if(bSkipInputPath)
			continue;

		// log
		fmt.SetFormat(_T("Adding file/folder (clipboard) : %path ..."));
		fmt.SetParam(_t("%path"), rTaskDefinition.GetSourcePathAt(stIndex).ToString());
		rLog.logi(fmt);

		// found file/folder - check if the dest name has been generated
		if(!rarrSourcePathsInfo.GetAt(stIndex)->IsDestinationPathSet())
		{
			// generate something - if dest folder == src folder - search for copy
			if(rTaskDefinition.GetDestinationPath() == spFileInfo->GetFullFilePath().GetFileRoot())
			{
				chcore::TSmartPath pathSubst = FindFreeSubstituteName(spFileInfo->GetFullFilePath(), rTaskDefinition.GetDestinationPath());
				rarrSourcePathsInfo.GetAt(stIndex)->SetDestinationPath(pathSubst);
			}
			else
				rarrSourcePathsInfo.GetAt(stIndex)->SetDestinationPath(spFileInfo->GetFullFilePath().GetFileName());
		}

		// add if needed
		if(spFileInfo->IsDirectory())
		{
			// add if folder's aren't ignored
			if(!bIgnoreDirs && !bForceDirectories)
			{
				// add directory info; it is not to be filtered with afFilters
				rFilesCache.AddFileInfo(spFileInfo);

				// log
				fmt.SetFormat(_T("Added folder %path"));
				fmt.SetParam(_t("%path"), spFileInfo->GetFullFilePath().ToString());
				rLog.logi(fmt);
			}

			// don't add folder contents when moving inside one disk boundary
			if(bIgnoreDirs || !bMove || iDestDrvNumber == -1 || iDestDrvNumber != GetDriveNumber(spFileInfo) ||
				TLocalFilesystem::PathExist(CalculateDestinationPath(spFileInfo, rTaskDefinition.GetDestinationPath(), ((int)bForceDirectories) << 1)) )
			{
				// log
				fmt.SetFormat(_T("Recursing folder %path"));
				fmt.SetParam(_t("%path"), spFileInfo->GetFullFilePath().ToString());
				rLog.logi(fmt);

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
			if(bMove && iDestDrvNumber != -1 && iDestDrvNumber == GetDriveNumber(spFileInfo) &&
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
			fmt.SetFormat(_T("Added file %path"));
			fmt.SetParam(_t("%path"), spFileInfo->GetFullFilePath().ToString());
			rLog.logi(fmt);
		}
	}

	// calc size of all files
	GetContext().GetTaskLocalStats().SetTotalSize(rFilesCache.CalculateTotalSize());

	// log
	rLog.logi(_T("Searching for files finished"));

	return eSubResult_Continue;
}

int TSubTaskScanDirectories::ScanDirectory(chcore::TSmartPath pathDirName, size_t stSrcIndex, bool bRecurse, bool bIncludeDirs, CFiltersArray& afFilters)
{
	CFileInfoArray& rFilesCache = GetContext().GetFilesCache();
	TTaskDefinition& rTaskDefinition = GetContext().GetTaskDefinition();
	TWorkerThreadController& rThreadController = GetContext().GetThreadController();

	WIN32_FIND_DATA wfd;
	chcore::TSmartPath pathCurrent;

	pathCurrent = pathDirName + chcore::PathFromString(_T("*"));

	// Iterate through dirs & files
	HANDLE hFind = FindFirstFile(pathCurrent.ToString(), &wfd);
	if(hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if(!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				CFileInfoPtr spFileInfo(boost::make_shared<CFileInfo>());
				spFileInfo->SetClipboard(&rTaskDefinition.GetSourcePaths());	// this is the link table (CClipboardArray)

				spFileInfo->Create(&wfd, pathDirName, stSrcIndex);
				if(afFilters.Match(spFileInfo))
					rFilesCache.AddFileInfo(spFileInfo);
			}
			else if(wfd.cFileName[0] != _T('.') || (wfd.cFileName[1] != _T('\0') && (wfd.cFileName[1] != _T('.') || wfd.cFileName[2] != _T('\0'))))
			{
				if(bIncludeDirs)
				{
					CFileInfoPtr spFileInfo(boost::make_shared<CFileInfo>());
					spFileInfo->SetClipboard(&rTaskDefinition.GetSourcePaths());	// this is the link table (CClipboardArray)

					// Add directory itself
					spFileInfo->Create(&wfd, pathDirName, stSrcIndex);
					rFilesCache.AddFileInfo(spFileInfo);
				}
				if(bRecurse)
				{
					pathCurrent = pathDirName + chcore::PathFromString(wfd.cFileName) + chcore::PathFromString(_T("\\"));
					// Recurse Dirs
					ScanDirectory(pathCurrent, stSrcIndex, bRecurse, bIncludeDirs, afFilters);
				}
			}

			if(rThreadController.KillRequested())
				break;
		}
		while(FindNextFile(hFind, &wfd));

		FindClose(hFind);
	}

	return 0;
}
