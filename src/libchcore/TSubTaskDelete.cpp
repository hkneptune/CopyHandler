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
#include "IFeedbackHandler.h"
#include <boost\lexical_cast.hpp>
#include "TFileInfoArray.h"
#include "TFileInfo.h"
#include "TTaskLocalStats.h"
#include "TCoreException.h"
#include "ErrorCodes.h"
#include "TScopedRunningTimeTracker.h"
#include "TFeedbackHandlerWrapper.h"
#include <boost/make_shared.hpp>
#include "TBufferSizes.h"
#include "TFileException.h"
#include "TFilesystemFeedbackWrapper.h"
#include "log.h"

namespace chcore
{
	///////////////////////////////////////////////////////////////////////////////////////////////////
	// class TSubTaskDelete

	TSubTaskDelete::TSubTaskDelete(TSubTaskContext& rContext) :
		TSubTaskBase(rContext),
		m_tSubTaskStats(eSubOperation_Deleting)
	{
	}

	void TSubTaskDelete::Reset()
	{
		m_tSubTaskStats.Clear();
	}

	void TSubTaskDelete::InitBeforeExec()
	{
		TFileInfoArray& rFilesCache = GetContext().GetFilesCache();

		file_count_t fcCount = rFilesCache.GetSize();
		if(fcCount == 0)
		{
			m_tSubTaskStats.SetCurrentPath(TString());
			return;
		}

		file_count_t fcIndex = m_tSubTaskStats.GetCurrentIndex();
		if(fcIndex >= fcCount)
			fcIndex = 0;

		TFileInfoPtr spFileInfo = rFilesCache.GetAt(fcIndex);
		m_tSubTaskStats.SetCurrentPath(spFileInfo->GetFullFilePath().ToString());
	}

	TSubTaskBase::ESubOperationResult TSubTaskDelete::Exec(const IFeedbackHandlerPtr& spFeedback)
	{
		TScopedRunningTimeTracker guard(m_tSubTaskStats);
		TFeedbackHandlerWrapperPtr spFeedbackHandler(boost::make_shared<TFeedbackHandlerWrapper>(spFeedback, guard));

		// log
		chcore::log_file& rLog = GetContext().GetLog();
		TFileInfoArray& rFilesCache = GetContext().GetFilesCache();
		TWorkerThreadController& rThreadController = GetContext().GetThreadController();
		IFilesystemPtr spFilesystem = GetContext().GetLocalFilesystem();

		TFilesystemFeedbackWrapper tFilesystemFBWrapper(spFeedbackHandler, spFilesystem, rLog, rThreadController);

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
		TFileInfoPtr spFileInfo;
		TString strFormat;

		// index points to 0 or next item to process
		file_count_t fcIndex = m_tSubTaskStats.GetCurrentIndex();
		while (fcIndex < rFilesCache.GetSize())
		{
			spFileInfo = rFilesCache.GetAt(rFilesCache.GetSize() - fcIndex - 1);

			m_tSubTaskStats.SetCurrentIndex(fcIndex);

			// new stats
			m_tSubTaskStats.SetProcessedCount(fcIndex);
			m_tSubTaskStats.SetCurrentPath(spFileInfo->GetFullFilePath().ToString());

			// check for kill flag
			if (rThreadController.KillRequested())
			{
				// log
				rLog.logi(_T("Kill request while deleting files (Delete Files)"));
				return TSubTaskBase::eSubResult_KillRequest;
			}

			// if the file/dir was not processed by copy/move then do not delete
			// on the other hand, if the base path was processed (at this this it would be only by fast-move) then skip deleting
			if (!spFileInfo->IsProcessed() || spFileInfo->IsBasePathProcessed())
			{
				++fcIndex;
				continue;
			}

			ESubOperationResult eResult = TSubTaskBase::eSubResult_Continue;
			// delete data
			bool bProtectReadOnlyFiles = GetTaskPropValue<eTO_ProtectReadOnlyFiles>(GetContext().GetConfig());
			if (spFileInfo->IsDirectory())
				eResult = tFilesystemFBWrapper.RemoveDirectoryFB(spFileInfo, bProtectReadOnlyFiles);
			else
				eResult = tFilesystemFBWrapper.DeleteFileFB(spFileInfo, bProtectReadOnlyFiles);

			++fcIndex;
		}

		m_tSubTaskStats.SetCurrentIndex(fcIndex);
		m_tSubTaskStats.SetProcessedCount(fcIndex);
		m_tSubTaskStats.SetCurrentPath(TString());

		// log
		rLog.logi(_T("Deleting files finished"));

		return TSubTaskBase::eSubResult_Continue;
	}

	void TSubTaskDelete::GetStatsSnapshot(TSubTaskStatsSnapshotPtr& spStats) const
	{
		m_tSubTaskStats.GetSnapshot(spStats);
		// if this subtask is not started yet, try to get the most fresh information for processing
		if (!spStats->IsRunning() && spStats->GetTotalCount() == 0 && spStats->GetTotalSize() == 0)
		{
			spStats->SetTotalCount(GetContext().GetFilesCache().GetSize());
			spStats->SetTotalSize(0);
		}
	}

	void TSubTaskDelete::Store(const ISerializerPtr& spSerializer) const
	{
		ISerializerContainerPtr spContainer = spSerializer->GetContainer(_T("subtask_delete"));
		InitColumns(spContainer);

		ISerializerRowData& rRow = spContainer->GetRow(0, m_tSubTaskStats.WasAdded());

		m_tSubTaskStats.Store(rRow);
	}

	void TSubTaskDelete::Load(const ISerializerPtr& spSerializer)
	{
		ISerializerContainerPtr spContainer = spSerializer->GetContainer(_T("subtask_delete"));

		InitColumns(spContainer);

		ISerializerRowReaderPtr spRowReader = spContainer->GetRowReader();
		if (spRowReader->Next())
			m_tSubTaskStats.Load(spRowReader);
	}

	void TSubTaskDelete::InitColumns(const ISerializerContainerPtr& spContainer) const
	{
		IColumnsDefinition& rColumns = spContainer->GetColumnsDefinition();
		if (rColumns.IsEmpty())
			TSubTaskStatsInfo::InitColumns(rColumns);
	}
}
