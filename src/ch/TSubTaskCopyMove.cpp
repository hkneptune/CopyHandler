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
#include "../libchcore/TTaskDefinition.h"
#include "task.h"
#include "TLocalFilesystem.h"
#include "FeedbackHandler.h"

// assume max sectors of 4kB (for rounding)
#define MAXSECTORSIZE			4096

struct CUSTOM_COPY_PARAMS
{
	CFileInfoPtr spSrcFile;		// CFileInfo - src file
	chcore::TSmartPath pathDstFile;			// dest path with filename

	CDataBuffer dbBuffer;		// buffer handling
	bool bOnlyCreate;			// flag from configuration - skips real copying - only create
	bool bProcessed;			// has the element been processed ? (false if skipped)
};

TSubTaskCopyMove::TSubTaskCopyMove(TSubTaskContext& tSubTaskContext) :
TSubTaskBase(tSubTaskContext)
{
}

TSubTaskBase::ESubOperationResult TSubTaskCopyMove::Exec()
{
	icpf::log_file& rLog = GetContext().GetLog();
	CFileInfoArray& rFilesCache = GetContext().GetFilesCache();
	chcore::TTaskDefinition& rTaskDefinition = GetContext().GetTaskDefinition();
	TTaskConfigTracker& rCfgTracker = GetContext().GetCfgTracker();
	TTaskBasicProgressInfo& rBasicProgressInfo = GetContext().GetTaskBasicProgressInfo();
	TWorkerThreadController& rThreadController = GetContext().GetThreadController();
	chcore::IFeedbackHandler* piFeedbackHandler = GetContext().GetFeedbackHandler();
	TTaskLocalStats& rLocalStats = GetContext().GetTaskLocalStats();

	BOOST_ASSERT(piFeedbackHandler != NULL);
	if(piFeedbackHandler == NULL)
		return eSubResult_Error;

	// log
	rLog.logi(_T("Processing files/folders (ProcessFiles)"));

	// count how much has been done (updates also a member in TSubTaskCopyMoveArray)
	rLocalStats.SetProcessedSize(rFilesCache.CalculatePartialSize(rBasicProgressInfo.GetCurrentIndex()));

	// begin at index which wasn't processed previously
	size_t stSize = rFilesCache.GetSize();
	bool bIgnoreFolders = GetTaskPropValue<eTO_IgnoreDirectories>(rTaskDefinition.GetConfiguration());
	bool bForceDirectories = GetTaskPropValue<eTO_CreateDirectoriesRelativeToRoot>(rTaskDefinition.GetConfiguration());

	// create a buffer of size m_nBufferSize
	CUSTOM_COPY_PARAMS ccp;
	ccp.bProcessed = false;
	ccp.bOnlyCreate = GetTaskPropValue<eTO_CreateEmptyFiles>(rTaskDefinition.GetConfiguration());

	// remove changes in buffer sizes to avoid re-creation later
	rCfgTracker.RemoveModificationSet(TOptionsSet() % eTO_DefaultBufferSize % eTO_OneDiskBufferSize % eTO_TwoDisksBufferSize % eTO_CDBufferSize % eTO_LANBufferSize % eTO_UseOnlyDefaultBuffer);

	BUFFERSIZES bs;
	bs.m_bOnlyDefault = GetTaskPropValue<eTO_UseOnlyDefaultBuffer>(rTaskDefinition.GetConfiguration());
	bs.m_uiDefaultSize = GetTaskPropValue<eTO_DefaultBufferSize>(rTaskDefinition.GetConfiguration());
	bs.m_uiOneDiskSize = GetTaskPropValue<eTO_OneDiskBufferSize>(rTaskDefinition.GetConfiguration());
	bs.m_uiTwoDisksSize = GetTaskPropValue<eTO_TwoDisksBufferSize>(rTaskDefinition.GetConfiguration());
	bs.m_uiCDSize = GetTaskPropValue<eTO_CDBufferSize>(rTaskDefinition.GetConfiguration());
	bs.m_uiLANSize = GetTaskPropValue<eTO_LANBufferSize>(rTaskDefinition.GetConfiguration());

	ccp.dbBuffer.Create(&bs);

	// helpers
	DWORD dwLastError = 0;

	// log
	const BUFFERSIZES* pbs = ccp.dbBuffer.GetSizes();

	ictranslate::CFormat fmt;
	fmt.SetFormat(_T("Processing files/folders (ProcessFiles):\r\n\tOnlyCreate: %create\r\n\tBufferSize: [Def:%defsize, One:%onesize, Two:%twosize, CD:%cdsize, LAN:%lansize]\r\n\tFiles/folders count: %filecount\r\n\tIgnore Folders: %ignorefolders\r\n\tDest path: %dstpath\r\n\tCurrent index (0-based): %currindex"));
	fmt.SetParam(_t("%create"), ccp.bOnlyCreate);
	fmt.SetParam(_t("%defsize"), pbs->m_uiDefaultSize);
	fmt.SetParam(_t("%onesize"), pbs->m_uiOneDiskSize);
	fmt.SetParam(_t("%twosize"), pbs->m_uiTwoDisksSize);
	fmt.SetParam(_t("%cdsize"), pbs->m_uiCDSize);
	fmt.SetParam(_t("%lansize"), pbs->m_uiLANSize);
	fmt.SetParam(_t("%filecount"), stSize);
	fmt.SetParam(_t("%ignorefolders"), bIgnoreFolders);
	fmt.SetParam(_t("%dstpath"), rTaskDefinition.GetDestinationPath().ToString());
	fmt.SetParam(_t("%currindex"), rBasicProgressInfo.GetCurrentIndex());

	rLog.logi(fmt);

	for(size_t stIndex = rBasicProgressInfo.GetCurrentIndex(); stIndex < stSize; stIndex++)
	{
		// should we kill ?
		if(rThreadController.KillRequested())
		{
			// log
			rLog.logi(_T("Kill request while processing file in ProcessFiles"));
			return TSubTaskBase::eSubResult_KillRequest;
		}

		// update m_stNextIndex, getting current CFileInfo
		CFileInfoPtr spFileInfo = rFilesCache.GetAt(rBasicProgressInfo.GetCurrentIndex());

		// set dest path with filename
		ccp.pathDstFile = CalculateDestinationPath(spFileInfo, rTaskDefinition.GetDestinationPath(), ((int)bForceDirectories) << 1 | (int)bIgnoreFolders);

		// are the files/folders lie on the same partition ?
		wchar_t wchDestinationDrive = rTaskDefinition.GetDestinationPath().GetDriveLetter();
		bool bMove = rTaskDefinition.GetOperationType() == chcore::eOperation_Move;
		chcore::TSmartPath pathCurrent = spFileInfo->GetFullFilePath();
		if(bMove && wchDestinationDrive != L'\0' && wchDestinationDrive == pathCurrent.GetDriveLetter() && GetMove(spFileInfo))
		{
			bool bRetry = true;
			if(bRetry && !TLocalFilesystem::FastMove(pathCurrent, ccp.pathDstFile))
			{
				dwLastError=GetLastError();
				//log
				fmt.SetFormat(_T("Error %errno while calling MoveFile %srcpath -> %dstpath (ProcessFiles)"));
				fmt.SetParam(_t("%errno"), dwLastError);
				fmt.SetParam(_t("%srcpath"), spFileInfo->GetFullFilePath().ToString());
				fmt.SetParam(_t("%dstpath"), ccp.pathDstFile.ToString());
				rLog.loge(fmt);

				FEEDBACK_FILEERROR ferr = { spFileInfo->GetFullFilePath().ToString(), ccp.pathDstFile.ToString(), eFastMoveError, dwLastError };
				CFeedbackHandler::EFeedbackResult frResult = (CFeedbackHandler::EFeedbackResult)piFeedbackHandler->RequestFeedback(CFeedbackHandler::eFT_FileError, &ferr);
				switch(frResult)
				{
				case CFeedbackHandler::eResult_Cancel:
					return TSubTaskBase::eSubResult_CancelRequest;

				case CFeedbackHandler::eResult_Retry:
					continue;

				case CFeedbackHandler::eResult_Pause:
					return TSubTaskBase::eSubResult_PauseRequest;

				case CFeedbackHandler::eResult_Skip:
					bRetry = false;
					break;		// just do nothing
				default:
					BOOST_ASSERT(FALSE);		// unknown result
					THROW(_T("Unhandled case"), 0, 0, 0);
				}
			}
			else
				spFileInfo->SetFlags(FIF_PROCESSED, FIF_PROCESSED);
		}
		else
		{
			// if folder - create it
			if(spFileInfo->IsDirectory())
			{
				bool bRetry = true;
				if(bRetry && !TLocalFilesystem::CreateDirectory(ccp.pathDstFile, false) && (dwLastError=GetLastError()) != ERROR_ALREADY_EXISTS )
				{
					// log
					fmt.SetFormat(_T("Error %errno while calling CreateDirectory %path (ProcessFiles)"));
					fmt.SetParam(_t("%errno"), dwLastError);
					fmt.SetParam(_t("%path"), ccp.pathDstFile.ToString());
					rLog.loge(fmt);

					FEEDBACK_FILEERROR ferr = { ccp.pathDstFile.ToString(), NULL, eCreateError, dwLastError };
					CFeedbackHandler::EFeedbackResult frResult = (CFeedbackHandler::EFeedbackResult)piFeedbackHandler->RequestFeedback(CFeedbackHandler::eFT_FileError, &ferr);
					switch(frResult)
					{
					case CFeedbackHandler::eResult_Cancel:
						return TSubTaskBase::eSubResult_CancelRequest;

					case CFeedbackHandler::eResult_Retry:
						continue;

					case CFeedbackHandler::eResult_Pause:
						return TSubTaskBase::eSubResult_PauseRequest;

					case CFeedbackHandler::eResult_Skip:
						bRetry = false;
						break;		// just do nothing
					default:
						BOOST_ASSERT(FALSE);		// unknown result
						THROW(_T("Unhandled case"), 0, 0, 0);
					}
				}

				rLocalStats.IncreaseProcessedSize(spFileInfo->GetLength64());
				spFileInfo->SetFlags(FIF_PROCESSED, FIF_PROCESSED);
			}
			else
			{
				// start copying/moving file
				ccp.spSrcFile = spFileInfo;
				ccp.bProcessed = false;

				// copy data
				TSubTaskBase::ESubOperationResult eResult = CustomCopyFileFB(&ccp);
				if(eResult != TSubTaskBase::eSubResult_Continue)
					return eResult;

				spFileInfo->SetFlags(ccp.bProcessed ? FIF_PROCESSED : 0, FIF_PROCESSED);

				// if moving - delete file (only if config flag is set)
				if(bMove && spFileInfo->GetFlags() & FIF_PROCESSED && !GetTaskPropValue<eTO_DeleteInSeparateSubTask>(rTaskDefinition.GetConfiguration()))
				{
					if(!GetTaskPropValue<eTO_ProtectReadOnlyFiles>(rTaskDefinition.GetConfiguration()))
						TLocalFilesystem::SetAttributes(spFileInfo->GetFullFilePath(), FILE_ATTRIBUTE_NORMAL);
					TLocalFilesystem::DeleteFile(spFileInfo->GetFullFilePath());	// there will be another try later, so I don't check
					// if succeeded
				}
			}

			// set a time
			if(GetTaskPropValue<eTO_SetDestinationDateTime>(rTaskDefinition.GetConfiguration()))
				TLocalFilesystem::SetFileDirectoryTime(ccp.pathDstFile, spFileInfo->GetCreationTime(), spFileInfo->GetLastAccessTime(), spFileInfo->GetLastWriteTime()); // no error checking (but most probably it should be checked)

			// attributes
			if(GetTaskPropValue<eTO_SetDestinationAttributes>(rTaskDefinition.GetConfiguration()))
				TLocalFilesystem::SetAttributes(ccp.pathDstFile, spFileInfo->GetAttributes());	// as above
		}

		rBasicProgressInfo.SetCurrentIndex(stIndex + 1);
	}

	// delete buffer - it's not needed
	ccp.dbBuffer.Delete();

	// to look better (as 100%) - increase current index by 1
	rBasicProgressInfo.SetCurrentIndex(stSize);

	// log
	rLog.logi(_T("Finished processing in ProcessFiles"));

	return TSubTaskBase::eSubResult_Continue;
}

bool TSubTaskCopyMove::GetMove(const CFileInfoPtr& spFileInfo)
{
	if(!spFileInfo)
		THROW(_T("Invalid pointer"), 0, 0, 0);
	if(spFileInfo->GetSrcIndex() == std::numeric_limits<size_t>::max())
		THROW(_T("Received non-relative (standalone) path info"), 0, 0, 0);

	// check if this information has already been stored
	size_t stBaseIndex = spFileInfo->GetSrcIndex();
	if(stBaseIndex >= GetContext().GetBasePathDataContainer().GetCount())
		THROW(_T("Index out of bounds"), 0, 0, 0);

	TBasePathDataPtr spPathData = GetContext().GetBasePathDataContainer().GetAt(stBaseIndex);
	return spPathData->GetMove();
}

int TSubTaskCopyMove::GetBufferIndex(const CFileInfoPtr& spFileInfo)
{
	if(!spFileInfo)
		THROW(_T("Invalid pointer"), 0, 0, 0);

	chcore::TSmartPath pathSource = spFileInfo->GetFullFilePath();
	chcore::TSmartPath pathDestination = GetContext().GetTaskDefinition().GetDestinationPath();

	TLocalFilesystem::EPathsRelation eRelation = GetContext().GetLocalFilesystem().GetPathsRelation(pathSource, pathDestination);
	switch(eRelation)
	{
	case TLocalFilesystem::eRelation_Network:
		return BI_LAN;

	case TLocalFilesystem::eRelation_CDRom:
		return BI_CD;

	case TLocalFilesystem::eRelation_TwoPhysicalDisks:
		return BI_TWODISKS;

	case TLocalFilesystem::eRelation_SinglePhysicalDisk:
		return BI_ONEDISK;

	//case eRelation_Other:
	default:
		return BI_DEFAULT;
	}
}

TSubTaskBase::ESubOperationResult TSubTaskCopyMove::CustomCopyFileFB(CUSTOM_COPY_PARAMS* pData)
{
	chcore::TTaskDefinition& rTaskDefinition = GetContext().GetTaskDefinition();
	TTaskBasicProgressInfo& rBasicProgressInfo = GetContext().GetTaskBasicProgressInfo();
	TWorkerThreadController& rThreadController = GetContext().GetThreadController();
	TTaskLocalStats& rLocalStats = GetContext().GetTaskLocalStats();
	icpf::log_file& rLog = GetContext().GetLog();
	TTaskConfigTracker& rCfgTracker = GetContext().GetCfgTracker();

	TLocalFilesystemFile fileSrc = TLocalFilesystem::CreateFileObject();
	TLocalFilesystemFile fileDst = TLocalFilesystem::CreateFileObject();

	ictranslate::CFormat fmt;
	TSubTaskBase::ESubOperationResult eResult = TSubTaskBase::eSubResult_Continue;
	bool bSkip = false;

	// calculate if we want to disable buffering for file transfer
	// NOTE: we are using here the file size read when scanning directories for files; it might be
	//       outdated at this point, but at present we don't want to re-read file size since it
	//       will cost additional disk access
	bool bNoBuffer = (GetTaskPropValue<eTO_DisableBuffering>(rTaskDefinition.GetConfiguration()) &&
		pData->spSrcFile->GetLength64() >= GetTaskPropValue<eTO_DisableBufferingMinSize>(rTaskDefinition.GetConfiguration()));

	// first open the source file and handle any failures
	eResult = OpenSourceFileFB(fileSrc, pData->spSrcFile->GetFullFilePath(), bNoBuffer);
	if(eResult != TSubTaskBase::eSubResult_Continue)
		return eResult;
	else if(!fileSrc.IsOpen())
	{
		// invalid handle = operation skipped by user
		rLocalStats.IncreaseProcessedSize(pData->spSrcFile->GetLength64() - rBasicProgressInfo.GetCurrentFileProcessedSize());
		pData->bProcessed = false;
		return TSubTaskBase::eSubResult_Continue;
	}

	// change attributes of a dest file
	// NOTE: probably should be removed from here and report problems with read-only files
	//       directly to the user (as feedback request)
	if(!GetTaskPropValue<eTO_ProtectReadOnlyFiles>(rTaskDefinition.GetConfiguration()))
		SetFileAttributes(pData->pathDstFile.ToString(), FILE_ATTRIBUTE_NORMAL);

	// open destination file, handle the failures and possibly existence of the destination file
	unsigned long long ullSeekTo = 0;
	bool bDstFileFreshlyCreated = false;

	if(rBasicProgressInfo.GetCurrentFileProcessedSize() == 0)
	{
		// open destination file for case, when we start operation on this file (i.e. it is not resume of the
		// old operation)
		eResult = OpenDestinationFileFB(fileDst, pData->pathDstFile, bNoBuffer, pData->spSrcFile, ullSeekTo, bDstFileFreshlyCreated);
		if(eResult != TSubTaskBase::eSubResult_Continue)
			return eResult;
		else if(!fileDst.IsOpen())
		{
			rLocalStats.IncreaseProcessedSize(pData->spSrcFile->GetLength64() - rBasicProgressInfo.GetCurrentFileProcessedSize());
			pData->bProcessed = false;
			return TSubTaskBase::eSubResult_Continue;
		}
	}
	else
	{
		// we are resuming previous operation
		eResult = OpenExistingDestinationFileFB(fileDst, pData->pathDstFile, bNoBuffer);
		if(eResult != TSubTaskBase::eSubResult_Continue)
			return eResult;
		else if(!fileDst.IsOpen())
		{
			rLocalStats.IncreaseProcessedSize(pData->spSrcFile->GetLength64() - rBasicProgressInfo.GetCurrentFileProcessedSize());
			pData->bProcessed = false;
			return TSubTaskBase::eSubResult_Continue;
		}

		ullSeekTo = rBasicProgressInfo.GetCurrentFileProcessedSize();
	}

	if(!pData->bOnlyCreate)
	{
		// seek to the position where copying will start
		if(ullSeekTo != 0)		// src and dst files exists, requested resume at the specified index
		{
			// try to move file pointers to the end
			ULONGLONG ullMove = (bNoBuffer ? ROUNDDOWN(ullSeekTo, MAXSECTORSIZE) : ullSeekTo);

			eResult = SetFilePointerFB(fileSrc, ullMove, pData->spSrcFile->GetFullFilePath(), bSkip);
			if(eResult != TSubTaskBase::eSubResult_Continue)
				return eResult;
			else if(bSkip)
			{
				rLocalStats.IncreaseProcessedSize(pData->spSrcFile->GetLength64() - rBasicProgressInfo.GetCurrentFileProcessedSize());
				pData->bProcessed = false;
				return TSubTaskBase::eSubResult_Continue;
			}

			eResult = SetFilePointerFB(fileDst, ullMove, pData->pathDstFile, bSkip);
			if(eResult != TSubTaskBase::eSubResult_Continue)
				return eResult;
			else if(bSkip)
			{
				// with either first or second seek we got 'skip' answer...
				rLocalStats.IncreaseProcessedSize(pData->spSrcFile->GetLength64() - rBasicProgressInfo.GetCurrentFileProcessedSize());
				pData->bProcessed = false;
				return TSubTaskBase::eSubResult_Continue;
			}

			rBasicProgressInfo.IncreaseCurrentFileProcessedSize(ullMove);
			rLocalStats.IncreaseProcessedSize(ullMove);
		}

		// if the destination file already exists - truncate it to the current file position
		if(!bDstFileFreshlyCreated)
		{
			// if destination file was opened (as opposed to newly created)
			eResult = SetEndOfFileFB(fileDst, pData->pathDstFile, bSkip);
			if(eResult != TSubTaskBase::eSubResult_Continue)
				return eResult;
			else if(bSkip)
			{
				pData->bProcessed = false;
				return TSubTaskBase::eSubResult_Continue;
			}
		}

		// copying
		unsigned long ulToRead = 0;
		unsigned long ulRead = 0;
		unsigned long ulWritten = 0;
		int iBufferIndex = 0;
		bool bLastPart = false;

		do
		{
			// kill flag checks
			if(rThreadController.KillRequested())
			{
				// log
				fmt.SetFormat(_T("Kill request while main copying file %srcpath -> %dstpath"));
				fmt.SetParam(_t("%srcpath"), pData->spSrcFile->GetFullFilePath().ToString());
				fmt.SetParam(_t("%dstpath"), pData->pathDstFile.ToString());
				rLog.logi(fmt);
				return TSubTaskBase::eSubResult_KillRequest;
			}

			// recreate buffer if needed
			if(rCfgTracker.IsModified() && rCfgTracker.IsModified(TOptionsSet() % eTO_DefaultBufferSize % eTO_OneDiskBufferSize % eTO_TwoDisksBufferSize % eTO_CDBufferSize % eTO_LANBufferSize % eTO_UseOnlyDefaultBuffer, true))
			{
				BUFFERSIZES bs;
				bs.m_bOnlyDefault = GetTaskPropValue<eTO_UseOnlyDefaultBuffer>(rTaskDefinition.GetConfiguration());
				bs.m_uiDefaultSize = GetTaskPropValue<eTO_DefaultBufferSize>(rTaskDefinition.GetConfiguration());
				bs.m_uiOneDiskSize = GetTaskPropValue<eTO_OneDiskBufferSize>(rTaskDefinition.GetConfiguration());
				bs.m_uiTwoDisksSize = GetTaskPropValue<eTO_TwoDisksBufferSize>(rTaskDefinition.GetConfiguration());
				bs.m_uiCDSize = GetTaskPropValue<eTO_CDBufferSize>(rTaskDefinition.GetConfiguration());
				bs.m_uiLANSize = GetTaskPropValue<eTO_LANBufferSize>(rTaskDefinition.GetConfiguration());

				// log
				const BUFFERSIZES* pbs1 = pData->dbBuffer.GetSizes();

				fmt.SetFormat(_T("Changing buffer size from [Def:%defsize, One:%onesize, Two:%twosize, CD:%cdsize, LAN:%lansize] to [Def:%defsize2, One:%onesize2, Two:%twosize2, CD:%cdsize2, LAN:%lansize2] wile copying %srcfile -> %dstfile (CustomCopyFileFB)"));

				fmt.SetParam(_t("%defsize"), pbs1->m_uiDefaultSize);
				fmt.SetParam(_t("%onesize"), pbs1->m_uiOneDiskSize);
				fmt.SetParam(_t("%twosize"), pbs1->m_uiTwoDisksSize);
				fmt.SetParam(_t("%cdsize"), pbs1->m_uiCDSize);
				fmt.SetParam(_t("%lansize"), pbs1->m_uiLANSize);
				fmt.SetParam(_t("%defsize2"), bs.m_uiDefaultSize);
				fmt.SetParam(_t("%onesize2"), bs.m_uiOneDiskSize);
				fmt.SetParam(_t("%twosize2"), bs.m_uiTwoDisksSize);
				fmt.SetParam(_t("%cdsize2"), bs.m_uiCDSize);
				fmt.SetParam(_t("%lansize2"), bs.m_uiLANSize);
				fmt.SetParam(_t("%srcfile"), pData->spSrcFile->GetFullFilePath().ToString());
				fmt.SetParam(_t("%dstfile"), pData->pathDstFile.ToString());

				rLog.logi(fmt);
				pData->dbBuffer.Create(&bs);
			}

			// establish count of data to read
			if(GetTaskPropValue<eTO_UseOnlyDefaultBuffer>(rTaskDefinition.GetConfiguration()))
				iBufferIndex = BI_DEFAULT;
			else
				iBufferIndex = GetBufferIndex(pData->spSrcFile);
			rLocalStats.SetCurrentBufferIndex(iBufferIndex);

			ulToRead = bNoBuffer ? ROUNDUP(pData->dbBuffer.GetSizes()->m_auiSizes[iBufferIndex], MAXSECTORSIZE) : pData->dbBuffer.GetSizes()->m_auiSizes[iBufferIndex];

			// read data from file to buffer
			eResult = ReadFileFB(fileSrc, pData->dbBuffer, ulToRead, ulRead, pData->spSrcFile->GetFullFilePath(), bSkip);
			if(eResult != TSubTaskBase::eSubResult_Continue)
				return eResult;
			else if(bSkip)
			{
				rLocalStats.IncreaseProcessedSize(pData->spSrcFile->GetLength64() - rBasicProgressInfo.GetCurrentFileProcessedSize());
				pData->bProcessed = false;
				return TSubTaskBase::eSubResult_Continue;
			}

			if(ulRead > 0)
			{
				// determine if this is the last chunk of data we could get from the source file (EOF condition)
				bLastPart = (ulToRead != ulRead);

				// handle not aligned part at the end of file when no buffering is enabled
				if(bNoBuffer && bLastPart)
				{
					// count of data read from the file is less than requested - we're at the end of source file
					// and this is the operation with system buffering turned off

					// write as much as possible to the destination file with no buffering
					// NOTE: as an alternative, we could write more data to the destination file and then truncate the file
					unsigned long ulDataToWrite = ROUNDDOWN(ulRead, MAXSECTORSIZE);
					if(ulDataToWrite > 0)
					{
						eResult = WriteFileFB(fileDst, pData->dbBuffer, ulDataToWrite, ulWritten, pData->pathDstFile, bSkip);
						if(eResult != TSubTaskBase::eSubResult_Continue)
							return eResult;
						else if(bSkip)
						{
							rLocalStats.IncreaseProcessedSize(pData->spSrcFile->GetLength64() - rBasicProgressInfo.GetCurrentFileProcessedSize());
							pData->bProcessed = false;
							return TSubTaskBase::eSubResult_Continue;
						}

						// increase count of processed data
						rBasicProgressInfo.IncreaseCurrentFileProcessedSize(ulWritten);
						rLocalStats.IncreaseProcessedSize(ulWritten);

						// calculate count of bytes left to be written
						ulRead -= ulWritten;

						// now remove part of data from buffer (ulWritten bytes)
						pData->dbBuffer.CutDataFromBuffer(ulWritten);
					}

					// close and re-open the destination file with buffering option for append
					fileDst.Close();

					// are there any more data to be written?
					if(ulRead != 0)
					{
						// re-open the destination file, this time with standard buffering to allow writing not aligned part of file data
						eResult = OpenExistingDestinationFileFB(fileDst, pData->pathDstFile, false);
						if(eResult != TSubTaskBase::eSubResult_Continue)
							return eResult;
						else if(!fileDst.IsOpen())
						{
							rLocalStats.IncreaseProcessedSize(pData->spSrcFile->GetLength64() - rBasicProgressInfo.GetCurrentFileProcessedSize());
							pData->bProcessed = false;
							return TSubTaskBase::eSubResult_Continue;
						}

						// move file pointer to the end of destination file
						eResult = SetFilePointerFB(fileDst, rBasicProgressInfo.GetCurrentFileProcessedSize(), pData->pathDstFile, bSkip);
						if(eResult != TSubTaskBase::eSubResult_Continue)
							return eResult;
						else if(bSkip)
						{
							// with either first or second seek we got 'skip' answer...
							rLocalStats.IncreaseProcessedSize(pData->spSrcFile->GetLength64() - rBasicProgressInfo.GetCurrentFileProcessedSize());
							pData->bProcessed = false;
							return TSubTaskBase::eSubResult_Continue;
						}
					}
				}

				// write
				if(ulRead != 0)
				{
					eResult = WriteFileFB(fileDst, pData->dbBuffer, ulRead, ulWritten, pData->pathDstFile, bSkip);
					if(eResult != TSubTaskBase::eSubResult_Continue)
						return eResult;
					else if(bSkip)
					{
						rLocalStats.IncreaseProcessedSize(pData->spSrcFile->GetLength64() - rBasicProgressInfo.GetCurrentFileProcessedSize());
						pData->bProcessed = false;
						return TSubTaskBase::eSubResult_Continue;
					}

					// increase count of processed data
					rBasicProgressInfo.IncreaseCurrentFileProcessedSize(ulRead);
					rLocalStats.IncreaseProcessedSize(ulRead);
				}
			}
		}
		while(ulRead != 0 && !bLastPart);
	}
	else
	{
		// we don't copy contents, but need to increase processed size
		rLocalStats.IncreaseProcessedSize(pData->spSrcFile->GetLength64() - rBasicProgressInfo.GetCurrentFileProcessedSize());
	}

	pData->bProcessed = true;
	rBasicProgressInfo.SetCurrentFileProcessedSize(0);

	return TSubTaskBase::eSubResult_Continue;
}


TSubTaskBase::ESubOperationResult TSubTaskCopyMove::OpenSourceFileFB(TLocalFilesystemFile& fileSrc, const chcore::TSmartPath& spPathToOpen, bool bNoBuffering)
{
	chcore::IFeedbackHandler* piFeedbackHandler = GetContext().GetFeedbackHandler();
	icpf::log_file& rLog = GetContext().GetLog();

	BOOST_ASSERT(!spPathToOpen.IsEmpty());
	if(spPathToOpen.IsEmpty())
		THROW(_T("Invalid argument"), 0, 0, 0);

	bool bRetry = false;

	fileSrc.Close();

	do
	{
		bRetry = false;

		if(!fileSrc.OpenExistingForReading(spPathToOpen, bNoBuffering))
		{
			DWORD dwLastError = GetLastError();

			FEEDBACK_FILEERROR feedStruct = { spPathToOpen.ToString(), NULL, eCreateError, dwLastError };
			CFeedbackHandler::EFeedbackResult frResult = (CFeedbackHandler::EFeedbackResult)piFeedbackHandler->RequestFeedback(CFeedbackHandler::eFT_FileError, &feedStruct);

			switch(frResult)
			{
			case CFeedbackHandler::eResult_Skip:
				break;	// will return INVALID_HANDLE_VALUE

			case CFeedbackHandler::eResult_Cancel:
				{
					// log
					ictranslate::CFormat fmt;
					fmt.SetFormat(_T("Cancel request [error %errno] while opening source file %path (OpenSourceFileFB)"));
					fmt.SetParam(_t("%errno"), dwLastError);
					fmt.SetParam(_t("%path"), spPathToOpen.ToString());
					rLog.loge(fmt);

					return TSubTaskBase::eSubResult_CancelRequest;
				}

			case CFeedbackHandler::eResult_Pause:
				return TSubTaskBase::eSubResult_PauseRequest;

			case CFeedbackHandler::eResult_Retry:
				{
					// log
					ictranslate::CFormat fmt;
					fmt.SetFormat(_T("Retrying [error %errno] to open source file %path (OpenSourceFileFB)"));
					fmt.SetParam(_t("%errno"), dwLastError);
					fmt.SetParam(_t("%path"), spPathToOpen.ToString());
					rLog.loge(fmt);

					bRetry = true;
					break;
				}

			default:
				BOOST_ASSERT(FALSE);		// unknown result
				THROW(_T("Unhandled case"), 0, 0, 0);
			}
		}
	}
	while(bRetry);

	return TSubTaskBase::eSubResult_Continue;
}

TSubTaskBase::ESubOperationResult TSubTaskCopyMove::OpenDestinationFileFB(TLocalFilesystemFile& fileDst, const chcore::TSmartPath& pathDstFile, bool bNoBuffering, const CFileInfoPtr& spSrcFileInfo, unsigned long long& ullSeekTo, bool& bFreshlyCreated)
{
	chcore::IFeedbackHandler* piFeedbackHandler = GetContext().GetFeedbackHandler();
	icpf::log_file& rLog = GetContext().GetLog();

	bool bRetry = false;

	ullSeekTo = 0;
	bFreshlyCreated = true;

	fileDst.Close();
	do
	{
		bRetry = false;

		if(!fileDst.CreateNewForWriting(pathDstFile, bNoBuffering))
		{
			DWORD dwLastError = GetLastError();
			if(dwLastError == ERROR_FILE_EXISTS)
			{
				bFreshlyCreated = false;

				// pass it to the specialized method
				TSubTaskBase::ESubOperationResult eResult = OpenExistingDestinationFileFB(fileDst, pathDstFile, bNoBuffering);
				if(eResult != TSubTaskBase::eSubResult_Continue)
					return eResult;
				else if(!fileDst.IsOpen())
					return TSubTaskBase::eSubResult_Continue;

				// read info about the existing destination file,
				// NOTE: it is not known which one would be faster - reading file parameters
				//       by using spDstFileInfo->Create() (which uses FindFirstFile()) or by
				//       reading parameters using opened handle; need to be tested in the future
				CFileInfoPtr spDstFileInfo(boost::make_shared<CFileInfo>());

				if(!TLocalFilesystem::GetFileInfo(pathDstFile, spDstFileInfo))
					THROW(_T("Cannot get information about file which has already been opened!"), 0, GetLastError(), 0);

				// src and dst files are the same
				FEEDBACK_ALREADYEXISTS feedStruct = { spSrcFileInfo, spDstFileInfo };
				CFeedbackHandler::EFeedbackResult frResult = (CFeedbackHandler::EFeedbackResult)piFeedbackHandler->RequestFeedback(CFeedbackHandler::eFT_FileAlreadyExists, &feedStruct);
				// check for dialog result
				switch(frResult)
				{
				case CFeedbackHandler::eResult_Overwrite:
					ullSeekTo = 0;
					break;

				case CFeedbackHandler::eResult_CopyRest:
					ullSeekTo = spDstFileInfo->GetLength64();
					break;

				case CFeedbackHandler::eResult_Skip:
					return TSubTaskBase::eSubResult_Continue;

				case CFeedbackHandler::eResult_Cancel:
					{
						// log
						ictranslate::CFormat fmt;
						fmt.SetFormat(_T("Cancel request while checking result of dialog before opening source file %path (CustomCopyFileFB)"));
						fmt.SetParam(_t("%path"), pathDstFile.ToString());
						rLog.logi(fmt);

						return TSubTaskBase::eSubResult_CancelRequest;
					}
				case CFeedbackHandler::eResult_Pause:
					return TSubTaskBase::eSubResult_PauseRequest;

				default:
					BOOST_ASSERT(FALSE);		// unknown result
					THROW(_T("Unhandled case"), 0, 0, 0);
				}
			}
			else
			{
				FEEDBACK_FILEERROR feedStruct = { pathDstFile.ToString(), NULL, eCreateError, dwLastError };
				CFeedbackHandler::EFeedbackResult frResult = (CFeedbackHandler::EFeedbackResult)piFeedbackHandler->RequestFeedback(CFeedbackHandler::eFT_FileError, &feedStruct);
				switch (frResult)
				{
				case CFeedbackHandler::eResult_Retry:
					{
						// log
						ictranslate::CFormat fmt;
						fmt.SetFormat(_T("Retrying [error %errno] to open destination file %path (CustomCopyFileFB)"));
						fmt.SetParam(_t("%errno"), dwLastError);
						fmt.SetParam(_t("%path"), pathDstFile.ToString());
						rLog.loge(fmt);

						bRetry = true;

						break;
					}
				case CFeedbackHandler::eResult_Cancel:
					{
						// log
						ictranslate::CFormat fmt;

						fmt.SetFormat(_T("Cancel request [error %errno] while opening destination file %path (CustomCopyFileFB)"));
						fmt.SetParam(_t("%errno"), dwLastError);
						fmt.SetParam(_t("%path"), pathDstFile.ToString());
						rLog.loge(fmt);

						return TSubTaskBase::eSubResult_CancelRequest;
					}

				case CFeedbackHandler::eResult_Skip:
					break;		// will return invalid handle value

				case CFeedbackHandler::eResult_Pause:
					return TSubTaskBase::eSubResult_PauseRequest;

				default:
					BOOST_ASSERT(FALSE);		// unknown result
					THROW(_T("Unhandled case"), 0, 0, 0);
				}
			}
		}
	}
	while(bRetry);

	return TSubTaskBase::eSubResult_Continue;
}

TSubTaskBase::ESubOperationResult TSubTaskCopyMove::OpenExistingDestinationFileFB(TLocalFilesystemFile& fileDst, const chcore::TSmartPath& pathDstFile, bool bNoBuffering)
{
	chcore::IFeedbackHandler* piFeedbackHandler = GetContext().GetFeedbackHandler();
	icpf::log_file& rLog = GetContext().GetLog();

	bool bRetry = false;

	fileDst.Close();

	do
	{
		bRetry = false;

		if(!fileDst.OpenExistingForWriting(pathDstFile, bNoBuffering))
		{
			DWORD dwLastError = GetLastError();
			FEEDBACK_FILEERROR feedStruct = { pathDstFile.ToString(), NULL, eCreateError, dwLastError };
			CFeedbackHandler::EFeedbackResult frResult = (CFeedbackHandler::EFeedbackResult)piFeedbackHandler->RequestFeedback(CFeedbackHandler::eFT_FileError, &feedStruct);
			switch (frResult)
			{
			case CFeedbackHandler::eResult_Retry:
				{
					// log
					ictranslate::CFormat fmt;
					fmt.SetFormat(_T("Retrying [error %errno] to open destination file %path (CustomCopyFileFB)"));
					fmt.SetParam(_t("%errno"), dwLastError);
					fmt.SetParam(_t("%path"), pathDstFile.ToString());
					rLog.loge(fmt);

					bRetry = true;

					break;
				}
			case CFeedbackHandler::eResult_Cancel:
				{
					// log
					ictranslate::CFormat fmt;

					fmt.SetFormat(_T("Cancel request [error %errno] while opening destination file %path (CustomCopyFileFB)"));
					fmt.SetParam(_t("%errno"), dwLastError);
					fmt.SetParam(_t("%path"), pathDstFile.ToString());
					rLog.loge(fmt);

					return TSubTaskBase::eSubResult_CancelRequest;
				}

			case CFeedbackHandler::eResult_Skip:
				break;		// will return invalid handle value

			case CFeedbackHandler::eResult_Pause:
				return TSubTaskBase::eSubResult_PauseRequest;

			default:
				BOOST_ASSERT(FALSE);		// unknown result
				THROW(_T("Unhandled case"), 0, 0, 0);
			}
		}
	}
	while(bRetry);

	return TSubTaskBase::eSubResult_Continue;
}

TSubTaskBase::ESubOperationResult TSubTaskCopyMove::SetFilePointerFB(TLocalFilesystemFile& file, long long llDistance, const chcore::TSmartPath& pathFile, bool& bSkip)
{
	chcore::IFeedbackHandler* piFeedbackHandler = GetContext().GetFeedbackHandler();
	icpf::log_file& rLog = GetContext().GetLog();

	bSkip = false;
	bool bRetry = false;
	do
	{
		bRetry = false;

		if(!file.SetFilePointer(llDistance, FILE_BEGIN))
		{
			DWORD dwLastError = GetLastError();

			// log
			ictranslate::CFormat fmt;

			fmt.SetFormat(_T("Error %errno while moving file pointer of %path to %pos"));
			fmt.SetParam(_t("%errno"), dwLastError);
			fmt.SetParam(_t("%path"), pathFile.ToString());
			fmt.SetParam(_t("%pos"), llDistance);
			rLog.loge(fmt);

			FEEDBACK_FILEERROR ferr = { pathFile.ToString(), NULL, eSeekError, dwLastError };
			CFeedbackHandler::EFeedbackResult frResult = (CFeedbackHandler::EFeedbackResult)piFeedbackHandler->RequestFeedback(CFeedbackHandler::eFT_FileError, &ferr);
			switch(frResult)
			{
			case CFeedbackHandler::eResult_Cancel:
				return TSubTaskBase::eSubResult_CancelRequest;

			case CFeedbackHandler::eResult_Retry:
				bRetry = true;
				break;

			case CFeedbackHandler::eResult_Pause:
				return TSubTaskBase::eSubResult_PauseRequest;

			case CFeedbackHandler::eResult_Skip:
				bSkip = true;
				return TSubTaskBase::eSubResult_Continue;

			default:
				BOOST_ASSERT(FALSE);		// unknown result
				THROW(_T("Unhandled case"), 0, 0, 0);
			}
		}
	}
	while(bRetry);

	return TSubTaskBase::eSubResult_Continue;
}

TSubTaskBase::ESubOperationResult TSubTaskCopyMove::SetEndOfFileFB(TLocalFilesystemFile& file, const chcore::TSmartPath& pathFile, bool& bSkip)
{
	chcore::IFeedbackHandler* piFeedbackHandler = GetContext().GetFeedbackHandler();
	icpf::log_file& rLog = GetContext().GetLog();

	bSkip = false;

	bool bRetry = false;
	do
	{
		if(!file.SetEndOfFile())
		{
			// log
			DWORD dwLastError = GetLastError();

			ictranslate::CFormat fmt;
			fmt.SetFormat(_T("Error %errno while setting size of file %path to 0"));
			fmt.SetParam(_t("%errno"), dwLastError);
			fmt.SetParam(_t("%path"), pathFile.ToString());
			rLog.loge(fmt);

			FEEDBACK_FILEERROR ferr = { pathFile.ToString(), NULL, eResizeError, dwLastError };
			CFeedbackHandler::EFeedbackResult frResult = (CFeedbackHandler::EFeedbackResult)piFeedbackHandler->RequestFeedback(CFeedbackHandler::eFT_FileError, &ferr);
			switch(frResult)
			{
			case CFeedbackHandler::eResult_Cancel:
				return TSubTaskBase::eSubResult_CancelRequest;

			case CFeedbackHandler::eResult_Retry:
				bRetry = true;

			case CFeedbackHandler::eResult_Pause:
				return TSubTaskBase::eSubResult_PauseRequest;

			case CFeedbackHandler::eResult_Skip:
				bSkip = true;
				return TSubTaskBase::eSubResult_Continue;

			default:
				BOOST_ASSERT(FALSE);		// unknown result
				THROW(_T("Unhandled case"), 0, 0, 0);
			}
		}
	}
	while(bRetry);

	return TSubTaskBase::eSubResult_Continue;
}

TSubTaskBase::ESubOperationResult TSubTaskCopyMove::ReadFileFB(TLocalFilesystemFile& file, CDataBuffer& rBuffer, DWORD dwToRead, DWORD& rdwBytesRead, const chcore::TSmartPath& pathFile, bool& bSkip)
{
	chcore::IFeedbackHandler* piFeedbackHandler = GetContext().GetFeedbackHandler();
	icpf::log_file& rLog = GetContext().GetLog();

	bSkip = false;
	bool bRetry = false;
	do
	{
		bRetry = false;

		if(!file.ReadFile(rBuffer, dwToRead, rdwBytesRead))
		{
			// log
			DWORD dwLastError = GetLastError();

			ictranslate::CFormat fmt;
			fmt.SetFormat(_T("Error %errno while trying to read %count bytes from source file %path (CustomCopyFileFB)"));
			fmt.SetParam(_t("%errno"), dwLastError);
			fmt.SetParam(_t("%count"), dwToRead);
			fmt.SetParam(_t("%path"), pathFile.ToString());
			rLog.loge(fmt);

			FEEDBACK_FILEERROR ferr = { pathFile.ToString(), NULL, eReadError, dwLastError };
			CFeedbackHandler::EFeedbackResult frResult = (CFeedbackHandler::EFeedbackResult)piFeedbackHandler->RequestFeedback(CFeedbackHandler::eFT_FileError, &ferr);
			switch(frResult)
			{
			case CFeedbackHandler::eResult_Cancel:
				return TSubTaskBase::eSubResult_CancelRequest;

			case CFeedbackHandler::eResult_Retry:
				bRetry = true;
				break;

			case CFeedbackHandler::eResult_Pause:
				return TSubTaskBase::eSubResult_PauseRequest;

			case CFeedbackHandler::eResult_Skip:
				bSkip = true;
				return TSubTaskBase::eSubResult_Continue;

			default:
				BOOST_ASSERT(FALSE);		// unknown result
				THROW(_T("Unhandled case"), 0, 0, 0);
			}
		}
	}
	while(bRetry);

	return TSubTaskBase::eSubResult_Continue;
}

TSubTaskBase::ESubOperationResult TSubTaskCopyMove::WriteFileFB(TLocalFilesystemFile& file, CDataBuffer& rBuffer, DWORD dwToWrite, DWORD& rdwBytesWritten, const chcore::TSmartPath& pathFile, bool& bSkip)
{
	chcore::IFeedbackHandler* piFeedbackHandler = GetContext().GetFeedbackHandler();
	icpf::log_file& rLog = GetContext().GetLog();

	bSkip = false;

	bool bRetry = false;
	do
	{
		bRetry = false;

		if(!file.WriteFile(rBuffer, dwToWrite, rdwBytesWritten))
		{
			// log
			DWORD dwLastError = GetLastError();

			ictranslate::CFormat fmt;
			fmt.SetFormat(_T("Error %errno while trying to write %count bytes to destination file %path (CustomCopyFileFB)"));
			fmt.SetParam(_t("%errno"), dwLastError);
			fmt.SetParam(_t("%count"), dwToWrite);
			fmt.SetParam(_t("%path"), pathFile.ToString());
			rLog.loge(fmt);

			FEEDBACK_FILEERROR ferr = { pathFile.ToString(), NULL, eWriteError, dwLastError };
			CFeedbackHandler::EFeedbackResult frResult = (CFeedbackHandler::EFeedbackResult)piFeedbackHandler->RequestFeedback(CFeedbackHandler::eFT_FileError, &ferr);
			switch(frResult)
			{
			case CFeedbackHandler::eResult_Cancel:
				return TSubTaskBase::eSubResult_CancelRequest;

			case CFeedbackHandler::eResult_Retry:
				bRetry = true;
				break;

			case CFeedbackHandler::eResult_Pause:
				return TSubTaskBase::eSubResult_PauseRequest;

			case CFeedbackHandler::eResult_Skip:
				bSkip = true;
				return TSubTaskBase::eSubResult_Continue;

			default:
				BOOST_ASSERT(FALSE);		// unknown result
				THROW(_T("Unhandled case"), 0, 0, 0);
			}
		}
	}
	while(bRetry);

	return TSubTaskBase::eSubResult_Continue;
}
