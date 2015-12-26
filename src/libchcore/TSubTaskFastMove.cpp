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
#include "TCoreException.h"
#include "ErrorCodes.h"
#include "TPathContainer.h"
#include "TScopedRunningTimeTracker.h"
#include "TFeedbackHandlerWrapper.h"
#include "TBufferSizes.h"
#include "TFileException.h"
#include "TFilesystemFeedbackWrapper.h"

namespace chcore
{
	TSubTaskFastMove::TSubTaskFastMove(TSubTaskContext& rContext) :
		TSubTaskBase(rContext),
		m_tSubTaskStats(eSubOperation_FastMove)
	{
	}

	TSubTaskFastMove::~TSubTaskFastMove()
	{
	}

	void TSubTaskFastMove::Reset()
	{
		m_tSubTaskStats.Clear();
	}

	TSubTaskFastMove::ESubOperationResult TSubTaskFastMove::Exec(const IFeedbackHandlerPtr& spFeedback)
	{
		TScopedRunningTimeTracker guard(m_tSubTaskStats);
		TFeedbackHandlerWrapperPtr spFeedbackHandler(boost::make_shared<TFeedbackHandlerWrapper>(spFeedback, guard));

		// log
		icpf::log_file& rLog = GetContext().GetLog();
		TWorkerThreadController& rThreadController = GetContext().GetThreadController();
		TBasePathDataContainerPtr spBasePaths = GetContext().GetBasePaths();
		const TConfig& rConfig = GetContext().GetConfig();
		TSmartPath pathDestination = GetContext().GetDestinationPath();
		const TFileFiltersArray& rafFilters = GetContext().GetFilters();
		IFilesystemPtr spFilesystem = GetContext().GetLocalFilesystem();

		TFilesystemFeedbackWrapper tFilesystemFBWrapper(spFeedbackHandler, spFilesystem, rLog, rThreadController);

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
		if (bIgnoreDirs || bForceDirectories)
			return eSubResult_Continue;

		// add everything
		TString strFormat;

		file_count_t fcSize = spBasePaths->GetCount();
		file_count_t fcIndex = m_tSubTaskStats.GetCurrentIndex();
		for (; fcIndex < fcSize; fcIndex++)
		{
			TBasePathDataPtr spBasePath = spBasePaths->GetAt(fcIndex);
			TSmartPath pathCurrent = spBasePath->GetSrcPath();

			// store currently processed index
			m_tSubTaskStats.SetCurrentIndex(fcIndex);

			// new stats
			m_tSubTaskStats.SetProcessedCount(fcIndex);
			m_tSubTaskStats.SetCurrentPath(pathCurrent.ToString());

			// retrieve base path data
			// check if we want to process this path at all
			if (spBasePath->GetSkipFurtherProcessing())
				continue;

			TFileInfoPtr spFileInfo(boost::make_shared<TFileInfo>());

			bool bSkip = false;
			ESubOperationResult eResult = tFilesystemFBWrapper.GetFileInfoFB(pathCurrent, spFileInfo, spBasePath, bSkip);
			if (eResult != TSubTaskBase::eSubResult_Continue)
				return eResult;
			else if (bSkip)
			{
				spBasePath->SetSkipFurtherProcessing(true);
				continue;
			}

			// does it match the input filter?
			if (!spFileInfo->IsDirectory() && !rafFilters.Match(spFileInfo))
			{
				spBasePath->SetSkipFurtherProcessing(true);
				continue;
			}

			// try to fast move
			eResult = tFilesystemFBWrapper.FastMoveFB(spFileInfo, CalculateDestinationPath(spFileInfo, pathDestination, 0), spBasePath, bSkip);
			if (eResult != TSubTaskBase::eSubResult_Continue)
				return eResult;
			//else if (bSkip)
			//	continue;

			// check for kill need
			if (rThreadController.KillRequested())
			{
				// log
				rLog.logi(_T("Kill request while fast moving items"));
				return eSubResult_KillRequest;
			}
		}

		m_tSubTaskStats.SetCurrentIndex(fcIndex);
		m_tSubTaskStats.SetProcessedCount(fcIndex);
		m_tSubTaskStats.SetCurrentPath(TString());

		// log
		rLog.logi(_T("Fast moving finished"));

		return eSubResult_Continue;
	}

	void TSubTaskFastMove::GetStatsSnapshot(TSubTaskStatsSnapshotPtr& spStats) const
	{
		m_tSubTaskStats.GetSnapshot(spStats);
	}

	void TSubTaskFastMove::Store(const ISerializerPtr& spSerializer) const
	{
		ISerializerContainerPtr spContainer = spSerializer->GetContainer(_T("subtask_fastmove"));

		InitColumns(spContainer);

		ISerializerRowData& rRow = spContainer->GetRow(0, m_tSubTaskStats.WasAdded());

		m_tSubTaskStats.Store(rRow);
	}

	void TSubTaskFastMove::Load(const ISerializerPtr& spSerializer)
	{
		ISerializerContainerPtr spContainer = spSerializer->GetContainer(_T("subtask_fastmove"));

		InitColumns(spContainer);

		ISerializerRowReaderPtr spRowReader = spContainer->GetRowReader();
		if (spRowReader->Next())
			m_tSubTaskStats.Load(spRowReader);
	}

	void TSubTaskFastMove::InitColumns(const ISerializerContainerPtr& spContainer) const
	{
		IColumnsDefinition& rColumns = spContainer->GetColumnsDefinition();
		if (rColumns.IsEmpty())
			TSubTaskStatsInfo::InitColumns(rColumns);
	}
}
