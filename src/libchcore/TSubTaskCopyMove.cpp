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
#include <boost/smart_ptr/make_shared.hpp>
#include "TFileInfo.h"
#include "TFileInfoArray.h"
#include "ErrorCodes.h"
#include "TCoreException.h"
#include "TCoreWin32Exception.h"
#include "TPathContainer.h"
#include "TScopedRunningTimeTracker.h"
#include "TFeedbackHandlerWrapper.h"
#include "TOverlappedDataBufferQueue.h"
#include "TOverlappedDataBuffer.h"
#include "RoundingFunctions.h"
#include <array>
#include "TTaskConfigBufferSizes.h"
#include "TFileException.h"
#include "TFilesystemFeedbackWrapper.h"
#include "TFilesystemFileFeedbackWrapper.h"
#include "TDestinationPathProvider.h"

namespace chcore
{
	struct CUSTOM_COPY_PARAMS
	{
		TFileInfoPtr spSrcFile;		// CFileInfo - src file
		TSmartPath pathDstFile;			// dest path with filename

		TBufferSizes tBufferSizes;
		TOverlappedDataBufferQueue dbBuffer;		// buffer handling
		bool bOnlyCreate;			// flag from configuration - skips real copying - only create
		bool bProcessed;			// has the element been processed ? (false if skipped)
	};

	///////////////////////////////////////////////////////////////////////////////////////////////////
	// class TSubTaskCopyMove

	TSubTaskCopyMove::TSubTaskCopyMove(TSubTaskContext& rContext) :
		TSubTaskBase(rContext),
		m_tSubTaskStats(eSubOperation_Copying),
		m_spLog(rContext.GetLogFactory()->CreateLogger(L"ST-CopyMove"))
	{
	}

	void TSubTaskCopyMove::Reset()
	{
		m_tSubTaskStats.Clear();
	}

	void TSubTaskCopyMove::InitBeforeExec()
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

	TSubTaskBase::ESubOperationResult TSubTaskCopyMove::Exec(const IFeedbackHandlerPtr& spFeedback)
	{
		TScopedRunningTimeTracker guard(m_tSubTaskStats);
		TFeedbackHandlerWrapperPtr spFeedbackHandler(std::make_shared<TFeedbackHandlerWrapper>(spFeedback, guard));

		TFileInfoArray& rFilesCache = GetContext().GetFilesCache();
		TTaskConfigTracker& rCfgTracker = GetContext().GetCfgTracker();
		TWorkerThreadController& rThreadController = GetContext().GetThreadController();
		const TConfig& rConfig = GetContext().GetConfig();
		TSmartPath pathDestination = GetContext().GetDestinationPath();
		IFilesystemPtr spFilesystem = GetContext().GetLocalFilesystem();
		TBasePathDataContainerPtr spSrcPaths = GetContext().GetBasePaths();

		TFilesystemFeedbackWrapper tFilesystemFBWrapper(spFeedbackHandler, spFilesystem, GetContext().GetLogFactory(), rThreadController);

		// log
		LOG_INFO(m_spLog) << _T("Processing files/folders (ProcessFiles)");

		// initialize stats if not resuming (when resuming we have already initialized
		// the stats once - it is being restored in Load() too).
		if (!m_tSubTaskStats.IsInitialized())
			m_tSubTaskStats.Init(TBufferSizes::eBuffer_Default, rFilesCache.GetSize(), 0, rFilesCache.CalculateTotalSize(), rFilesCache.CalculatePartialSize(m_tSubTaskStats.GetCurrentIndex()), TString());
		else
		{
			_ASSERTE(rFilesCache.GetSize() == m_tSubTaskStats.GetTotalCount());
			if (rFilesCache.GetSize() != m_tSubTaskStats.GetTotalCount())
				throw TCoreException(eErr_InternalProblem, L"Size of files' cache differs from stats information", LOCATION);
		}

		// now it's time to check if there is enough space on destination device
		unsigned long long ullNeededSize = rFilesCache.CalculateTotalSize() - rFilesCache.CalculatePartialSize(m_tSubTaskStats.GetCurrentIndex());
		TSmartPath pathSingleSrc = spSrcPaths->GetAt(0)->GetSrcPath();
		TSubTaskBase::ESubOperationResult eResult = tFilesystemFBWrapper.CheckForFreeSpaceFB(pathSingleSrc, pathDestination, ullNeededSize);
		if(eResult != TSubTaskBase::eSubResult_Continue)
			return eResult;

		// begin at index which wasn't processed previously
		file_count_t fcSize = rFilesCache.GetSize();
		file_count_t fcIndex = m_tSubTaskStats.GetCurrentIndex();
		unsigned long long ullCurrentItemProcessedSize = m_tSubTaskStats.GetCurrentItemProcessedSize();
		bool bCurrentFileSilentResume = m_tSubTaskStats.CanCurrentItemSilentResume();

		// create a buffer of size m_nBufferSize
		CUSTOM_COPY_PARAMS ccp;
		ccp.bProcessed = false;
		ccp.bOnlyCreate = GetTaskPropValue<eTO_CreateEmptyFiles>(rConfig);

		// remove changes in buffer sizes to avoid re-creation later
		rCfgTracker.RemoveModificationSet(TOptionsSet() % eTO_DefaultBufferSize % eTO_OneDiskBufferSize % eTO_TwoDisksBufferSize % eTO_CDBufferSize % eTO_LANBufferSize % eTO_UseOnlyDefaultBuffer % eTO_BufferQueueDepth);

		AdjustBufferIfNeeded(ccp.dbBuffer, ccp.tBufferSizes, true);

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

		for(; fcIndex < fcSize; fcIndex++)
		{
			// should we kill ?
			if(rThreadController.KillRequested())
			{
				// log
				LOG_INFO(m_spLog) << _T("Kill request while processing file in ProcessFiles");
				return TSubTaskBase::eSubResult_KillRequest;
			}

			// next file to be copied
			TFileInfoPtr spFileInfo = rFilesCache.GetAt(fcIndex);
			TSmartPath pathCurrent = spFileInfo->GetFullFilePath();

			// new stats
			m_tSubTaskStats.SetCurrentIndex(fcIndex);
			m_tSubTaskStats.SetProcessedCount(fcIndex);
			m_tSubTaskStats.SetCurrentPath(pathCurrent.ToString());
			m_tSubTaskStats.SetCurrentItemSizes(ullCurrentItemProcessedSize, spFileInfo->GetLength64());	// preserve the processed size for the first item
			ullCurrentItemProcessedSize = 0;	// in next iteration we're not resuming anymore
			m_tSubTaskStats.SetCurrentItemSilentResume(bCurrentFileSilentResume);
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
				if(eResult != TSubTaskBase::eSubResult_Continue)
					return eResult;

				// new stats
				AdjustProcessedSizeForSkip(spFileInfo);

				spFileInfo->MarkAsProcessed(true);
			}
			else
			{
				// start copying/moving file
				ccp.spSrcFile = spFileInfo;
				ccp.bProcessed = false;

				// copy data
				eResult = CustomCopyFileFB(spFeedbackHandler, &ccp);
				if(eResult != TSubTaskBase::eSubResult_Continue)
					return eResult;

				spFileInfo->MarkAsProcessed(ccp.bProcessed);

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
					return TSubTaskBase::eSubResult_KillRequest;
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
		m_tSubTaskStats.SetCurrentIndex(fcIndex);
		m_tSubTaskStats.SetProcessedCount(fcIndex);
		m_tSubTaskStats.SetCurrentPath(TString());

		// log
		LOG_INFO(m_spLog) << _T("Finished processing in ProcessFiles");

		return TSubTaskBase::eSubResult_Continue;
	}

	void TSubTaskCopyMove::GetStatsSnapshot(TSubTaskStatsSnapshotPtr& spStats) const
	{
		m_tSubTaskStats.GetSnapshot(spStats);
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

	TSubTaskBase::ESubOperationResult TSubTaskCopyMove::CustomCopyFileFB(const IFeedbackHandlerPtr& spFeedbackHandler, CUSTOM_COPY_PARAMS* pData)
	{
		TWorkerThreadController& rThreadController = GetContext().GetThreadController();
		const TConfig& rConfig = GetContext().GetConfig();
		IFilesystemPtr spFilesystem = GetContext().GetLocalFilesystem();

		TFilesystemFileFeedbackWrapper tFileFBWrapper(spFeedbackHandler, GetContext().GetLogFactory(), rThreadController, spFilesystem);

		TString strFormat;
		TSubTaskBase::ESubOperationResult eResult = TSubTaskBase::eSubResult_Continue;

		// calculate if we want to disable buffering for file transfer
		// NOTE: we are using here the file size read when scanning directories for files; it might be
		//       outdated at this point, but at present we don't want to re-read file size since it
		//       will cost additional disk access
		bool bNoBuffer = (GetTaskPropValue<eTO_DisableBuffering>(rConfig) &&
			pData->spSrcFile->GetLength64() >= GetTaskPropValue<eTO_DisableBufferingMinSize>(rConfig));

		IFilesystemFilePtr fileSrc = spFilesystem->CreateFileObject(pData->spSrcFile->GetFullFilePath(), bNoBuffer);
		IFilesystemFilePtr fileDst = spFilesystem->CreateFileObject(pData->pathDstFile, bNoBuffer);

		bool bSkip = false;
		eResult = OpenSrcAndDstFilesFB(tFileFBWrapper, pData, fileSrc, fileDst, bSkip);
		if(eResult != TSubTaskBase::eSubResult_Continue)
			return eResult;
		else if(bSkip)
			return TSubTaskBase::eSubResult_Continue;

		// let the buffer queue know that we change the data source
		pData->dbBuffer.DataSourceChanged();

		// recreate buffer if needed
		AdjustBufferIfNeeded(pData->dbBuffer, pData->tBufferSizes);

		ATLTRACE(_T("CustomCopyFile: %s\n"), pData->spSrcFile->GetFullFilePath().ToString());

		// establish count of data to read
		TBufferSizes::EBufferType eBufferIndex = GetBufferIndex(pData->tBufferSizes, pData->spSrcFile);
		m_tSubTaskStats.SetCurrentBufferIndex(eBufferIndex);

		DWORD dwToRead = RoundUp(pData->tBufferSizes.GetSizeByType(eBufferIndex), IFilesystemFile::MaxSectorSize);

		// read data from file to buffer
		// NOTE: order is critical here:
		// - write finished is first, so that all the data that were already queued to be written, will be written and accounted for (in stats)
		// - kill request is second, so that we can stop processing as soon as all the data is written to destination location;
		//      that also means that we don't want to queue reads or writes anymore - all the data that were read until now, will be lost
		// - write possible - we're prioritizing write queuing here to empty buffers as soon as possible
		// - read possible - lowest priority - if we don't have anything to write or finalize , then read another part of source data
		enum { eWriteFinished, eKillThread, eWritePossible, eReadPossible, eHandleCount };
		std::array<HANDLE, eHandleCount> arrHandles = {
			pData->dbBuffer.GetEventWriteFinishedHandle(),
			rThreadController.GetKillThreadHandle(),
			pData->dbBuffer.GetEventWritePossibleHandle(),
			pData->dbBuffer.GetEventReadPossibleHandle()
		};

		// resume copying from the position after the last processed mark; the proper value should be set
		// by OpenSrcAndDstFilesFB() - that includes the no-buffering setting if required.
		unsigned long long ullNextReadPos = m_tSubTaskStats.GetCurrentItemProcessedSize();

		bool bStopProcessing = false;
		while(!bStopProcessing)
		{
			DWORD dwResult = WaitForMultipleObjectsEx(eHandleCount, arrHandles.data(), false, INFINITE, true);
			switch(dwResult)
			{
			case STATUS_USER_APC:
				break;

			case WAIT_OBJECT_0 + eKillThread:
				{
					// log
					strFormat = _T("Kill request while main copying file %srcpath -> %dstpath");
					strFormat.Replace(_T("%srcpath"), pData->spSrcFile->GetFullFilePath().ToString());
					strFormat.Replace(_T("%dstpath"), pData->pathDstFile.ToString());
					LOG_INFO(m_spLog) << strFormat.c_str();

					eResult = TSubTaskBase::eSubResult_KillRequest;
					bStopProcessing = true;
					break;
				}

			case WAIT_OBJECT_0 + eReadPossible:
				{
					TOverlappedDataBuffer* pBuffer = pData->dbBuffer.GetEmptyBuffer();
					if (!pBuffer)
						throw TCoreException(eErr_InternalProblem, L"Read was possible, but no buffer is available", LOCATION);

					pBuffer->InitForRead(ullNextReadPos, dwToRead);
					ullNextReadPos += dwToRead;

					eResult = tFileFBWrapper.ReadFileFB(fileSrc, *pBuffer, pData->spSrcFile->GetFullFilePath(), bSkip);
					if(eResult != TSubTaskBase::eSubResult_Continue)
					{
						pBuffer->RequeueAsEmpty();
						bStopProcessing = true;
					}
					else if(bSkip)
					{
						pBuffer->RequeueAsEmpty();

						AdjustProcessedSizeForSkip(pData->spSrcFile);

						pData->bProcessed = false;
						bStopProcessing = true;
					}
					break;
				}
			case WAIT_OBJECT_0 + eWritePossible:
				{
					TOverlappedDataBuffer* pBuffer = pData->dbBuffer.GetFullBuffer();
					if (!pBuffer)
						throw TCoreException(eErr_InternalProblem, L"Write was possible, but no buffer is available", LOCATION);

					// was there an error reported?
					if(pBuffer->GetErrorCode() != ERROR_SUCCESS)
					{
						// read error encountered - handle it
						eResult = HandleReadError(spFeedbackHandler, *pBuffer, pData->spSrcFile->GetFullFilePath(), bSkip);
						if(eResult == TSubTaskBase::eSubResult_Retry)
						{
							// re-request read of the same data
							eResult = tFileFBWrapper.ReadFileFB(fileSrc, *pBuffer, pData->spSrcFile->GetFullFilePath(), bSkip);
							if(eResult != TSubTaskBase::eSubResult_Continue)
							{
								pBuffer->RequeueAsEmpty();
								bStopProcessing = true;
							}
							else if(bSkip)
							{
								pBuffer->RequeueAsEmpty();

								AdjustProcessedSizeForSkip(pData->spSrcFile);

								pData->bProcessed = false;
								bStopProcessing = true;
							}
						}
						else if(eResult != TSubTaskBase::eSubResult_Continue)
						{
							pBuffer->RequeueAsEmpty();
							bStopProcessing = true;
						}
						else if(bSkip)
						{
							pBuffer->RequeueAsEmpty();

							AdjustProcessedSizeForSkip(pData->spSrcFile);

							pData->bProcessed = false;
							bStopProcessing = true;
						}
					}
					else
					{
						pBuffer->InitForWrite();

						eResult = tFileFBWrapper.WriteFileFB(fileDst, *pBuffer, pData->pathDstFile, bSkip);
						if(eResult != TSubTaskBase::eSubResult_Continue)
						{
							pBuffer->RequeueAsEmpty();
							bStopProcessing = true;
						}
						else if(bSkip)
						{
							pBuffer->RequeueAsEmpty();

							AdjustProcessedSizeForSkip(pData->spSrcFile);

							pData->bProcessed = false;
							bStopProcessing = true;
						}
					}

					break;
				}

			case WAIT_OBJECT_0 + eWriteFinished:
				{
					TOverlappedDataBuffer* pBuffer = pData->dbBuffer.GetFinishedBuffer();
					if (!pBuffer)
						throw TCoreException(eErr_InternalProblem, L"Write finished was possible, but no buffer is available", LOCATION);

					if(pBuffer->GetErrorCode() != ERROR_SUCCESS)
					{
						eResult = HandleWriteError(spFeedbackHandler, *pBuffer, pData->pathDstFile, bSkip);
						if(eResult == TSubTaskBase::eSubResult_Retry)
						{
							eResult = tFileFBWrapper.WriteFileFB(fileDst, *pBuffer, pData->pathDstFile, bSkip);
							if(eResult != TSubTaskBase::eSubResult_Continue)
							{
								pBuffer->RequeueAsEmpty();
								bStopProcessing = true;
							}
							else if(bSkip)
							{
								pBuffer->RequeueAsEmpty();

								AdjustProcessedSizeForSkip(pData->spSrcFile);

								pData->bProcessed = false;
								bStopProcessing = true;
							}
						}
						else if(eResult != TSubTaskBase::eSubResult_Continue)
						{
							pBuffer->RequeueAsEmpty();
							bStopProcessing = true;
						}
						else if(bSkip)
						{
							pBuffer->RequeueAsEmpty();

							AdjustProcessedSizeForSkip(pData->spSrcFile);

							pData->bProcessed = false;
							bStopProcessing = true;
						}
					}
					else
					{
						eResult = tFileFBWrapper.FinalizeFileFB(fileDst, *pBuffer, pData->pathDstFile, bSkip);
						if (eResult != TSubTaskBase::eSubResult_Continue)
						{
							pBuffer->RequeueAsEmpty();
							bStopProcessing = true;
						}
						else if (bSkip)
						{
							pBuffer->RequeueAsEmpty();

							AdjustProcessedSizeForSkip(pData->spSrcFile);

							pData->bProcessed = false;
							bStopProcessing = true;
						}
						else
						{
							file_size_t fsWritten = pBuffer->GetRealDataSize();

							// in case we read past the original eof, try to get new file size from filesystem
							AdjustProcessedSize(fsWritten, pData->spSrcFile, fileSrc);

							// stop iterating through file
							bStopProcessing = pBuffer->IsLastPart();

							pData->dbBuffer.MarkFinishedBufferAsComplete(pBuffer);
							pBuffer->RequeueAsEmpty();

							if(bStopProcessing)
							{
								// this is the end of copying of src file - in case it is smaller than expected fix the stats so that difference is accounted for
								AdjustFinalSize(pData->spSrcFile, fileSrc);

								pData->bProcessed = true;
								m_tSubTaskStats.ResetCurrentItemProcessedSize();
							}
						}
					}

					break;
				}

			default:
				throw TCoreException(eErr_UnhandledCase, L"Unknown result from async waiting function", LOCATION);
			}
		}

		pData->dbBuffer.WaitForMissingBuffersAndResetState(rThreadController.GetKillThreadHandle());

		return eResult;
	}

	void TSubTaskCopyMove::AdjustProcessedSize(file_size_t fsWritten, const TFileInfoPtr& spSrcFileInfo, const IFilesystemFilePtr& spSrcFile)
	{
		// in case we read past the original eof, try to get new file size from filesystem
		if (m_tSubTaskStats.WillAdjustProcessedSizeExceedTotalSize(0, fsWritten))
		{
			file_size_t fsNewSize = spSrcFile->GetFileSize();
			if (fsNewSize == spSrcFileInfo->GetLength64())
				throw TCoreException(eErr_InternalProblem, L"Read more data from file than it really contained. Possible destination file corruption.", LOCATION);

			m_tSubTaskStats.AdjustTotalSize(spSrcFileInfo->GetLength64(), fsNewSize);
			spSrcFileInfo->SetLength64(m_tSubTaskStats.GetCurrentItemTotalSize());
		}

		m_tSubTaskStats.AdjustProcessedSize(0, fsWritten);
	}

	void TSubTaskCopyMove::AdjustFinalSize(const TFileInfoPtr& spSrcFileInfo, const IFilesystemFilePtr& spSrcFile)
	{
		unsigned long long ullCITotalSize = m_tSubTaskStats.GetCurrentItemTotalSize();
		unsigned long long ullCIProcessedSize = m_tSubTaskStats.GetCurrentItemProcessedSize();
		if (ullCIProcessedSize < ullCITotalSize)
		{
			file_size_t fsNewSize = spSrcFile->GetFileSize();
			if (fsNewSize == spSrcFileInfo->GetLength64())
				throw TCoreException(eErr_InternalProblem, L"Read less data from file than it really contained. Possible destination file corruption.", LOCATION);

			if (fsNewSize != ullCIProcessedSize)
				throw TCoreException(eErr_InternalProblem, L"Updated file size still does not match the count of data read. Possible destination file corruption.", LOCATION);

			m_tSubTaskStats.AdjustTotalSize(ullCITotalSize, fsNewSize);
			spSrcFileInfo->SetLength64(fsNewSize);
		}
	}

	void TSubTaskCopyMove::AdjustProcessedSizeForSkip(const TFileInfoPtr& spSrcFileInfo)
	{
		m_tSubTaskStats.AdjustProcessedSize(m_tSubTaskStats.GetCurrentItemProcessedSize(), spSrcFileInfo->GetLength64());
	}

	TSubTaskCopyMove::ESubOperationResult TSubTaskCopyMove::OpenSrcAndDstFilesFB(TFilesystemFileFeedbackWrapper& rFileFBWrapper, CUSTOM_COPY_PARAMS* pData,
		const IFilesystemFilePtr& spFileSrc, const IFilesystemFilePtr& spFileDst, bool& bSkip)
	{
		const TConfig& rConfig = GetContext().GetConfig();
		IFilesystemPtr spFilesystem = GetContext().GetLocalFilesystem();

		bSkip = false;

		unsigned long long ullProcessedSize = m_tSubTaskStats.GetCurrentItemProcessedSize();

		// first open the source file and handle any failures
		TSubTaskCopyMove::ESubOperationResult eResult = rFileFBWrapper.OpenSourceFileFB(spFileSrc);
		if(eResult != TSubTaskBase::eSubResult_Continue)
			return eResult;
		else if(!spFileSrc->IsOpen())
		{
			// invalid handle = operation skipped by user
			AdjustProcessedSizeForSkip(pData->spSrcFile);

			pData->bProcessed = false;
			bSkip = true;
			return TSubTaskBase::eSubResult_Continue;
		}

		// update the source file size (it might differ from the time this file was originally scanned).
		// NOTE: this kind of update could be also done when copying chunks of data beyond the original end-of-file,
		//       but it would require frequent total size updates and thus - serializations).
		// NOTE2: the by-chunk corrections of stats are still applied when copying to ensure even further size
		//        matching; this update however still allows for better serialization management.
		file_size_t fsNewSize = spFileSrc->GetFileSize();
		file_size_t fsOldSize = pData->spSrcFile->GetLength64();
		if(fsNewSize != fsOldSize)
		{
			m_tSubTaskStats.AdjustTotalSize(fsOldSize, fsNewSize);
			pData->spSrcFile->SetLength64(fsNewSize);
		}

		// open destination file, handle the failures and possibly existence of the destination file
		unsigned long long ullSeekTo = ullProcessedSize;
		bool bDstFileFreshlyCreated = false;

		// try to resume if possible
		bool bResumeSucceeded = false;
		if (m_tSubTaskStats.CanCurrentItemSilentResume())
		{
			bool bContinue = true;
			TFileInfoPtr spDstFileInfo(std::make_shared<TFileInfo>());
			// verify that the file qualifies for silent resume
			try
			{
				spFilesystem->GetFileInfo(spFileDst->GetFilePath(), spDstFileInfo);
			}
			catch(const TFileException&)
			{
				bContinue = false;
			}

			if (bContinue && spDstFileInfo->GetLength64() != ullProcessedSize)
				bContinue = false;

			// we are resuming previous operation
			if(bContinue)
			{
				eResult = rFileFBWrapper.OpenExistingDestinationFileFB(spFileDst, GetTaskPropValue<eTO_ProtectReadOnlyFiles>(rConfig));
				if (eResult != TSubTaskBase::eSubResult_Continue)
					return eResult;
				else if (!spFileDst->IsOpen())
				{
					AdjustProcessedSizeForSkip(pData->spSrcFile);

					pData->bProcessed = false;
					bSkip = true;
					return TSubTaskBase::eSubResult_Continue;
				}

				bResumeSucceeded = true;
			}
		}

		if(!bResumeSucceeded)
		{
			// open destination file for case, when we start operation on this file (i.e. it is not resume of the
			// old operation)
			eResult = rFileFBWrapper.OpenDestinationFileFB(spFileDst, pData->spSrcFile, ullSeekTo, bDstFileFreshlyCreated, bSkip, GetTaskPropValue<eTO_ProtectReadOnlyFiles>(rConfig));
			if(eResult != TSubTaskBase::eSubResult_Continue)
				return eResult;
			else if(bSkip)
			{
				AdjustProcessedSizeForSkip(pData->spSrcFile);

				pData->bProcessed = false;
				return TSubTaskBase::eSubResult_Continue;
			}
		}

		if(pData->bOnlyCreate)
		{
			// we don't copy contents, but need to increase processed size
			AdjustProcessedSizeForSkip(pData->spSrcFile);

			return TSubTaskBase::eSubResult_Continue;
		}

		// ullSeekTo contains the seek position in destination file; in case the destination is already
		// larger than source file all we can do is to perform truncation of destination file to the size of
		// source file.
		// NOTE: the truncation that will be the result of the following assignment might cause the end of destination file
		// to be overwritten by the end of source file.
		ullSeekTo = std::min(ullSeekTo, fsNewSize);

		// seek to the position where copying will start
		file_size_t fsMoveTo = spFileDst->GetSeekPositionForResume(ullSeekTo);

		// sanity check
		if (bDstFileFreshlyCreated && ullSeekTo != 0)
			throw TCoreException(eErr_InternalProblem, L"Destination file was freshly created, but seek position is not 0", LOCATION);
		if(fsMoveTo > ullSeekTo)
			throw TCoreException(eErr_InternalProblem, L"File position to move to is placed after the end of file", LOCATION);

		// adjust the stats for the difference between what was already processed and what will now be considered processed
		m_tSubTaskStats.AdjustProcessedSize(ullProcessedSize, fsMoveTo);

		// if the destination file already exists - truncate it to the current file position
		if(!bDstFileFreshlyCreated)
		{
			// if destination file was opened (as opposed to newly created)
			eResult = rFileFBWrapper.TruncateFileFB(spFileDst, fsMoveTo, pData->pathDstFile, bSkip);
			if(eResult != TSubTaskBase::eSubResult_Continue)
				return eResult;
			else if(bSkip)
			{
				pData->bProcessed = false;
				return TSubTaskBase::eSubResult_Continue;
			}
		}

		// at this point user already decided that he want to write data into destination file;
		// so if we're to resume copying after this point, we don't have to ask user for overwriting existing file
		m_tSubTaskStats.SetCurrentItemSilentResume(true);

		return eResult;
	}

	bool TSubTaskCopyMove::AdjustBufferIfNeeded(TOverlappedDataBufferQueue& rBuffer, TBufferSizes& rBufferSizes, bool bForce)
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

			rBuffer.ReinitializeBuffers(rBufferSizes.GetBufferCount(), rBufferSizes.GetMaxSize());

			return true;	// buffer adjusted
		}

		return false;	// buffer did not need adjusting
	}

	TSubTaskBase::ESubOperationResult TSubTaskCopyMove::HandleReadError(const IFeedbackHandlerPtr& spFeedbackHandler,
		TOverlappedDataBuffer& rBuffer,
		const TSmartPath& pathFile,
		bool& bSkip)
	{
		DWORD dwLastError = rBuffer.GetErrorCode();

		bSkip = false;

		// log
		TString strFormat = _T("Error %errno while requesting read of %count bytes from source file %path (CustomCopyFileFB)");
		strFormat.Replace(_T("%errno"), boost::lexical_cast<std::wstring>(dwLastError).c_str());
		strFormat.Replace(_T("%count"), boost::lexical_cast<std::wstring>(rBuffer.GetRequestedDataSize()).c_str());
		strFormat.Replace(_T("%path"), pathFile.ToString());
		LOG_ERROR(m_spLog) << strFormat.c_str();

		TFeedbackResult frResult = spFeedbackHandler->FileError(pathFile.ToWString(), TString(), EFileError::eReadError, dwLastError);
		switch(frResult.GetResult())
		{
		case EFeedbackResult::eResult_Cancel:
			return TSubTaskBase::eSubResult_CancelRequest;

		case EFeedbackResult::eResult_Retry:
			return TSubTaskBase::eSubResult_Retry;

		case EFeedbackResult::eResult_Pause:
			return TSubTaskBase::eSubResult_PauseRequest;

		case EFeedbackResult::eResult_Skip:
			bSkip = true;
			return TSubTaskBase::eSubResult_Continue;

		default:
			BOOST_ASSERT(FALSE);		// unknown result
			throw TCoreException(eErr_UnhandledCase, L"Unknown feedback result", LOCATION);
		}
	}

	TSubTaskBase::ESubOperationResult TSubTaskCopyMove::HandleWriteError(const IFeedbackHandlerPtr& spFeedbackHandler,
		TOverlappedDataBuffer& rBuffer,
		const TSmartPath& pathFile,
		bool& bSkip)
	{
		DWORD dwLastError = rBuffer.GetErrorCode();

		bSkip = false;

		// log
		TString strFormat = _T("Error %errno while trying to write %count bytes to destination file %path (CustomCopyFileFB)");
		strFormat.Replace(_T("%errno"), boost::lexical_cast<std::wstring>(rBuffer.GetErrorCode()).c_str());
		strFormat.Replace(_T("%count"), boost::lexical_cast<std::wstring>(rBuffer.GetBytesTransferred()).c_str());
		strFormat.Replace(_T("%path"), pathFile.ToString());
		LOG_ERROR(m_spLog) << strFormat.c_str();

		TFeedbackResult frResult = spFeedbackHandler->FileError(pathFile.ToWString(), TString(), EFileError::eWriteError, dwLastError);
		switch (frResult.GetResult())
		{
		case EFeedbackResult::eResult_Cancel:
			return TSubTaskBase::eSubResult_CancelRequest;

		case EFeedbackResult::eResult_Retry:
			return TSubTaskBase::eSubResult_Retry;

		case EFeedbackResult::eResult_Pause:
			return TSubTaskBase::eSubResult_PauseRequest;

		case EFeedbackResult::eResult_Skip:
			bSkip = true;
			return TSubTaskBase::eSubResult_Continue;

		default:
			BOOST_ASSERT(FALSE);		// unknown result
			throw TCoreException(eErr_UnhandledCase, L"Unknown feedback result", LOCATION);
		}
	}

	void TSubTaskCopyMove::Store(const ISerializerPtr& spSerializer) const
	{
		ISerializerContainerPtr spContainer = spSerializer->GetContainer(_T("subtask_copymove"));
		InitColumns(spContainer);

		ISerializerRowData& rRow = spContainer->GetRow(0, m_tSubTaskStats.WasAdded());

		m_tSubTaskStats.Store(rRow);
	}

	void TSubTaskCopyMove::Load(const ISerializerPtr& spSerializer)
	{
		ISerializerContainerPtr spContainer = spSerializer->GetContainer(_T("subtask_copymove"));

		InitColumns(spContainer);

		ISerializerRowReaderPtr spRowReader = spContainer->GetRowReader();
		if(spRowReader->Next())
			m_tSubTaskStats.Load(spRowReader);
	}

	void TSubTaskCopyMove::InitColumns(const ISerializerContainerPtr& spContainer) const
	{
		IColumnsDefinition& rColumns = spContainer->GetColumnsDefinition();
		if(rColumns.IsEmpty())
			TSubTaskStatsInfo::InitColumns(rColumns);
	}
}
