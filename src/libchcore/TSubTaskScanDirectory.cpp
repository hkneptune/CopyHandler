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
#include "TLocalFilesystem.h"
#include "FeedbackHandlerBase.h"
#include "TBasePathData.h"
#include "TWorkerThreadController.h"
#include "TTaskLocalStats.h"
#include <boost\smart_ptr\make_shared.hpp>
#include "..\libicpf\log.h"
#include "TFileInfoArray.h"
#include "TFileInfo.h"
#include "SerializationHelpers.h"
#include "TBinarySerializer.h"

BEGIN_CHCORE_NAMESPACE

namespace details
{
	///////////////////////////////////////////////////////////////////////////////////////////////////
	// class TScanDirectoriesProgressInfo

	TScanDirectoriesProgressInfo::TScanDirectoriesProgressInfo() :
		m_stCurrentIndex(0)
	{
	}

	TScanDirectoriesProgressInfo::~TScanDirectoriesProgressInfo()
	{
	}

	void TScanDirectoriesProgressInfo::Serialize(TReadBinarySerializer& rSerializer)
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);
		Serializers::Serialize(rSerializer, m_stCurrentIndex);
	}

	void TScanDirectoriesProgressInfo::Serialize(TWriteBinarySerializer& rSerializer) const
	{
		boost::shared_lock<boost::shared_mutex> lock(m_lock);
		Serializers::Serialize(rSerializer, m_stCurrentIndex);
	}

	void TScanDirectoriesProgressInfo::ResetProgress()
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);
		m_stCurrentIndex = 0;
	}

	void TScanDirectoriesProgressInfo::SetCurrentIndex(size_t stIndex)
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);
		m_stCurrentIndex = stIndex;
	}

	void TScanDirectoriesProgressInfo::IncreaseCurrentIndex()
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);
		++m_stCurrentIndex;
	}

	size_t TScanDirectoriesProgressInfo::GetCurrentIndex() const
	{
		boost::shared_lock<boost::shared_mutex> lock(m_lock);
		return m_stCurrentIndex;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// class TSubTaskScanDirectories
TSubTaskScanDirectories::TSubTaskScanDirectories(TSubTaskContext& rContext) :
	TSubTaskBase(rContext)
{
}

TSubTaskScanDirectories::~TSubTaskScanDirectories()
{
}

TSubTaskScanDirectories::ESubOperationResult TSubTaskScanDirectories::Exec()
{
	TSubTaskProcessingGuard guard(m_tSubTaskStats);

	// log
	icpf::log_file& rLog = GetContext().GetLog();
	TFileInfoArray& rFilesCache = GetContext().GetFilesCache();
	TTaskDefinition& rTaskDefinition = GetContext().GetTaskDefinition();
	IFeedbackHandler* piFeedbackHandler = GetContext().GetFeedbackHandler();
	TWorkerThreadController& rThreadController = GetContext().GetThreadController();
	TTaskLocalStats& rTaskLocalStats = GetContext().GetTaskLocalStats();
	TBasePathDataContainer& rBasePathDataContainer = GetContext().GetBasePathDataContainer();

	rLog.logi(_T("Searching for files..."));

	// reset progress
	rFilesCache.SetComplete(false);

	// old stats
	rTaskLocalStats.SetProcessedSize(0);
	rTaskLocalStats.SetTotalSize(0);
	rTaskLocalStats.SetCurrentIndex(0);
	rTaskLocalStats.SetTotalItems(rTaskDefinition.GetSourcePathCount());
	rTaskLocalStats.SetCurrentPath(TString());

	// new stats
	m_tSubTaskStats.SetCurrentBufferIndex(-1);
	m_tSubTaskStats.SetTotalCount(rTaskDefinition.GetSourcePathCount());
	m_tSubTaskStats.SetProcessedCount(0);
	m_tSubTaskStats.SetTotalSize(0);
	m_tSubTaskStats.SetProcessedSize(0);
	m_tSubTaskStats.SetCurrentPath(TString());

	// delete the content of rFilesCache
	rFilesCache.Clear();

	// read filtering options
	TFileFiltersArray afFilters;
	GetTaskPropValue<eTO_Filters>(rTaskDefinition.GetConfiguration(), afFilters);

	bool bIgnoreDirs = GetTaskPropValue<eTO_IgnoreDirectories>(rTaskDefinition.GetConfiguration());
	bool bForceDirectories = GetTaskPropValue<eTO_CreateDirectoriesRelativeToRoot>(rTaskDefinition.GetConfiguration());

	// add everything
	TString strFormat;
	bool bRetry = true;
	bool bSkipInputPath = false;

	size_t stSize = rTaskDefinition.GetSourcePathCount();
	// NOTE: in theory, we should resume the scanning, but in practice we are always restarting scanning if interrupted.
	size_t stIndex = 0;		// m_tProgressInfo.GetCurrentIndex()
	for(; stIndex < stSize; stIndex++)
	{
		TSmartPath pathCurrent = rTaskDefinition.GetSourcePathAt(stIndex);

		m_tProgressInfo.SetCurrentIndex(stIndex);

		// old stats
		rTaskLocalStats.SetCurrentIndex(stIndex);
		rTaskLocalStats.SetCurrentPath(pathCurrent.ToString());

		// new stats
		m_tSubTaskStats.SetProcessedCount(stIndex);
		m_tSubTaskStats.SetCurrentPath(pathCurrent.ToString());

		bSkipInputPath = false;
		TFileInfoPtr spFileInfo(boost::make_shared<TFileInfo>());

		// retrieve base path data
		TBasePathDataPtr spBasePathData = rBasePathDataContainer.GetAt(stIndex);
		if(!spBasePathData)
			THROW_CORE_EXCEPTION(eErr_InvalidPointer);

		// check if we want to process this path at all (might be already fast moved)
		if(spBasePathData->GetSkipFurtherProcessing())
			continue;

		// try to get some info about the input path; let user know if the path does not exist.
		do
		{
			bRetry = false;

			// read attributes of src file/folder
			bool bExists = TLocalFilesystem::GetFileInfo(pathCurrent, spFileInfo, stIndex, &rTaskDefinition.GetSourcePaths());
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
			// log
			strFormat = _T("Recursing folder %path");
			strFormat.Replace(_t("%path"), spFileInfo->GetFullFilePath().ToString());
			rLog.logi(strFormat);

			ScanDirectory(spFileInfo->GetFullFilePath(), stIndex, true, !bIgnoreDirs || bForceDirectories, afFilters);

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
	m_tProgressInfo.SetCurrentIndex(stIndex);

	// old stats
	rTaskLocalStats.SetCurrentIndex(stIndex);
	rTaskLocalStats.SetCurrentPath(TString());

	// new stats
	m_tSubTaskStats.SetProcessedCount(stIndex);
	m_tSubTaskStats.SetCurrentPath(TString());

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
