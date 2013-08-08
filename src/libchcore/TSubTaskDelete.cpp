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
#include "TWorkerThreadController.h"
#include "TTaskConfiguration.h"
#include "TLocalFilesystem.h"
#include "..\libicpf\log.h"
#include "FeedbackHandlerBase.h"
#include <boost\lexical_cast.hpp>
#include "TFileInfoArray.h"
#include "TFileInfo.h"
#include "SerializationHelpers.h"
#include "TBinarySerializer.h"
#include "TTaskLocalStats.h"
#include "DataBuffer.h"
#include "TCoreException.h"
#include "ErrorCodes.h"

BEGIN_CHCORE_NAMESPACE

namespace details
{
	///////////////////////////////////////////////////////////////////////////////////////////////////
	// class TDeleteProgressInfo

	TDeleteProgressInfo::TDeleteProgressInfo() :
		m_stCurrentIndex(0)
	{
	}

	TDeleteProgressInfo::~TDeleteProgressInfo()
	{
	}

	void TDeleteProgressInfo::Serialize(TReadBinarySerializer& rSerializer)
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);
		Serializers::Serialize(rSerializer, m_stCurrentIndex);
	}

	void TDeleteProgressInfo::Serialize(TWriteBinarySerializer& rSerializer) const
	{
		boost::shared_lock<boost::shared_mutex> lock(m_lock);
		Serializers::Serialize(rSerializer, m_stCurrentIndex);
	}

	void TDeleteProgressInfo::ResetProgress()
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);
		m_stCurrentIndex = 0;
	}

	void TDeleteProgressInfo::SetCurrentIndex(size_t stIndex)
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);
		m_stCurrentIndex = stIndex;
	}

	void TDeleteProgressInfo::IncreaseCurrentIndex()
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);
		++m_stCurrentIndex;
	}

	size_t TDeleteProgressInfo::GetCurrentIndex() const
	{
		boost::shared_lock<boost::shared_mutex> lock(m_lock);
		return m_stCurrentIndex;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// class TSubTaskDelete

TSubTaskDelete::TSubTaskDelete(TSubTaskContext& rContext) : 
	TSubTaskBase(rContext)
{
	m_tSubTaskStats.SetSubOperationType(eSubOperation_Deleting);
}

void TSubTaskDelete::Reset()
{
	m_tProgressInfo.ResetProgress();
	m_tSubTaskStats.Clear();
}

TSubTaskBase::ESubOperationResult TSubTaskDelete::Exec()
{
	TSubTaskProcessingGuard guard(m_tSubTaskStats);

	// log
	icpf::log_file& rLog = GetContext().GetLog();
	TFileInfoArray& rFilesCache = GetContext().GetFilesCache();
	TWorkerThreadController& rThreadController = GetContext().GetThreadController();
	IFeedbackHandler* piFeedbackHandler = GetContext().GetFeedbackHandler();
	const TConfig& rConfig = GetContext().GetConfig();

	// log
	rLog.logi(_T("Deleting files (DeleteFiles)..."));

	// new stats
	m_tSubTaskStats.SetCurrentBufferIndex(TBufferSizes::eBuffer_Default);
	m_tSubTaskStats.SetTotalCount(rFilesCache.GetSize());
	m_tSubTaskStats.SetProcessedCount(0);
	m_tSubTaskStats.SetTotalSize(0);
	m_tSubTaskStats.SetProcessedSize(0);
	m_tSubTaskStats.SetCurrentPath(TString());

	// current processed path
	BOOL bSuccess;
	TFileInfoPtr spFileInfo;
	TString strFormat;

	// index points to 0 or next item to process
	size_t stIndex = m_tProgressInfo.GetCurrentIndex();
	while(stIndex < rFilesCache.GetSize())
	{
		spFileInfo = rFilesCache.GetAt(rFilesCache.GetSize() - stIndex - 1);

		m_tProgressInfo.SetCurrentIndex(stIndex);

		// new stats
		m_tSubTaskStats.SetProcessedCount(stIndex);
		m_tSubTaskStats.SetCurrentPath(spFileInfo->GetFullFilePath().ToString());

		// check for kill flag
		if(rThreadController.KillRequested())
		{
			// log
			rLog.logi(_T("Kill request while deleting files (Delete Files)"));
			return TSubTaskBase::eSubResult_KillRequest;
		}

		// current processed element
		if(!(spFileInfo->GetFlags() & FIF_PROCESSED))
		{
			++stIndex;
			continue;
		}

		// delete data
		if(spFileInfo->IsDirectory())
		{
			if(!GetTaskPropValue<eTO_ProtectReadOnlyFiles>(rConfig))
				TLocalFilesystem::SetAttributes(spFileInfo->GetFullFilePath(), FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_DIRECTORY);
			bSuccess = TLocalFilesystem::RemoveDirectory(spFileInfo->GetFullFilePath());
		}
		else
		{
			// set files attributes to normal - it'd slow processing a bit, but it's better.
			if(!GetTaskPropValue<eTO_ProtectReadOnlyFiles>(rConfig))
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

	m_tProgressInfo.SetCurrentIndex(stIndex);

	// new stats
	m_tSubTaskStats.SetProcessedCount(stIndex);
	m_tSubTaskStats.SetCurrentPath(TString());

	// log
	rLog.logi(_T("Deleting files finished"));

	return TSubTaskBase::eSubResult_Continue;
}

void TSubTaskDelete::GetStatsSnapshot(TSubTaskStatsSnapshotPtr& spStats) const
{
	m_tSubTaskStats.GetSnapshot(spStats);
	// if this subtask is not started yet, try to get the most fresh information for processing
	if(!spStats->IsRunning() && spStats->GetTotalCount() == 0 && spStats->GetTotalSize() == 0)
	{
		spStats->SetTotalCount(GetContext().GetFilesCache().GetSize());
		spStats->SetTotalSize(0);
	}

}

END_CHCORE_NAMESPACE
