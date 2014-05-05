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
#include "TLocalFilesystem.h"
#include "IFeedbackHandler.h"
#include "TBasePathData.h"
#include "TWorkerThreadController.h"
#include "TTaskLocalStats.h"
#include "..\libicpf\log.h"
#include "TFileInfo.h"
#include <boost\lexical_cast.hpp>
#include "SerializationHelpers.h"
#include "TBinarySerializer.h"
#include "DataBuffer.h"
#include "TCoreException.h"
#include "ErrorCodes.h"
#include "TPathContainer.h"

BEGIN_CHCORE_NAMESPACE

namespace details
{
	///////////////////////////////////////////////////////////////////////////////////////////////////
	// class TFastMoveProgressInfo

	TFastMoveProgressInfo::TFastMoveProgressInfo() :
		m_stCurrentIndex(0)
	{
	}

	TFastMoveProgressInfo::~TFastMoveProgressInfo()
	{
	}

	void TFastMoveProgressInfo::Serialize(TReadBinarySerializer& rSerializer)
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);
		Serializers::Serialize(rSerializer, m_stCurrentIndex);
	}

	void TFastMoveProgressInfo::Serialize(TWriteBinarySerializer& rSerializer) const
	{
		boost::shared_lock<boost::shared_mutex> lock(m_lock);
		Serializers::Serialize(rSerializer, m_stCurrentIndex);
	}

	void TFastMoveProgressInfo::ResetProgress()
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);
		m_stCurrentIndex = 0;
	}

	void TFastMoveProgressInfo::SetCurrentIndex(size_t stIndex)
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);
		m_stCurrentIndex = stIndex;
	}

	void TFastMoveProgressInfo::IncreaseCurrentIndex()
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);
		++m_stCurrentIndex;
	}

	size_t TFastMoveProgressInfo::GetCurrentIndex() const
	{
		boost::shared_lock<boost::shared_mutex> lock(m_lock);
		return m_stCurrentIndex;
	}
}

TSubTaskFastMove::TSubTaskFastMove(TSubTaskContext& rContext) :
	TSubTaskBase(rContext)
{
	m_tSubTaskStats.SetSubOperationType(eSubOperation_FastMove);
}

TSubTaskFastMove::~TSubTaskFastMove()
{
}

void TSubTaskFastMove::Reset()
{
	m_tProgressInfo.ResetProgress();
	m_tSubTaskStats.Clear();
}

TSubTaskFastMove::ESubOperationResult TSubTaskFastMove::Exec()
{
	TSubTaskProcessingGuard guard(m_tSubTaskStats);

	// log
	icpf::log_file& rLog = GetContext().GetLog();
	IFeedbackHandlerPtr spFeedbackHandler = GetContext().GetFeedbackHandler();
	TWorkerThreadController& rThreadController = GetContext().GetThreadController();
	TBasePathDataContainerPtr spBasePaths = GetContext().GetBasePaths();
	const TConfig& rConfig = GetContext().GetConfig();
	TSmartPath pathDestination = GetContext().GetDestinationPath();
	const TFileFiltersArray& rafFilters = GetContext().GetFilters();

	rLog.logi(_T("Performing initial fast-move operation..."));

	// new stats
	m_tSubTaskStats.SetCurrentBufferIndex(TBufferSizes::eBuffer_Default);
	m_tSubTaskStats.SetTotalCount(spBasePaths->GetCount());
	m_tSubTaskStats.SetProcessedCount(0);
	m_tSubTaskStats.SetTotalSize(0);
	m_tSubTaskStats.SetProcessedSize(0);
	m_tSubTaskStats.SetCurrentPath(TString());

	bool bIgnoreDirs = GetTaskPropValue<eTO_IgnoreDirectories>(rConfig);
	bool bForceDirectories = GetTaskPropValue<eTO_CreateDirectoriesRelativeToRoot>(rConfig);

	// when using special options with move operation, we don't want to use fast-moving, since most probably
	// some searching and special processing needs to be done
	if(bIgnoreDirs || bForceDirectories)
		return eSubResult_Continue;

	// add everything
	TString strFormat;
	bool bRetry = true;
	bool bSkipInputPath = false;

	size_t stSize = spBasePaths->GetCount();
	size_t stIndex = m_tProgressInfo.GetCurrentIndex();
	for(; stIndex < stSize ; stIndex++)
	{
		TBasePathDataPtr spBasePath = spBasePaths->GetAt(stIndex);
		TSmartPath pathCurrent = spBasePath->GetSrcPath();

		// store currently processed index
		m_tProgressInfo.SetCurrentIndex(stIndex);

		// new stats
		m_tSubTaskStats.SetProcessedCount(stIndex);
		m_tSubTaskStats.SetCurrentPath(pathCurrent.ToString());

		// retrieve base path data
		// check if we want to process this path at all
		if(spBasePath->GetSkipFurtherProcessing())
			continue;

		TFileInfoPtr spFileInfo(boost::make_shared<TFileInfo>());
		bSkipInputPath = false;
		// try to get some info about the input path; let user know if the path does not exist.
		do
		{
			bRetry = false;

			// read attributes of src file/folder
			bool bExists = TLocalFilesystem::GetFileInfo(pathCurrent, spFileInfo, spBasePath);
			if(!bExists)
			{
				FEEDBACK_FILEERROR ferr = { pathCurrent.ToString(), NULL, eFastMoveError, ERROR_FILE_NOT_FOUND };
				IFeedbackHandler::EFeedbackResult frResult = (IFeedbackHandler::EFeedbackResult)spFeedbackHandler->RequestFeedback(IFeedbackHandler::eFT_FileError, &ferr);
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
		if(!spFileInfo->IsDirectory() && !rafFilters.Match(spFileInfo))
		{
			spBasePath->SetSkipFurtherProcessing(true);
			continue;
		}

		// try to fast move
		bRetry = true;
		bool bResult = true;
		do
		{
			TSmartPath pathDestinationPath = CalculateDestinationPath(spFileInfo, pathDestination, 0);
			TSmartPath pathSrc = spBasePath->GetSrcPath();
			bResult = TLocalFilesystem::FastMove(pathSrc, pathDestinationPath);
			if(!bResult)
			{
				DWORD dwLastError = GetLastError();

				// check if this is one of the errors, that will just cause fast move to skip
				if(dwLastError == ERROR_ACCESS_DENIED || dwLastError == ERROR_ALREADY_EXISTS)
				{
					bRetry = false;
					bResult = true;
				}
				else
				{
					//log
					strFormat = _T("Error %errno while calling fast move %srcpath -> %dstpath (TSubTaskFastMove)");
					strFormat.Replace(_T("%errno"), boost::lexical_cast<std::wstring>(dwLastError).c_str());
					strFormat.Replace(_T("%srcpath"), spFileInfo->GetFullFilePath().ToString());
					strFormat.Replace(_T("%dstpath"), pathDestination.ToString());
					rLog.loge(strFormat);

					FEEDBACK_FILEERROR ferr = { pathSrc.ToString(), pathDestinationPath.ToString(), eFastMoveError, dwLastError };
					IFeedbackHandler::EFeedbackResult frResult = (IFeedbackHandler::EFeedbackResult)spFeedbackHandler->RequestFeedback(IFeedbackHandler::eFT_FileError, &ferr);
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
			}
			else
				spBasePath->SetSkipFurtherProcessing(true);		// mark that this path should not be processed any further
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

	m_tProgressInfo.SetCurrentIndex(stIndex);

	// new stats
	m_tSubTaskStats.SetProcessedCount(stIndex);
	m_tSubTaskStats.SetCurrentPath(TString());

	// log
	rLog.logi(_T("Fast moving finished"));

	return eSubResult_Continue;
}

void TSubTaskFastMove::GetStatsSnapshot(TSubTaskStatsSnapshotPtr& spStats) const
{
	m_tSubTaskStats.GetSnapshot(spStats);
}

END_CHCORE_NAMESPACE
