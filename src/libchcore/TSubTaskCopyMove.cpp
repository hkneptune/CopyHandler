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
/// @file  TSubTaskCopyMove.cpp
/// @date  2010/09/19
/// @brief Contains implementations of classes responsible for copy and move sub-operation.
// ============================================================================
#include "stdafx.h"
#include "TSubTaskCopyMove.h"
#include "TSubTaskContext.h"
#include "TTaskConfiguration.h"
#include "TTaskLocalStats.h"
#include "TTaskConfigTracker.h"
#include "TWorkerThreadController.h"
#include "IFeedbackHandler.h"
#include <boost/lexical_cast.hpp>
#include "TBasePathData.h"
#include "TFileInfo.h"
#include "TFileInfoArray.h"
#include "ErrorCodes.h"
#include "TCoreException.h"
#include "TPathContainer.h"
#include "TScopedRunningTimeTracker.h"
#include "TFeedbackHandlerWrapper.h"
#include "TOverlappedMemoryPool.h"
#include "RoundingFunctions.h"
#include "TTaskConfigBufferSizes.h"
#include "TFileException.h"
#include "TFilesystemFeedbackWrapper.h"
#include "TFilesystemFileFeedbackWrapper.h"
#include "TDestinationPathProvider.h"
#include "TOverlappedReaderWriterFB.h"
#include "TThreadedQueueRunner.h"
#include "TOverlappedThreadPool.h"

namespace chcore
{
	struct CUSTOM_COPY_PARAMS
	{
		CUSTOM_COPY_PARAMS() :
			spMemoryPool(std::make_shared<TOverlappedMemoryPool>())
		{
		}

		TFileInfoPtr spSrcFile;		// CFileInfo - src file
		TSmartPath pathDstFile;			// dest path with filename

		TBufferSizes tBufferSizes;
		TOverlappedMemoryPoolPtr spMemoryPool;		// buffer handling
		bool bOnlyCreate = false;			// flag from configuration - skips real copying - only create
	};

	///////////////////////////////////////////////////////////////////////////////////////////////////
	// class TSubTaskCopyMove

	TSubTaskCopyMove::TSubTaskCopyMove(TSubTaskContext& rContext) :
		TSubTaskBase(rContext),
		m_spSubTaskStats(std::make_shared<TSubTaskStatsInfo>(eSubOperation_Copying)),
		m_spLog(std::make_unique<logger::TLogger>(rContext.GetLogFileData(), L"ST-CopyMove"))
	{
	}

	void TSubTaskCopyMove::Reset()
	{
		m_spSubTaskStats->Clear();
	}

	void TSubTaskCopyMove::InitBeforeExec()
	{
		TFileInfoArray& rFilesCache = GetContext().GetFilesCache();

		file_count_t fcCount = rFilesCache.GetSize();
		if(fcCount == 0)
		{
			m_spSubTaskStats->SetCurrentPath(TString());
			return;
		}

		file_count_t fcIndex = m_spSubTaskStats->GetCurrentIndex();
		if(fcIndex >= fcCount)
			fcIndex = 0;

		TFileInfoPtr spFileInfo = rFilesCache.GetAt(fcIndex);
		m_spSubTaskStats->SetCurrentPath(spFileInfo->GetFullFilePath().ToString());
	}

	TSubTaskBase::ESubOperationResult TSubTaskCopyMove::Exec(const IFeedbackHandlerPtr& spFeedback)
	{
		TScopedRunningTimeTracker guard(*m_spSubTaskStats);
		TFeedbackHandlerWrapperPtr spFeedbackHandler(std::make_shared<TFeedbackHandlerWrapper>(spFeedback, guard));

		TFileInfoArray& rFilesCache = GetContext().GetFilesCache();
		TTaskConfigTracker& rCfgTracker = GetContext().GetCfgTracker();
		TWorkerThreadController& rThreadController = GetContext().GetThreadController();
		const TConfig& rConfig = GetContext().GetConfig();
		TSmartPath pathDestination = GetContext().GetDestinationPath();
		IFilesystemPtr spFilesystem = GetContext().GetLocalFilesystem();
		TBasePathDataContainerPtr spSrcPaths = GetContext().GetBasePaths();

		TFilesystemFeedbackWrapper tFilesystemFBWrapper(spFeedbackHandler, spFilesystem, GetContext().GetLogFileData(), rThreadController);

		// log
		LOG_INFO(m_spLog) << _T("Processing files/folders (ProcessFiles)");

		// initialize stats if not resuming (when resuming we have already initialized
		// the stats once - it is being restored in Load() too).
		if (!m_spSubTaskStats->IsInitialized())
			m_spSubTaskStats->Init(TBufferSizes::eBuffer_Default, rFilesCache.GetSize(), 0, rFilesCache.CalculateTotalSize(), rFilesCache.CalculatePartialSize(m_spSubTaskStats->GetCurrentIndex()), TString());
		else
		{
			_ASSERTE(rFilesCache.GetSize() == m_spSubTaskStats->GetTotalCount());
			if (rFilesCache.GetSize() != m_spSubTaskStats->GetTotalCount())
				throw TCoreException(eErr_InternalProblem, L"Size of files' cache differs from stats information", LOCATION);
		}

		// now it's time to check if there is enough space on destination device
		unsigned long long ullNeededSize = rFilesCache.CalculateTotalSize() - rFilesCache.CalculatePartialSize(m_spSubTaskStats->GetCurrentIndex());
		TSmartPath pathSingleSrc = spSrcPaths->GetAt(0)->GetSrcPath();
		TSubTaskBase::ESubOperationResult eResult = tFilesystemFBWrapper.CheckForFreeSpaceFB(pathSingleSrc, pathDestination, ullNeededSize);
		if(eResult != eSubResult_Continue)
			return eResult;

		// begin at index which wasn't processed previously
		file_count_t fcSize = rFilesCache.GetSize();
		file_count_t fcIndex = m_spSubTaskStats->GetCurrentIndex();
		unsigned long long ullCurrentItemProcessedSize = m_spSubTaskStats->GetCurrentItemProcessedSize();
		bool bCurrentFileSilentResume = m_spSubTaskStats->CanCurrentItemSilentResume();

		// create a buffer of size m_nBufferSize
		CUSTOM_COPY_PARAMS ccp;
		ccp.bOnlyCreate = GetTaskPropValue<eTO_CreateEmptyFiles>(rConfig);

		// remove changes in buffer sizes to avoid re-creation later
		rCfgTracker.RemoveModificationSet(TOptionsSet() % eTO_DefaultBufferSize % eTO_OneDiskBufferSize % eTO_TwoDisksBufferSize % eTO_CDBufferSize % eTO_LANBufferSize % eTO_UseOnlyDefaultBuffer % eTO_BufferQueueDepth);

		AdjustBufferIfNeeded(ccp.spMemoryPool, ccp.tBufferSizes, true);

		bool bIgnoreFolders = GetTaskPropValue<eTO_IgnoreDirectories>(rConfig);
		bool bForceDirectories = GetTaskPropValue<eTO_CreateDirectoriesRelativeToRoot>(rConfig);

		TDestinationPathProvider tDstPathProvider(spFilesystem, pathDestination,
			bIgnoreFolders, bForceDirectories,
			GetTaskPropValue<eTO_AlternateFilenameFormatString_First>(GetContext().GetConfig()),
			GetTaskPropValue<eTO_AlternateFilenameFormatString_AfterFirst>(GetContext().GetConfig()));

		// log
		TString strFormat;
		strFormat = _T("Processing files/folders (ProcessFiles):\r\n\tOnlyCreate: %create\r\n\tFiles/folders count: %filecount\r\n\tIgnore Folders: %ignorefolders\r\n\tDest path: %dstpath\r\n\tCurrent index (0-based): %currindex");
		strFormat.Replace(_T("%create"), boost::lexical_cast<std::wstring>(ccp.bOnlyCreate).c_str());
		strFormat.Replace(_T("%filecount"), boost::lexical_cast<std::wstring>(fcSize).c_str());
		strFormat.Replace(_T("%ignorefolders"), boost::lexical_cast<std::wstring>(bIgnoreFolders).c_str());
		strFormat.Replace(_T("%dstpath"), pathDestination.ToString());
		strFormat.Replace(_T("%currindex"), boost::lexical_cast<std::wstring>(fcIndex).c_str());

		LOG_INFO(m_spLog) << strFormat.c_str();

		TOverlappedThreadPool threadPool(rThreadController.GetKillThreadHandle());

		for(; fcIndex < fcSize; fcIndex++)
		{
			// should we kill ?
			if(rThreadController.KillRequested())
			{
				// log
				LOG_INFO(m_spLog) << _T("Kill request while processing file in ProcessFiles");
				return eSubResult_KillRequest;
			}

			// next file to be copied
			TFileInfoPtr spFileInfo = rFilesCache.GetAt(fcIndex);
			if(spFileInfo->IsProcessed())
				continue;

			TSmartPath pathCurrent = spFileInfo->GetFullFilePath();

			// new stats
			m_spSubTaskStats->SetCurrentIndex(fcIndex);
			m_spSubTaskStats->SetProcessedCount(fcIndex);
			m_spSubTaskStats->SetCurrentPath(pathCurrent.ToString());
			m_spSubTaskStats->SetCurrentItemSizes(ullCurrentItemProcessedSize, spFileInfo->GetLength64());	// preserve the processed size for the first item
			ullCurrentItemProcessedSize = 0;	// in next iteration we're not resuming anymore
			m_spSubTaskStats->SetCurrentItemSilentResume(bCurrentFileSilentResume);
			bCurrentFileSilentResume = false;

			// if the file was already processed (e.g. by fast-move), just consider the file skipped
			if(spFileInfo->IsBasePathProcessed())
			{
				AdjustProcessedSizeForSkip(spFileInfo);
				spFileInfo->MarkAsProcessed(true);
				continue;
			}

			// set dest path with filename
			ccp.pathDstFile = tDstPathProvider.CalculateDestinationPath(spFileInfo);

			// are the files/folders lie on the same partition ?
			bool bMove = GetContext().GetOperationType() == eOperation_Move;

			// if folder - create it
			if(spFileInfo->IsDirectory())
			{
				eResult = tFilesystemFBWrapper.CreateDirectoryFB(ccp.pathDstFile);
				if(eResult != eSubResult_Continue)
					return eResult;

				// new stats
				AdjustProcessedSizeForSkip(spFileInfo);

				spFileInfo->MarkAsProcessed(true);
			}
			else
			{
				// start copying/moving file
				ccp.spSrcFile = spFileInfo;

				// copy data
				eResult = CustomCopyFileFB(spFeedbackHandler, threadPool, &ccp);
				if (eResult == eSubResult_SkipFile)
				{
					spFileInfo->MarkAsProcessed(false);
					AdjustProcessedSizeForSkip(spFileInfo);
				}
				else if(eResult != eSubResult_Continue)
					return eResult;
				else
					spFileInfo->MarkAsProcessed(true);

				// if moving - delete file (only if config flag is set)
				if(bMove && spFileInfo->IsProcessed() && !GetTaskPropValue<eTO_DeleteInSeparateSubTask>(rConfig))
				{
					tFilesystemFBWrapper.DeleteFileFB(spFileInfo, GetTaskPropValue<eTO_ProtectReadOnlyFiles>(rConfig));
				}
			}

			// only set attributes and times when file/dir had been processed successfully.
			if(spFileInfo->IsProcessed())
			{
				if(GetTaskPropValue<eTO_SetDestinationDateTime>(rConfig))
					spFilesystem->SetFileDirectoryTime(ccp.pathDstFile, spFileInfo->GetCreationTime(), spFileInfo->GetLastAccessTime(), spFileInfo->GetLastWriteTime()); // no error checking (but most probably it should be checked)

				// attributes
				if(GetTaskPropValue<eTO_SetDestinationAttributes>(rConfig))
					spFilesystem->SetAttributes(ccp.pathDstFile, spFileInfo->GetAttributes());	// as above
			}
		}

		// update directories file times
		bool bUpdateDirTimes = GetTaskPropValue<eTO_SetDestinationDateTime>(rConfig);
		if(bUpdateDirTimes)
		{
			LOG_INFO(m_spLog) << _T("Setting directory attributes");

			// iterate backwards
			for(file_count_t fcAttrIndex = fcSize; fcAttrIndex != 0; --fcAttrIndex)
			{
				// should we kill ?
				if(rThreadController.KillRequested())
				{
					// log
					LOG_INFO(m_spLog) << _T("Kill request while processing file in ProcessFiles");
					return eSubResult_KillRequest;
				}

				TFileInfoPtr spFileInfo = rFilesCache.GetAt(fcAttrIndex - 1);
				if(spFileInfo->IsDirectory())
				{
					TSmartPath pathDstDir = tDstPathProvider.CalculateDestinationPath(spFileInfo);

					spFilesystem->SetFileDirectoryTime(pathDstDir, spFileInfo->GetCreationTime(), spFileInfo->GetLastAccessTime(), spFileInfo->GetLastWriteTime());
				}
			}
		}

		// stats
		m_spSubTaskStats->SetCurrentIndex(fcIndex);
		m_spSubTaskStats->SetProcessedCount(fcIndex);
		m_spSubTaskStats->SetCurrentPath(TString());

		// log
		LOG_INFO(m_spLog) << _T("Finished processing in ProcessFiles");

		return eSubResult_Continue;
	}

	void TSubTaskCopyMove::GetStatsSnapshot(TSubTaskStatsSnapshotPtr& spStats) const
	{
		m_spSubTaskStats->GetSnapshot(spStats);
		// if this subtask is not started yet, try to get the most fresh information for processing
		if(!spStats->IsRunning() && spStats->GetTotalCount() == 0 && spStats->GetTotalSize() == 0)
		{
			const auto& rCache = GetContext().GetFilesCache();
			spStats->SetTotalCount(rCache.GetSize());
			spStats->SetTotalSize(rCache.CalculateTotalSize());
		}
	}

	TBufferSizes::EBufferType TSubTaskCopyMove::GetBufferIndex(const TBufferSizes& rBufferSizes, const TFileInfoPtr& spFileInfo)
	{
		if(rBufferSizes.IsOnlyDefault())
			return TBufferSizes::eBuffer_Default;

		if(!spFileInfo)
			throw TCoreException(eErr_InvalidArgument, L"spFileInfo", LOCATION);

		TSmartPath pathSource = spFileInfo->GetFullFilePath();
		TSmartPath pathDestination = GetContext().GetDestinationPath();

		IFilesystem::EPathsRelation eRelation = GetContext().GetLocalFilesystem()->GetPathsRelation(pathSource, pathDestination);
		switch(eRelation)
		{
		case IFilesystem::eRelation_Network:
			return TBufferSizes::eBuffer_LAN;

		case IFilesystem::eRelation_CDRom:
			return TBufferSizes::eBuffer_CD;

		case IFilesystem::eRelation_TwoPhysicalDisks:
			return TBufferSizes::eBuffer_TwoDisks;

		case IFilesystem::eRelation_SinglePhysicalDisk:
			return TBufferSizes::eBuffer_OneDisk;

		//case eRelation_Other:
		default:
			return TBufferSizes::eBuffer_Default;
		}
	}

	TSubTaskBase::ESubOperationResult TSubTaskCopyMove::CustomCopyFileFB(const IFeedbackHandlerPtr& spFeedbackHandler,
		TOverlappedThreadPool& rThreadPool,
		CUSTOM_COPY_PARAMS* pData)
	{
		TWorkerThreadController& rThreadController = GetContext().GetThreadController();
		const TConfig& rConfig = GetContext().GetConfig();
		IFilesystemPtr spFilesystem = GetContext().GetLocalFilesystem();

		// calculate if we want to disable buffering for file transfer
		// NOTE: we are using here the file size read when scanning directories for files; it might be
		//       outdated at this point, but at present we don't want to re-read file size since it
		//       will cost additional disk access
		bool bNoBuffer = (GetTaskPropValue<eTO_DisableBuffering>(rConfig) &&
			pData->spSrcFile->GetLength64() >= GetTaskPropValue<eTO_DisableBufferingMinSize>(rConfig));

		// recreate buffer if needed
		AdjustBufferIfNeeded(pData->spMemoryPool, pData->tBufferSizes);

		// establish count of data to read
		TBufferSizes::EBufferType eBufferIndex = GetBufferIndex(pData->tBufferSizes, pData->spSrcFile);
		m_spSubTaskStats->SetCurrentBufferIndex(eBufferIndex);

		// determine buffer size to use for the operation
		DWORD dwCurrentBufferSize = RoundUp(pData->tBufferSizes.GetSizeByType(eBufferIndex), IFilesystemFile::MaxSectorSize);

		// resume copying from the position after the last processed mark; the proper value should be set
		// by OpenSrcAndDstFilesFB() - that includes the no-buffering setting if required.
		unsigned long long ullNextReadPos = m_spSubTaskStats->GetCurrentItemProcessedSize();

		TOverlappedReaderWriterFB tReaderWriter(spFilesystem,
			spFeedbackHandler,
			rThreadController,
			rThreadPool,
			pData->spSrcFile,
			pData->pathDstFile,
			m_spSubTaskStats,
			m_spLog->GetLogFileData(),
			pData->spMemoryPool,
			ullNextReadPos,
			dwCurrentBufferSize,
			bNoBuffer,
			GetTaskPropValue<eTO_ProtectReadOnlyFiles>(rConfig),
			pData->bOnlyCreate);

		ESubOperationResult eResult = tReaderWriter.Start();

		return eResult;
	}

	void TSubTaskCopyMove::AdjustProcessedSizeForSkip(const TFileInfoPtr& spSrcFileInfo)
	{
		m_spSubTaskStats->AdjustProcessedSize(m_spSubTaskStats->GetCurrentItemProcessedSize(), spSrcFileInfo->GetLength64());
	}

	bool TSubTaskCopyMove::AdjustBufferIfNeeded(const TOverlappedMemoryPoolPtr& spBuffer, TBufferSizes& rBufferSizes, bool bForce)
	{
		const TConfig& rConfig = GetContext().GetConfig();
		TTaskConfigTracker& rCfgTracker = GetContext().GetCfgTracker();

		if(bForce || (rCfgTracker.IsModified() && rCfgTracker.IsModified(TOptionsSet() % eTO_DefaultBufferSize % eTO_OneDiskBufferSize % eTO_TwoDisksBufferSize % eTO_CDBufferSize % eTO_LANBufferSize % eTO_UseOnlyDefaultBuffer % eTO_BufferQueueDepth, true)))
		{
			rBufferSizes = GetTaskPropBufferSizes(rConfig);

			// log
			TString strFormat;
			strFormat = _T("Changing buffer size to [Def:%defsize2, One:%onesize2, Two:%twosize2, CD:%cdsize2, LAN:%lansize2, Count:%cnt]");

			strFormat.Replace(_T("%defsize2"), boost::lexical_cast<std::wstring>(rBufferSizes.GetDefaultSize()).c_str());
			strFormat.Replace(_T("%onesize2"), boost::lexical_cast<std::wstring>(rBufferSizes.GetOneDiskSize()).c_str());
			strFormat.Replace(_T("%twosize2"), boost::lexical_cast<std::wstring>(rBufferSizes.GetTwoDisksSize()).c_str());
			strFormat.Replace(_T("%cdsize2"), boost::lexical_cast<std::wstring>(rBufferSizes.GetCDSize()).c_str());
			strFormat.Replace(_T("%lansize2"), boost::lexical_cast<std::wstring>(rBufferSizes.GetLANSize()).c_str());
			strFormat.Replace(_T("%cnt"), boost::lexical_cast<std::wstring>(rBufferSizes.GetBufferCount()).c_str());

			LOG_INFO(m_spLog) << strFormat.c_str();

			spBuffer->ReinitializeBuffers(rBufferSizes.GetBufferCount(), rBufferSizes.GetMaxSize());

			return true;	// buffer adjusted
		}

		return false;	// buffer did not need adjusting
	}

	void TSubTaskCopyMove::Store(const ISerializerPtr& spSerializer) const
	{
		ISerializerContainerPtr spContainer = spSerializer->GetContainer(_T("subtask_copymove"));
		InitColumns(spContainer);

		ISerializerRowData& rRow = spContainer->GetRow(0, m_spSubTaskStats->WasAdded());

		m_spSubTaskStats->Store(rRow);
	}

	void TSubTaskCopyMove::Load(const ISerializerPtr& spSerializer)
	{
		ISerializerContainerPtr spContainer = spSerializer->GetContainer(_T("subtask_copymove"));

		InitColumns(spContainer);

		ISerializerRowReaderPtr spRowReader = spContainer->GetRowReader();
		if(spRowReader->Next())
			m_spSubTaskStats->Load(spRowReader);
	}

	void TSubTaskCopyMove::InitColumns(const ISerializerContainerPtr& spContainer) const
	{
		IColumnsDefinition& rColumns = spContainer->GetColumnsDefinition();
		if(rColumns.IsEmpty())
			TSubTaskStatsInfo::InitColumns(rColumns);
	}
}
