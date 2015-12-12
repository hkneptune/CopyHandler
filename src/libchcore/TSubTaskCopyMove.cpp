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
#include "TLocalFilesystem.h"
#include "../libicpf/log.h"
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
#include "TPathContainer.h"
#include "TScopedRunningTimeTracker.h"
#include "TFeedbackHandlerWrapper.h"
#include "TOverlappedDataBufferQueue.h"
#include "TOverlappedDataBuffer.h"
#include "RoundingFunctions.h"
#include <array>
#include "TTaskConfigBufferSizes.h"

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

	TSubTaskCopyMove::TSubTaskCopyMove(TSubTaskContext& tSubTaskContext) :
		TSubTaskBase(tSubTaskContext),
		m_tSubTaskStats(eSubOperation_Copying)
	{
	}

	void TSubTaskCopyMove::Reset()
	{
		m_tSubTaskStats.Clear();
	}

	TSubTaskBase::ESubOperationResult TSubTaskCopyMove::Exec(const IFeedbackHandlerPtr& spFeedback)
	{
		TScopedRunningTimeTracker guard(m_tSubTaskStats);
		TFeedbackHandlerWrapperPtr spFeedbackHandler(boost::make_shared<TFeedbackHandlerWrapper>(spFeedback, guard));

		icpf::log_file& rLog = GetContext().GetLog();
		TFileInfoArray& rFilesCache = GetContext().GetFilesCache();
		TTaskConfigTracker& rCfgTracker = GetContext().GetCfgTracker();
		TWorkerThreadController& rThreadController = GetContext().GetThreadController();
		const TConfig& rConfig = GetContext().GetConfig();
		TSmartPath pathDestination = GetContext().GetDestinationPath();
		IFilesystemPtr spFilesystem = GetContext().GetLocalFilesystem();

		// log
		rLog.logi(_T("Processing files/folders (ProcessFiles)"));

		// initialize stats if not resuming (when resuming we have already initialized
		// the stats once - it is being restored in Load() too).
		if (!m_tSubTaskStats.IsInitialized())
			m_tSubTaskStats.Init(TBufferSizes::eBuffer_Default, rFilesCache.GetSize(), 0, rFilesCache.CalculateTotalSize(), rFilesCache.CalculatePartialSize(m_tSubTaskStats.GetCurrentIndex()), TString());
		else
		{
			_ASSERTE(rFilesCache.GetSize() == m_tSubTaskStats.GetTotalCount());
			if (rFilesCache.GetSize() != m_tSubTaskStats.GetTotalCount())
				THROW_CORE_EXCEPTION(eErr_InternalProblem);
		}

		// now it's time to check if there is enough space on destination device
		TSubTaskBase::ESubOperationResult eResult = CheckForFreeSpaceFB(spFeedbackHandler);
		if(eResult != TSubTaskBase::eSubResult_Continue)
			return eResult;

		// begin at index which wasn't processed previously
		file_count_t fcSize = rFilesCache.GetSize();
		file_count_t fcIndex = m_tSubTaskStats.GetCurrentIndex();
		unsigned long long ullCurrentItemProcessedSize = m_tSubTaskStats.GetCurrentItemProcessedSize();
		bool bCurrentFileSilentResume = m_tSubTaskStats.CanCurrentItemSilentResume();

		bool bIgnoreFolders = GetTaskPropValue<eTO_IgnoreDirectories>(rConfig);
		bool bForceDirectories = GetTaskPropValue<eTO_CreateDirectoriesRelativeToRoot>(rConfig);

		// create a buffer of size m_nBufferSize
		CUSTOM_COPY_PARAMS ccp;
		ccp.bProcessed = false;
		ccp.bOnlyCreate = GetTaskPropValue<eTO_CreateEmptyFiles>(rConfig);

		// remove changes in buffer sizes to avoid re-creation later
		rCfgTracker.RemoveModificationSet(TOptionsSet() % eTO_DefaultBufferSize % eTO_OneDiskBufferSize % eTO_TwoDisksBufferSize % eTO_CDBufferSize % eTO_LANBufferSize % eTO_UseOnlyDefaultBuffer % eTO_BufferQueueDepth);

		AdjustBufferIfNeeded(ccp.dbBuffer, ccp.tBufferSizes, true);

		// log
		TString strFormat;
		strFormat = _T("Processing files/folders (ProcessFiles):\r\n\tOnlyCreate: %create\r\n\tFiles/folders count: %filecount\r\n\tIgnore Folders: %ignorefolders\r\n\tDest path: %dstpath\r\n\tCurrent index (0-based): %currindex");
		strFormat.Replace(_T("%create"), boost::lexical_cast<std::wstring>(ccp.bOnlyCreate).c_str());
		strFormat.Replace(_T("%filecount"), boost::lexical_cast<std::wstring>(fcSize).c_str());
		strFormat.Replace(_T("%ignorefolders"), boost::lexical_cast<std::wstring>(bIgnoreFolders).c_str());
		strFormat.Replace(_T("%dstpath"), pathDestination.ToString());
		strFormat.Replace(_T("%currindex"), boost::lexical_cast<std::wstring>(fcIndex).c_str());

		rLog.logi(strFormat.c_str());

		for(; fcIndex < fcSize; fcIndex++)
		{
			// should we kill ?
			if(rThreadController.KillRequested())
			{
				// log
				rLog.logi(_T("Kill request while processing file in ProcessFiles"));
				return TSubTaskBase::eSubResult_KillRequest;
			}

			// next file to be copied
			TFileInfoPtr spFileInfo = rFilesCache.GetAt(fcIndex);
			TSmartPath pathCurrent = spFileInfo->GetFullFilePath();

			// new stats
			m_tSubTaskStats.SetCurrentIndex(fcIndex);
			m_tSubTaskStats.SetProcessedCount(fcIndex);
			m_tSubTaskStats.SetCurrentPath(pathCurrent.ToString());
			m_tSubTaskStats.SetCurrentItemProcessedSize(ullCurrentItemProcessedSize);	// preserve the processed size for the first item
			ullCurrentItemProcessedSize = 0;
			m_tSubTaskStats.SetCurrentItemTotalSize(spFileInfo->GetLength64());
			m_tSubTaskStats.SetCurrentItemSilentResume(bCurrentFileSilentResume);
			bCurrentFileSilentResume = false;

			// set dest path with filename
			ccp.pathDstFile = CalculateDestinationPath(spFileInfo, pathDestination, ((int)bForceDirectories) << 1 | (int)bIgnoreFolders);

			// are the files/folders lie on the same partition ?
			bool bMove = GetContext().GetOperationType() == eOperation_Move;

			// if folder - create it
			if(spFileInfo->IsDirectory())
			{
				eResult = CreateDirectoryFB(spFeedbackHandler, ccp.pathDstFile);
				if(eResult != TSubTaskBase::eSubResult_Continue)
					return eResult;

				// new stats
				m_tSubTaskStats.IncreaseProcessedSize(spFileInfo->GetLength64());
				m_tSubTaskStats.IncreaseCurrentItemProcessedSize(spFileInfo->GetLength64());

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
					if(!GetTaskPropValue<eTO_ProtectReadOnlyFiles>(rConfig))
						spFilesystem->SetAttributes(spFileInfo->GetFullFilePath(), FILE_ATTRIBUTE_NORMAL);
					spFilesystem->DeleteFile(spFileInfo->GetFullFilePath());	// there will be another try later, so we don't check
					// if succeeded
				}
			}

			// set a time
			if(GetTaskPropValue<eTO_SetDestinationDateTime>(rConfig))
				spFilesystem->SetFileDirectoryTime(ccp.pathDstFile, spFileInfo->GetCreationTime(), spFileInfo->GetLastAccessTime(), spFileInfo->GetLastWriteTime()); // no error checking (but most probably it should be checked)

			// attributes
			if(GetTaskPropValue<eTO_SetDestinationAttributes>(rConfig))
				spFilesystem->SetAttributes(ccp.pathDstFile, spFileInfo->GetAttributes());	// as above
		}

		m_tSubTaskStats.SetCurrentIndex(fcIndex);

		// new stats
		m_tSubTaskStats.SetProcessedCount(fcIndex);
		m_tSubTaskStats.SetCurrentPath(TString());

		// log
		rLog.logi(_T("Finished processing in ProcessFiles"));

		return TSubTaskBase::eSubResult_Continue;
	}

	void TSubTaskCopyMove::GetStatsSnapshot(TSubTaskStatsSnapshotPtr& spStats) const
	{
		m_tSubTaskStats.GetSnapshot(spStats);
		// if this subtask is not started yet, try to get the most fresh information for processing
		if(!spStats->IsRunning() && spStats->GetTotalCount() == 0 && spStats->GetTotalSize() == 0)
		{
			spStats->SetTotalCount(GetContext().GetFilesCache().GetSize());
			spStats->SetTotalSize(GetContext().GetFilesCache().CalculateTotalSize());
		}
	}

	TBufferSizes::EBufferType TSubTaskCopyMove::GetBufferIndex(const TBufferSizes& rBufferSizes, const TFileInfoPtr& spFileInfo)
	{
		if(rBufferSizes.IsOnlyDefault())
			return TBufferSizes::eBuffer_Default;

		if(!spFileInfo)
			THROW_CORE_EXCEPTION(eErr_InvalidArgument);

		TSmartPath pathSource = spFileInfo->GetFullFilePath();
		TSmartPath pathDestination = GetContext().GetDestinationPath();

		TLocalFilesystem::EPathsRelation eRelation = GetContext().GetLocalFilesystem()->GetPathsRelation(pathSource, pathDestination);
		switch(eRelation)
		{
		case TLocalFilesystem::eRelation_Network:
			return TBufferSizes::eBuffer_LAN;

		case TLocalFilesystem::eRelation_CDRom:
			return TBufferSizes::eBuffer_CD;

		case TLocalFilesystem::eRelation_TwoPhysicalDisks:
			return TBufferSizes::eBuffer_TwoDisks;

		case TLocalFilesystem::eRelation_SinglePhysicalDisk:
			return TBufferSizes::eBuffer_OneDisk;

		//case eRelation_Other:
		default:
			return TBufferSizes::eBuffer_Default;
		}
	}

	TSubTaskBase::ESubOperationResult TSubTaskCopyMove::CustomCopyFileFB(const IFeedbackHandlerPtr& spFeedbackHandler, CUSTOM_COPY_PARAMS* pData)
	{
		TWorkerThreadController& rThreadController = GetContext().GetThreadController();
		icpf::log_file& rLog = GetContext().GetLog();
		const TConfig& rConfig = GetContext().GetConfig();
		IFilesystemPtr spFilesystem = GetContext().GetLocalFilesystem();

		IFilesystemFilePtr fileSrc = spFilesystem->CreateFileObject(pData->spSrcFile->GetFullFilePath());
		IFilesystemFilePtr fileDst = spFilesystem->CreateFileObject(pData->pathDstFile);

		TString strFormat;
		TSubTaskBase::ESubOperationResult eResult = TSubTaskBase::eSubResult_Continue;

		// calculate if we want to disable buffering for file transfer
		// NOTE: we are using here the file size read when scanning directories for files; it might be
		//       outdated at this point, but at present we don't want to re-read file size since it
		//       will cost additional disk access
		bool bNoBuffer = (GetTaskPropValue<eTO_DisableBuffering>(rConfig) &&
			pData->spSrcFile->GetLength64() >= GetTaskPropValue<eTO_DisableBufferingMinSize>(rConfig));

		bool bSkip = false;
		eResult = OpenSrcAndDstFilesFB(spFeedbackHandler, pData, fileSrc, fileDst, bNoBuffer, bSkip);
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
		enum { eKillThread = 0, eWriteFinished, eWritePossible, eReadPossible, eHandleCount };
		std::array<HANDLE, eHandleCount> arrHandles = {
			rThreadController.GetKillThreadHandle(),
			pData->dbBuffer.GetEventWriteFinishedHandle(),
			pData->dbBuffer.GetEventWritePossibleHandle(),
			pData->dbBuffer.GetEventReadPossibleHandle()
		};

		// #bug: always starting at the beginning of file instead of the position that OpenSrcAndDstFilesFB set the files to
		unsigned long long ullNextReadPos = 0;

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
					rLog.logi(strFormat.c_str());
					return TSubTaskBase::eSubResult_KillRequest;
				}

			case WAIT_OBJECT_0 + eReadPossible:
				{
					TOverlappedDataBuffer* pBuffer = pData->dbBuffer.GetEmptyBuffer();
					if (!pBuffer)
						THROW_CORE_EXCEPTION(eErr_InternalProblem);

					pBuffer->InitForRead(ullNextReadPos, dwToRead);
					ullNextReadPos += dwToRead;

					eResult = ReadFileFB(spFeedbackHandler, fileSrc, *pBuffer, pData->spSrcFile->GetFullFilePath(), bSkip);
					if(eResult != TSubTaskBase::eSubResult_Continue)
						return eResult;
					else if(bSkip)
					{
						// new stats
						m_tSubTaskStats.IncreaseProcessedSize(pData->spSrcFile->GetLength64() - m_tSubTaskStats.GetCurrentItemProcessedSize());
						m_tSubTaskStats.IncreaseCurrentItemProcessedSize(pData->spSrcFile->GetLength64() - m_tSubTaskStats.GetCurrentItemProcessedSize());

						pData->bProcessed = false;
						return TSubTaskBase::eSubResult_Continue;
					}
					break;
				}
			case WAIT_OBJECT_0 + eWritePossible:
				{
					TOverlappedDataBuffer* pBuffer = pData->dbBuffer.GetFullBuffer();
					if (!pBuffer)
						THROW_CORE_EXCEPTION(eErr_InternalProblem);

					// was there an error reported?
					if(pBuffer->GetErrorCode() != ERROR_SUCCESS)
					{
						// read error encountered - handle it
						eResult = HandleReadError(spFeedbackHandler, *pBuffer, pData->spSrcFile->GetFullFilePath(), bSkip);
						if(eResult == TSubTaskBase::eSubResult_Retry)
						{
							// re-request read of the same data
							eResult = ReadFileFB(spFeedbackHandler, fileSrc, *pBuffer, pData->spSrcFile->GetFullFilePath(), bSkip);
							if(eResult != TSubTaskBase::eSubResult_Continue)
								return eResult;
							else if(bSkip)
							{
								// new stats
								m_tSubTaskStats.IncreaseProcessedSize(pData->spSrcFile->GetLength64() - m_tSubTaskStats.GetCurrentItemProcessedSize());
								m_tSubTaskStats.IncreaseCurrentItemProcessedSize(pData->spSrcFile->GetLength64() - m_tSubTaskStats.GetCurrentItemProcessedSize());

								pData->bProcessed = false;
								return TSubTaskBase::eSubResult_Continue;
							}
						}
						else if(eResult != TSubTaskBase::eSubResult_Continue)
							return eResult;
						else if(bSkip)
						{
							// new stats
							m_tSubTaskStats.IncreaseProcessedSize(pData->spSrcFile->GetLength64() - m_tSubTaskStats.GetCurrentItemProcessedSize());
							m_tSubTaskStats.IncreaseCurrentItemProcessedSize(pData->spSrcFile->GetLength64() - m_tSubTaskStats.GetCurrentItemProcessedSize());

							pData->bProcessed = false;
							return TSubTaskBase::eSubResult_Continue;
						}
					}
					else
					{
						pBuffer->InitForWrite();

						eResult = WriteFileFB(spFeedbackHandler, fileDst, *pBuffer, pData->pathDstFile, bSkip);
						if(eResult != TSubTaskBase::eSubResult_Continue)
							return eResult;
						else if(bSkip)
						{
							// new stats
							m_tSubTaskStats.IncreaseProcessedSize(pData->spSrcFile->GetLength64() - m_tSubTaskStats.GetCurrentItemProcessedSize());
							m_tSubTaskStats.IncreaseCurrentItemProcessedSize(pData->spSrcFile->GetLength64() - m_tSubTaskStats.GetCurrentItemProcessedSize());

							pData->bProcessed = false;
							return TSubTaskBase::eSubResult_Continue;
						}
					}

					break;
				}

			case WAIT_OBJECT_0 + eWriteFinished:
				{
					TOverlappedDataBuffer* pBuffer = pData->dbBuffer.GetFinishedBuffer();
					if (!pBuffer)
						THROW_CORE_EXCEPTION(eErr_InternalProblem);

					if(pBuffer->GetErrorCode() != ERROR_SUCCESS)
					{
						eResult = HandleWriteError(spFeedbackHandler, *pBuffer, pData->spSrcFile->GetFullFilePath(), bSkip);
						if(eResult == TSubTaskBase::eSubResult_Retry)
						{
							eResult = WriteFileFB(spFeedbackHandler, fileDst, *pBuffer, pData->pathDstFile, bSkip);
							if(eResult != TSubTaskBase::eSubResult_Continue)
								return eResult;
							else if(bSkip)
							{
								// new stats
								m_tSubTaskStats.IncreaseProcessedSize(pData->spSrcFile->GetLength64() - m_tSubTaskStats.GetCurrentItemProcessedSize());
								m_tSubTaskStats.IncreaseCurrentItemProcessedSize(pData->spSrcFile->GetLength64() - m_tSubTaskStats.GetCurrentItemProcessedSize());

								pData->bProcessed = false;
								return TSubTaskBase::eSubResult_Continue;
							}
						}
						else if(eResult != TSubTaskBase::eSubResult_Continue)
							return eResult;
						else if(bSkip)
						{
							// new stats
							m_tSubTaskStats.IncreaseProcessedSize(pData->spSrcFile->GetLength64() - m_tSubTaskStats.GetCurrentItemProcessedSize());
							m_tSubTaskStats.IncreaseCurrentItemProcessedSize(pData->spSrcFile->GetLength64() - m_tSubTaskStats.GetCurrentItemProcessedSize());

							pData->bProcessed = false;
							return TSubTaskBase::eSubResult_Continue;
						}
					}
					else
					{
						eResult = FinalizeFileFB(spFeedbackHandler, fileDst, *pBuffer, pData->pathDstFile, bSkip);
						if (eResult != TSubTaskBase::eSubResult_Continue)
							return eResult;
						else if (bSkip)
						{
							// new stats
							m_tSubTaskStats.IncreaseProcessedSize(pData->spSrcFile->GetLength64() - m_tSubTaskStats.GetCurrentItemProcessedSize());
							m_tSubTaskStats.IncreaseCurrentItemProcessedSize(pData->spSrcFile->GetLength64() - m_tSubTaskStats.GetCurrentItemProcessedSize());

							pData->bProcessed = false;
							return TSubTaskBase::eSubResult_Continue;
						}

						unsigned long long ullCITotalSize = m_tSubTaskStats.GetCurrentItemTotalSize();
						unsigned long long ullCIProcessedSize = m_tSubTaskStats.GetCurrentItemProcessedSize();

						unsigned long long ullWritten = pBuffer->GetBytesTransferred();
						if(ullCIProcessedSize + ullWritten > ullCITotalSize)
						{
							// total size changed
							pData->spSrcFile->SetLength64(ullCIProcessedSize + ullWritten);
							m_tSubTaskStats.IncreaseCurrentItemTotalSize(ullCIProcessedSize + ullWritten - ullCITotalSize);
							m_tSubTaskStats.IncreaseTotalSize(ullCIProcessedSize + ullWritten - ullCITotalSize);
						}

						// new stats
						m_tSubTaskStats.IncreaseProcessedSize(ullWritten);
						m_tSubTaskStats.IncreaseCurrentItemProcessedSize(ullWritten);

						// stop iterating through file
						bStopProcessing = pBuffer->IsLastPart();
						pBuffer->RequeueAsEmpty();
					}

					break;
				}

			default:
				THROW_CORE_EXCEPTION(eErr_UnhandledCase);
			}
		}

		// fix the stats for files shorter than expected
		unsigned long long ullCITotalSize = m_tSubTaskStats.GetCurrentItemTotalSize();
		unsigned long long ullCIProcessedSize = m_tSubTaskStats.GetCurrentItemProcessedSize();
		if(ullCIProcessedSize < ullCITotalSize)
		{
			pData->spSrcFile->SetLength64(ullCIProcessedSize);
			m_tSubTaskStats.DecreaseCurrentItemTotalSize(ullCITotalSize - ullCIProcessedSize);
			m_tSubTaskStats.DecreaseTotalSize(ullCITotalSize - ullCIProcessedSize);
		}

		pData->bProcessed = true;
		m_tSubTaskStats.SetCurrentItemProcessedSize(0);

		pData->dbBuffer.WaitForMissingBuffers(rThreadController.GetKillThreadHandle());

		return TSubTaskBase::eSubResult_Continue;
	}

	TSubTaskCopyMove::ESubOperationResult TSubTaskCopyMove::OpenSrcAndDstFilesFB(const IFeedbackHandlerPtr& spFeedbackHandler, CUSTOM_COPY_PARAMS* pData,
		const IFilesystemFilePtr& spFileSrc, const IFilesystemFilePtr& spFileDst, bool bNoBuffer, bool& bSkip)
	{
		const TConfig& rConfig = GetContext().GetConfig();

		bSkip = false;

		// first open the source file and handle any failures
		TSubTaskCopyMove::ESubOperationResult eResult = OpenSourceFileFB(spFeedbackHandler, spFileSrc, bNoBuffer);
		if(eResult != TSubTaskBase::eSubResult_Continue)
			return eResult;
		else if(!spFileSrc->IsOpen())
		{
			// invalid handle = operation skipped by user
			unsigned long long ullDiff = pData->spSrcFile->GetLength64() - m_tSubTaskStats.GetCurrentItemProcessedSize();

			m_tSubTaskStats.IncreaseProcessedSize(ullDiff);
			m_tSubTaskStats.IncreaseCurrentItemProcessedSize(ullDiff);

			pData->bProcessed = false;
			bSkip = true;
			return TSubTaskBase::eSubResult_Continue;
		}

		// update the source file size (it might differ from the time this file was originally scanned).
		// NOTE: this kind of update could be also done when copying chunks of data beyond the original end-of-file,
		//       but it would require frequent total size updates and thus - serializations).
		// NOTE2: the by-chunk corrections of stats are still applied when copying to ensure even further size
		//        matching; this update however still allows for better serialization management.
		unsigned long long ullNewSize = spFileSrc->GetFileSize();
		unsigned long long ullOldSize = pData->spSrcFile->GetLength64();
		if(ullNewSize != ullOldSize)
		{
			if(ullNewSize > ullOldSize)
			{
				m_tSubTaskStats.IncreaseTotalSize(ullNewSize - ullOldSize);
				m_tSubTaskStats.IncreaseCurrentItemTotalSize(ullNewSize - ullOldSize);
			}
			else
			{
				m_tSubTaskStats.DecreaseTotalSize(ullOldSize - ullNewSize);
				m_tSubTaskStats.DecreaseCurrentItemTotalSize(ullOldSize - ullNewSize);
			}

			pData->spSrcFile->SetLength64(ullNewSize);
		}

		// change attributes of a dest file
		// NOTE: probably should be removed from here and report problems with read-only files
		//       directly to the user (as feedback request)
		if(!GetTaskPropValue<eTO_ProtectReadOnlyFiles>(rConfig))
			SetFileAttributes(pData->pathDstFile.ToString(), FILE_ATTRIBUTE_NORMAL);

		// open destination file, handle the failures and possibly existence of the destination file
		unsigned long long ullProcessedSize = m_tSubTaskStats.GetCurrentItemProcessedSize();
		unsigned long long ullSeekTo = 0;
		bool bDstFileFreshlyCreated = false;

		if (!m_tSubTaskStats.CanCurrentItemSilentResume())
		{
			// open destination file for case, when we start operation on this file (i.e. it is not resume of the
			// old operation)
			eResult = OpenDestinationFileFB(spFeedbackHandler, spFileDst, bNoBuffer, pData->spSrcFile, ullSeekTo, bDstFileFreshlyCreated);
			if(eResult != TSubTaskBase::eSubResult_Continue)
				return eResult;
			else if(!spFileDst->IsOpen())
			{
				unsigned long long ullDiff = pData->spSrcFile->GetLength64() - ullProcessedSize;

				m_tSubTaskStats.IncreaseProcessedSize(ullDiff);
				m_tSubTaskStats.IncreaseCurrentItemProcessedSize(ullDiff);

				pData->bProcessed = false;
				bSkip = true;
				return TSubTaskBase::eSubResult_Continue;
			}
		}
		else
		{
			// we are resuming previous operation
			eResult = OpenExistingDestinationFileFB(spFeedbackHandler, spFileDst, bNoBuffer);
			if(eResult != TSubTaskBase::eSubResult_Continue)
				return eResult;
			else if(!spFileDst->IsOpen())
			{
				unsigned long long ullDiff = pData->spSrcFile->GetLength64() - ullProcessedSize;

				m_tSubTaskStats.IncreaseProcessedSize(ullDiff);
				m_tSubTaskStats.IncreaseCurrentItemProcessedSize(ullDiff);

				pData->bProcessed = false;
				bSkip = true;
				return TSubTaskBase::eSubResult_Continue;
			}
		}

		if(pData->bOnlyCreate)
		{
			// we don't copy contents, but need to increase processed size
			unsigned long long ullDiff = pData->spSrcFile->GetLength64() - ullProcessedSize;

			m_tSubTaskStats.IncreaseProcessedSize(ullDiff);
			m_tSubTaskStats.IncreaseCurrentItemProcessedSize(ullDiff);

			return TSubTaskBase::eSubResult_Continue;
		}

		// seek to the position where copying will start
		ULONGLONG ullMove = (bNoBuffer ? RoundDown<unsigned long long>(ullSeekTo, IFilesystemFile::MaxSectorSize) : ullSeekTo);;
		if(ullMove != 0)		// src and dst files exists, requested resume at the specified index
		{
			// adjust the stats for the difference between what was already processed and what will now be considered processed
			if (ullMove > ullProcessedSize)
			{
				unsigned long long ullDiff = ullMove - ullProcessedSize;
				m_tSubTaskStats.IncreaseCurrentItemProcessedSize(ullDiff);
				m_tSubTaskStats.IncreaseProcessedSize(ullDiff);
			}
			else if (ullMove < ullProcessedSize)
			{
				unsigned long long ullDiff = ullProcessedSize - ullMove;
				m_tSubTaskStats.DecreaseCurrentItemProcessedSize(ullDiff);
				m_tSubTaskStats.DecreaseProcessedSize(ullDiff);
			}
		}

		// if the destination file already exists - truncate it to the current file position
		if(!bDstFileFreshlyCreated)
		{
			// if destination file was opened (as opposed to newly created)
			eResult = TruncateFileFB(spFeedbackHandler, spFileDst, ullMove, pData->pathDstFile, bSkip);
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
		icpf::log_file& rLog = GetContext().GetLog();

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

			rLog.logi(strFormat.c_str());

			rBuffer.ReinitializeBuffers(rBufferSizes.GetBufferCount(), rBufferSizes.GetMaxSize());

			return true;	// buffer adjusted
		}

		return false;	// buffer did not need adjusting
	}

	TSubTaskBase::ESubOperationResult TSubTaskCopyMove::OpenSourceFileFB(const IFeedbackHandlerPtr& spFeedbackHandler, const IFilesystemFilePtr& fileSrc, bool bNoBuffering)
	{
		icpf::log_file& rLog = GetContext().GetLog();

		bool bRetry = false;

		fileSrc->Close();

		do
		{
			bRetry = false;

			if(!fileSrc->OpenExistingForReading(bNoBuffering))
			{
				DWORD dwLastError = GetLastError();

				EFeedbackResult frResult = spFeedbackHandler->FileError(fileSrc->GetFilePath().ToWString(), TString(), EFileError::eCreateError, dwLastError);
				switch(frResult)
				{
				case EFeedbackResult::eResult_Skip:
					break;	// will return INVALID_HANDLE_VALUE

				case EFeedbackResult::eResult_Cancel:
					{
						// log
						TString strFormat = _T("Cancel request [error %errno] while opening source file %path (OpenSourceFileFB)");
						strFormat.Replace(_T("%errno"), boost::lexical_cast<std::wstring>(dwLastError).c_str());
						strFormat.Replace(_T("%path"), fileSrc->GetFilePath().ToString());
						rLog.loge(strFormat.c_str());

						return TSubTaskBase::eSubResult_CancelRequest;
					}

				case EFeedbackResult::eResult_Pause:
					return TSubTaskBase::eSubResult_PauseRequest;

				case EFeedbackResult::eResult_Retry:
					{
						// log
						TString strFormat = _T("Retrying [error %errno] to open source file %path (OpenSourceFileFB)");
						strFormat.Replace(_T("%errno"), boost::lexical_cast<std::wstring>(dwLastError).c_str());
						strFormat.Replace(_T("%path"), fileSrc->GetFilePath().ToString());
						rLog.loge(strFormat.c_str());

						bRetry = true;
						break;
					}

				default:
					BOOST_ASSERT(FALSE);		// unknown result
					THROW_CORE_EXCEPTION(eErr_UnhandledCase);
				}
			}
		}
		while(bRetry);

		return TSubTaskBase::eSubResult_Continue;
	}

	TSubTaskBase::ESubOperationResult TSubTaskCopyMove::OpenDestinationFileFB(const IFeedbackHandlerPtr& spFeedbackHandler, const IFilesystemFilePtr& fileDst, bool bNoBuffering, const TFileInfoPtr& spSrcFileInfo, unsigned long long& ullSeekTo, bool& bFreshlyCreated)
	{
		icpf::log_file& rLog = GetContext().GetLog();
		IFilesystemPtr spFilesystem = GetContext().GetLocalFilesystem();

		bool bRetry = false;

		ullSeekTo = 0;
		bFreshlyCreated = true;

		fileDst->Close();
		do
		{
			bRetry = false;

			if(!fileDst->CreateNewForWriting(bNoBuffering))
			{
				DWORD dwLastError = GetLastError();
				if(dwLastError == ERROR_FILE_EXISTS)
				{
					bFreshlyCreated = false;

					// pass it to the specialized method
					TSubTaskBase::ESubOperationResult eResult = OpenExistingDestinationFileFB(spFeedbackHandler, fileDst, bNoBuffering);
					if(eResult != TSubTaskBase::eSubResult_Continue)
						return eResult;
					else if(!fileDst->IsOpen())
						return TSubTaskBase::eSubResult_Continue;

					// read info about the existing destination file,
					// NOTE: it is not known which one would be faster - reading file parameters
					//       by using spDstFileInfo->Create() (which uses FindFirstFile()) or by
					//       reading parameters using opened handle; need to be tested in the future
					TFileInfoPtr spDstFileInfo(boost::make_shared<TFileInfo>());

					if(!spFilesystem->GetFileInfo(fileDst->GetFilePath(), spDstFileInfo))
						THROW_CORE_EXCEPTION_WIN32(eErr_CannotGetFileInfo, GetLastError());

					// src and dst files are the same
					EFeedbackResult frResult = spFeedbackHandler->FileAlreadyExists(spSrcFileInfo, spDstFileInfo);
					switch(frResult)
					{
					case EFeedbackResult::eResult_Overwrite:
						ullSeekTo = 0;
						break;

					case EFeedbackResult::eResult_CopyRest:
						ullSeekTo = spDstFileInfo->GetLength64();
						break;

					case EFeedbackResult::eResult_Skip:
						return TSubTaskBase::eSubResult_Continue;

					case EFeedbackResult::eResult_Cancel:
						{
							// log
							TString strFormat = _T("Cancel request while checking result of dialog before opening source file %path (CustomCopyFileFB)");
							strFormat.Replace(_T("%path"), fileDst->GetFilePath().ToString());
							rLog.logi(strFormat.c_str());

							return TSubTaskBase::eSubResult_CancelRequest;
						}
					case EFeedbackResult::eResult_Pause:
						return TSubTaskBase::eSubResult_PauseRequest;

					default:
						BOOST_ASSERT(FALSE);		// unknown result
						THROW_CORE_EXCEPTION(eErr_UnhandledCase);
					}
				}
				else
				{
					EFeedbackResult frResult = spFeedbackHandler->FileError(fileDst->GetFilePath().ToWString(), TString(), EFileError::eCreateError, dwLastError);
					switch(frResult)
					{
					case EFeedbackResult::eResult_Retry:
						{
							// log
							TString strFormat = _T("Retrying [error %errno] to open destination file %path (CustomCopyFileFB)");
							strFormat.Replace(_T("%errno"), boost::lexical_cast<std::wstring>(dwLastError).c_str());
							strFormat.Replace(_T("%path"), fileDst->GetFilePath().ToString());
							rLog.loge(strFormat.c_str());

							bRetry = true;

							break;
						}
					case EFeedbackResult::eResult_Cancel:
						{
							// log
							TString strFormat = _T("Cancel request [error %errno] while opening destination file %path (CustomCopyFileFB)");
							strFormat.Replace(_T("%errno"), boost::lexical_cast<std::wstring>(dwLastError).c_str());
							strFormat.Replace(_T("%path"), fileDst->GetFilePath().ToString());
							rLog.loge(strFormat.c_str());

							return TSubTaskBase::eSubResult_CancelRequest;
						}

					case EFeedbackResult::eResult_Skip:
						break;		// will return invalid handle value

					case EFeedbackResult::eResult_Pause:
						return TSubTaskBase::eSubResult_PauseRequest;

					default:
						BOOST_ASSERT(FALSE);		// unknown result
						THROW_CORE_EXCEPTION(eErr_UnhandledCase);
					}
				}
			}
		}
		while(bRetry);

		return TSubTaskBase::eSubResult_Continue;
	}

	TSubTaskBase::ESubOperationResult TSubTaskCopyMove::OpenExistingDestinationFileFB(const IFeedbackHandlerPtr& spFeedbackHandler, const IFilesystemFilePtr& fileDst, bool bNoBuffering)
	{
		icpf::log_file& rLog = GetContext().GetLog();

		bool bRetry = false;

		fileDst->Close();

		do
		{
			bRetry = false;

			if(!fileDst->OpenExistingForWriting(bNoBuffering))
			{
				DWORD dwLastError = GetLastError();

				EFeedbackResult frResult = spFeedbackHandler->FileError(fileDst->GetFilePath().ToWString(), TString(), EFileError::eCreateError, dwLastError);
				switch (frResult)
				{
				case EFeedbackResult::eResult_Retry:
					{
						// log
						TString strFormat = _T("Retrying [error %errno] to open destination file %path (CustomCopyFileFB)");
						strFormat.Replace(_T("%errno"), boost::lexical_cast<std::wstring>(dwLastError).c_str());
						strFormat.Replace(_t("%path"), fileDst->GetFilePath().ToString());
						rLog.loge(strFormat.c_str());

						bRetry = true;

						break;
					}
				case EFeedbackResult::eResult_Cancel:
					{
						// log
						TString strFormat = _T("Cancel request [error %errno] while opening destination file %path (CustomCopyFileFB)");
						strFormat.Replace(_T("%errno"), boost::lexical_cast<std::wstring>(dwLastError).c_str());
						strFormat.Replace(_T("%path"), fileDst->GetFilePath().ToString());
						rLog.loge(strFormat.c_str());

						return TSubTaskBase::eSubResult_CancelRequest;
					}

				case EFeedbackResult::eResult_Skip:
					break;		// will return invalid handle value

				case EFeedbackResult::eResult_Pause:
					return TSubTaskBase::eSubResult_PauseRequest;

				default:
					BOOST_ASSERT(FALSE);		// unknown result
					THROW_CORE_EXCEPTION(eErr_UnhandledCase);
				}
			}
		}
		while(bRetry);

		return TSubTaskBase::eSubResult_Continue;
	}

	TSubTaskBase::ESubOperationResult TSubTaskCopyMove::TruncateFileFB(const IFeedbackHandlerPtr& spFeedbackHandler, const IFilesystemFilePtr& spFile, long long llNewSize, 
		const TSmartPath& pathFile, bool& bSkip)
	{
		icpf::log_file& rLog = GetContext().GetLog();

		bSkip = false;

		bool bRetry = false;
		do
		{
			if(!spFile->Truncate(llNewSize))
			{
				// log
				DWORD dwLastError = GetLastError();

				TString strFormat = _T("Error %errno while truncating file %path to 0");
				strFormat.Replace(_t("%errno"), boost::lexical_cast<std::wstring>(dwLastError).c_str());
				strFormat.Replace(_t("%path"), pathFile.ToString());
				rLog.loge(strFormat.c_str());

				EFeedbackResult frResult = spFeedbackHandler->FileError(pathFile.ToWString(), TString(), EFileError::eResizeError, dwLastError);
				switch(frResult)
				{
				case EFeedbackResult::eResult_Cancel:
					return TSubTaskBase::eSubResult_CancelRequest;

				case EFeedbackResult::eResult_Retry:
					bRetry = true;

				case EFeedbackResult::eResult_Pause:
					return TSubTaskBase::eSubResult_PauseRequest;

				case EFeedbackResult::eResult_Skip:
					bSkip = true;
					return TSubTaskBase::eSubResult_Continue;

				default:
					BOOST_ASSERT(FALSE);		// unknown result
					THROW_CORE_EXCEPTION(eErr_UnhandledCase);
				}
			}
		}
		while(bRetry);

		return TSubTaskBase::eSubResult_Continue;
	}

	TSubTaskBase::ESubOperationResult TSubTaskCopyMove::ReadFileFB(const IFeedbackHandlerPtr& spFeedbackHandler, const IFilesystemFilePtr& spFile,
		TOverlappedDataBuffer& rBuffer, const TSmartPath& pathFile, bool& bSkip)
	{
		icpf::log_file& rLog = GetContext().GetLog();

		bSkip = false;
		bool bRetry = false;
		do
		{
			bRetry = false;

			if(!spFile->ReadFile(rBuffer))
			{
				TString strFormat = _T("Error %errno while requesting read of %count bytes from source file %path (CustomCopyFileFB)");
				strFormat.Replace(_t("%errno"), boost::lexical_cast<std::wstring>(GetLastError()).c_str());
				strFormat.Replace(_t("%count"), boost::lexical_cast<std::wstring>(rBuffer.GetRequestedDataSize()).c_str());
				strFormat.Replace(_t("%path"), pathFile.ToString());
				rLog.loge(strFormat.c_str());

				EFeedbackResult frResult = spFeedbackHandler->FileError(pathFile.ToWString(), TString(), EFileError::eReadError, GetLastError());
				switch(frResult)
				{
				case EFeedbackResult::eResult_Cancel:
					return TSubTaskBase::eSubResult_CancelRequest;

				case EFeedbackResult::eResult_Retry:
					bRetry = true;
					break;

				case EFeedbackResult::eResult_Pause:
					return TSubTaskBase::eSubResult_PauseRequest;

				case EFeedbackResult::eResult_Skip:
					bSkip = true;
					return TSubTaskBase::eSubResult_Continue;

				default:
					BOOST_ASSERT(FALSE);		// unknown result
					THROW_CORE_EXCEPTION(eErr_UnhandledCase);
				}
			}
		}
		while(bRetry);

		return TSubTaskBase::eSubResult_Continue;
	}

	TSubTaskBase::ESubOperationResult TSubTaskCopyMove::HandleReadError(const IFeedbackHandlerPtr& spFeedbackHandler,
		TOverlappedDataBuffer& rBuffer,
		const TSmartPath& pathFile,
		bool& bSkip)
	{
		icpf::log_file& rLog = GetContext().GetLog();
		DWORD dwLastError = rBuffer.GetErrorCode();

		bSkip = false;

		// log
		TString strFormat = _T("Error %errno while requesting read of %count bytes from source file %path (CustomCopyFileFB)");
		strFormat.Replace(_t("%errno"), boost::lexical_cast<std::wstring>(dwLastError).c_str());
		strFormat.Replace(_t("%count"), boost::lexical_cast<std::wstring>(rBuffer.GetRequestedDataSize()).c_str());
		strFormat.Replace(_t("%path"), pathFile.ToString());
		rLog.loge(strFormat.c_str());

		EFeedbackResult frResult = spFeedbackHandler->FileError(pathFile.ToWString(), TString(), EFileError::eReadError, dwLastError);
		switch(frResult)
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
			THROW_CORE_EXCEPTION(eErr_UnhandledCase);
		}
	}

	TSubTaskBase::ESubOperationResult TSubTaskCopyMove::WriteFileFB(const IFeedbackHandlerPtr& spFeedbackHandler, const IFilesystemFilePtr& spFile,
		TOverlappedDataBuffer& rBuffer, const TSmartPath& pathFile, bool& bSkip)
	{
		icpf::log_file& rLog = GetContext().GetLog();

		bSkip = false;

		bool bRetry = false;
		do
		{
			bRetry = false;

			if(!spFile->WriteFile(rBuffer))
			{
				// log
				DWORD dwLastError = GetLastError();

				TString strFormat = _T("Error %errno while trying to write %count bytes to destination file %path (CustomCopyFileFB)");
				strFormat.Replace(_t("%errno"), boost::lexical_cast<std::wstring>(dwLastError).c_str());
				strFormat.Replace(_t("%count"), boost::lexical_cast<std::wstring>(rBuffer.GetBytesTransferred()).c_str());
				strFormat.Replace(_t("%path"), pathFile.ToString());
				rLog.loge(strFormat.c_str());

				EFeedbackResult frResult = spFeedbackHandler->FileError(pathFile.ToWString(), TString(), EFileError::eWriteError, dwLastError);
				switch(frResult)
				{
				case EFeedbackResult::eResult_Cancel:
					return TSubTaskBase::eSubResult_CancelRequest;

				case EFeedbackResult::eResult_Retry:
					bRetry = true;
					break;

				case EFeedbackResult::eResult_Pause:
					return TSubTaskBase::eSubResult_PauseRequest;

				case EFeedbackResult::eResult_Skip:
					bSkip = true;
					return TSubTaskBase::eSubResult_Continue;

				default:
					BOOST_ASSERT(FALSE);		// unknown result
					THROW_CORE_EXCEPTION(eErr_UnhandledCase);
				}
			}
		}
		while(bRetry);

		return TSubTaskBase::eSubResult_Continue;
	}

	TSubTaskBase::ESubOperationResult TSubTaskCopyMove::HandleWriteError(const IFeedbackHandlerPtr& spFeedbackHandler,
		TOverlappedDataBuffer& rBuffer,
		const TSmartPath& pathFile,
		bool& bSkip)
	{
		icpf::log_file& rLog = GetContext().GetLog();
		DWORD dwLastError = rBuffer.GetErrorCode();

		bSkip = false;

		// log
		TString strFormat = _T("Error %errno while trying to write %count bytes to destination file %path (CustomCopyFileFB)");
		strFormat.Replace(_t("%errno"), boost::lexical_cast<std::wstring>(rBuffer.GetErrorCode()).c_str());
		strFormat.Replace(_t("%count"), boost::lexical_cast<std::wstring>(rBuffer.GetBytesTransferred()).c_str());
		strFormat.Replace(_t("%path"), pathFile.ToString());
		rLog.loge(strFormat.c_str());

		EFeedbackResult frResult = spFeedbackHandler->FileError(pathFile.ToWString(), TString(), EFileError::eWriteError, dwLastError);
		switch(frResult)
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
			THROW_CORE_EXCEPTION(eErr_UnhandledCase);
		}
	}

	TSubTaskBase::ESubOperationResult TSubTaskCopyMove::FinalizeFileFB(const IFeedbackHandlerPtr& spFeedbackHandler, const IFilesystemFilePtr& spFile,
		TOverlappedDataBuffer& rBuffer, const TSmartPath& pathFile, bool& bSkip)
	{
		icpf::log_file& rLog = GetContext().GetLog();

		bSkip = false;

		bool bRetry = false;
		do
		{
			bRetry = false;

			if (!spFile->FinalizeFile(rBuffer))
			{
				// log
				DWORD dwLastError = GetLastError();

				TString strFormat = _T("Error %errno while trying to finalize file %path (CustomCopyFileFB)");
				strFormat.Replace(_t("%errno"), boost::lexical_cast<std::wstring>(dwLastError).c_str());
				strFormat.Replace(_t("%path"), pathFile.ToString());
				rLog.loge(strFormat.c_str());

				EFeedbackResult frResult = spFeedbackHandler->FileError(pathFile.ToWString(), TString(), EFileError::eFinalizeError, dwLastError);
				switch (frResult)
				{
				case EFeedbackResult::eResult_Cancel:
					return TSubTaskBase::eSubResult_CancelRequest;

				case EFeedbackResult::eResult_Retry:
					bRetry = true;
					break;

				case EFeedbackResult::eResult_Pause:
					return TSubTaskBase::eSubResult_PauseRequest;

				case EFeedbackResult::eResult_Skip:
					bSkip = true;
					return TSubTaskBase::eSubResult_Continue;

				default:
					BOOST_ASSERT(FALSE);		// unknown result
					THROW_CORE_EXCEPTION(eErr_UnhandledCase);
				}
			}
		}
		while (bRetry);

		return TSubTaskBase::eSubResult_Continue;
	}

	TSubTaskBase::ESubOperationResult TSubTaskCopyMove::CreateDirectoryFB(const IFeedbackHandlerPtr& spFeedbackHandler, const TSmartPath& pathDirectory)
	{
		icpf::log_file& rLog = GetContext().GetLog();
		IFilesystemPtr spFilesystem = GetContext().GetLocalFilesystem();

		bool bRetry = true;
		DWORD dwLastError = ERROR_SUCCESS;
		while(bRetry && !spFilesystem->CreateDirectory(pathDirectory, false) && (dwLastError = GetLastError()) != ERROR_ALREADY_EXISTS)
		{
			// log
			TString strFormat;
			strFormat = _T("Error %errno while calling CreateDirectory %path (ProcessFiles)");
			strFormat.Replace(_T("%errno"), boost::lexical_cast<std::wstring>(dwLastError).c_str());
			strFormat.Replace(_T("%path"), pathDirectory.ToString());
			rLog.loge(strFormat.c_str());

			EFeedbackResult frResult = spFeedbackHandler->FileError(pathDirectory.ToWString(), TString(), EFileError::eCreateError, dwLastError);
			switch(frResult)
			{
			case EFeedbackResult::eResult_Cancel:
				return TSubTaskBase::eSubResult_CancelRequest;

			case EFeedbackResult::eResult_Retry:
				bRetry = false;
				break;

			case EFeedbackResult::eResult_Pause:
				return TSubTaskBase::eSubResult_PauseRequest;

			case EFeedbackResult::eResult_Skip:
				bRetry = false;
				break;		// just do nothing

			default:
				BOOST_ASSERT(FALSE);		// unknown result
				THROW_CORE_EXCEPTION(eErr_UnhandledCase);
			}
		}

		return TSubTaskBase::eSubResult_Continue;
	}

	TSubTaskBase::ESubOperationResult TSubTaskCopyMove::CheckForFreeSpaceFB(const IFeedbackHandlerPtr& spFeedbackHandler)
	{
		icpf::log_file& rLog = GetContext().GetLog();
		IFilesystemPtr spFilesystem = GetContext().GetLocalFilesystem();
		TFileInfoArray& rFilesCache = GetContext().GetFilesCache();
		TBasePathDataContainerPtr spSrcPaths = GetContext().GetBasePaths();
		TSmartPath pathDestination = GetContext().GetDestinationPath();

		ull_t ullNeededSize = 0, ullAvailableSize = 0;
		bool bRetry = false;

		do
		{
			bRetry = false;

			rLog.logi(_T("Checking for free space on destination disk..."));

			ullNeededSize = rFilesCache.CalculateTotalSize() - rFilesCache.CalculatePartialSize(m_tSubTaskStats.GetCurrentIndex()); // it'd be nice to round up to take cluster size into consideration

			// get free space
			bool bResult = spFilesystem->GetDynamicFreeSpace(pathDestination, ullAvailableSize);
			if(bResult && ullNeededSize > ullAvailableSize)
			{
				TString strFormat = _T("Not enough free space on disk - needed %needsize bytes for data, available: %availablesize bytes.");
				strFormat.Replace(_t("%needsize"), boost::lexical_cast<std::wstring>(ullNeededSize).c_str());
				strFormat.Replace(_t("%availablesize"), boost::lexical_cast<std::wstring>(ullAvailableSize).c_str());
				rLog.logw(strFormat.c_str());

				if(!spSrcPaths->IsEmpty())
				{
					EFeedbackResult frResult = spFeedbackHandler->NotEnoughSpace(spSrcPaths->GetAt(0)->GetSrcPath().ToWString(), pathDestination.ToWString(), ullNeededSize);
					switch(frResult)
					{
					case EFeedbackResult::eResult_Cancel:
						rLog.logi(_T("Cancel request while checking for free space on disk."));
						return TSubTaskBase::eSubResult_CancelRequest;

					case EFeedbackResult::eResult_Retry:
						rLog.logi(_T("Retrying to read drive's free space..."));
						bRetry = true;
						break;

					case EFeedbackResult::eResult_Ignore:
						rLog.logi(_T("Ignored warning about not enough place on disk to copy data."));
						return TSubTaskBase::eSubResult_Continue;

					default:
						BOOST_ASSERT(FALSE);		// unknown result
						THROW_CORE_EXCEPTION(eErr_UnhandledCase);
					}
				}
			}
		}
		while(bRetry);

		return TSubTaskBase::eSubResult_Continue;
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
