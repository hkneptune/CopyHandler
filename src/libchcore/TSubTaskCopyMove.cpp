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
#include "TTaskDefinition.h"
#include "TLocalFilesystem.h"
#include "DataBuffer.h"
#include "../libicpf/log.h"
#include "TTaskLocalStats.h"
#include "TTaskConfigTracker.h"
#include "TWorkerThreadController.h"
#include "FeedbackHandlerBase.h"
#include <boost/lexical_cast.hpp>
#include "TBasePathData.h"
#include <boost/smart_ptr/make_shared.hpp>
#include "TFileInfo.h"
#include "TFileInfoArray.h"
#include "SerializationHelpers.h"
#include "TBinarySerializer.h"

BEGIN_CHCORE_NAMESPACE

namespace details
{
	///////////////////////////////////////////////////////////////////////////////////////////////////
	// class TCopyMoveProgressInfo

	TCopyMoveProgressInfo::TCopyMoveProgressInfo() :
		m_stCurrentIndex(0),
		m_ullCurrentFileProcessedSize(0)
	{
	}

	TCopyMoveProgressInfo::~TCopyMoveProgressInfo()
	{
	}

	void TCopyMoveProgressInfo::Serialize(TReadBinarySerializer& rSerializer)
	{
		size_t stIndex = 0;
		unsigned long long ullFilePos = 0;

		Serializers::Serialize(rSerializer, stIndex);
		Serializers::Serialize(rSerializer, ullFilePos);

		boost::unique_lock<boost::shared_mutex> lock(m_lock);
		m_stCurrentIndex = stIndex;
		m_ullCurrentFileProcessedSize = ullFilePos;
	}

	void TCopyMoveProgressInfo::Serialize(TWriteBinarySerializer& rSerializer) const
	{
		size_t stIndex = 0;
		unsigned long long ullFilePos = 0;

		{
			boost::shared_lock<boost::shared_mutex> lock(m_lock);
			stIndex = m_stCurrentIndex;
			ullFilePos = m_ullCurrentFileProcessedSize;
		}

		Serializers::Serialize(rSerializer, stIndex);
		Serializers::Serialize(rSerializer, ullFilePos);
	}

	void TCopyMoveProgressInfo::ResetProgress()
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);
		m_stCurrentIndex = 0;
	}

	void TCopyMoveProgressInfo::SetCurrentIndex(size_t stIndex)
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);
		m_stCurrentIndex = stIndex;
	}

	void TCopyMoveProgressInfo::IncreaseCurrentIndex()
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);
		++m_stCurrentIndex;
	}

	size_t TCopyMoveProgressInfo::GetCurrentIndex() const
	{
		boost::shared_lock<boost::shared_mutex> lock(m_lock);
		return m_stCurrentIndex;
	}

	void TCopyMoveProgressInfo::SetCurrentFileProcessedSize(unsigned long long ullSize)
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);
		m_ullCurrentFileProcessedSize = ullSize;
	}

	unsigned long long TCopyMoveProgressInfo::GetCurrentFileProcessedSize() const
	{
		boost::shared_lock<boost::shared_mutex> lock(m_lock);
		return m_ullCurrentFileProcessedSize;
	}

	void TCopyMoveProgressInfo::IncreaseCurrentFileProcessedSize(unsigned long long ullSizeToAdd)
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);
		m_ullCurrentFileProcessedSize += ullSizeToAdd;
	}
}

// assume max sectors of 4kB (for rounding)
#define MAXSECTORSIZE			4096

struct CUSTOM_COPY_PARAMS
{
	TFileInfoPtr spSrcFile;		// CFileInfo - src file
	TSmartPath pathDstFile;			// dest path with filename

	TDataBuffer dbBuffer;		// buffer handling
	bool bOnlyCreate;			// flag from configuration - skips real copying - only create
	bool bProcessed;			// has the element been processed ? (false if skipped)
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// class TSubTaskCopyMove

TSubTaskCopyMove::TSubTaskCopyMove(TSubTaskContext& tSubTaskContext) :
	TSubTaskBase(tSubTaskContext)
{
}

void TSubTaskCopyMove::Reset()
{
	m_tProgressInfo.ResetProgress();
	m_tSubTaskStats.Clear();
}

TSubTaskBase::ESubOperationResult TSubTaskCopyMove::Exec()
{
	TSubTaskProcessingGuard guard(m_tSubTaskStats);

	icpf::log_file& rLog = GetContext().GetLog();
	TFileInfoArray& rFilesCache = GetContext().GetFilesCache();
	TTaskDefinition& rTaskDefinition = GetContext().GetTaskDefinition();
	TTaskConfigTracker& rCfgTracker = GetContext().GetCfgTracker();
	TWorkerThreadController& rThreadController = GetContext().GetThreadController();
	IFeedbackHandler* piFeedbackHandler = GetContext().GetFeedbackHandler();

	BOOST_ASSERT(piFeedbackHandler != NULL);
	if(piFeedbackHandler == NULL)
		return eSubResult_Error;

	// log
	rLog.logi(_T("Processing files/folders (ProcessFiles)"));

	// new stats
	m_tSubTaskStats.SetCurrentBufferIndex(TBufferSizes::eBuffer_Default);
	m_tSubTaskStats.SetTotalCount(rFilesCache.GetSize());
	m_tSubTaskStats.SetProcessedCount(rFilesCache.GetSize());
	m_tSubTaskStats.SetTotalSize(rFilesCache.CalculateTotalSize());
	m_tSubTaskStats.SetProcessedSize(rFilesCache.CalculatePartialSize(m_tProgressInfo.GetCurrentIndex()));
	m_tSubTaskStats.SetCurrentPath(TString());

	// now it's time to check if there is enough space on destination device
	TSubTaskBase::ESubOperationResult eResult = CheckForFreeSpaceFB();
	if(eResult != TSubTaskBase::eSubResult_Continue)
		return eResult;

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

	RecreateBufferIfNeeded(ccp.dbBuffer, true);

	// log
	TString strFormat;
	strFormat = _T("Processing files/folders (ProcessFiles):\r\n\tOnlyCreate: %create\r\n\tFiles/folders count: %filecount\r\n\tIgnore Folders: %ignorefolders\r\n\tDest path: %dstpath\r\n\tCurrent index (0-based): %currindex");
	strFormat.Replace(_T("%create"), boost::lexical_cast<std::wstring>(ccp.bOnlyCreate).c_str());
	strFormat.Replace(_T("%filecount"), boost::lexical_cast<std::wstring>(stSize).c_str());
	strFormat.Replace(_T("%ignorefolders"), boost::lexical_cast<std::wstring>(bIgnoreFolders).c_str());
	strFormat.Replace(_T("%dstpath"), rTaskDefinition.GetDestinationPath().ToString());
	strFormat.Replace(_T("%currindex"), boost::lexical_cast<std::wstring>(m_tProgressInfo.GetCurrentIndex()).c_str());

	rLog.logi(strFormat);

	size_t stIndex = m_tProgressInfo.GetCurrentIndex();
	for(; stIndex < stSize; stIndex++)
	{
		m_tProgressInfo.SetCurrentIndex(stIndex);

		// should we kill ?
		if(rThreadController.KillRequested())
		{
			// log
			rLog.logi(_T("Kill request while processing file in ProcessFiles"));
			return TSubTaskBase::eSubResult_KillRequest;
		}

		// next file to be copied
		TFileInfoPtr spFileInfo = rFilesCache.GetAt(stIndex);
		TSmartPath pathCurrent = spFileInfo->GetFullFilePath();

		// new stats
		m_tSubTaskStats.SetProcessedCount(stIndex);
		m_tSubTaskStats.SetCurrentPath(pathCurrent.ToString());

		// set dest path with filename
		ccp.pathDstFile = CalculateDestinationPath(spFileInfo, rTaskDefinition.GetDestinationPath(), ((int)bForceDirectories) << 1 | (int)bIgnoreFolders);

		// are the files/folders lie on the same partition ?
		bool bMove = rTaskDefinition.GetOperationType() == eOperation_Move;

		// if folder - create it
		if(spFileInfo->IsDirectory())
		{
			TSubTaskBase::ESubOperationResult eResult = CreateDirectoryFB(ccp.pathDstFile);
			if(eResult != TSubTaskBase::eSubResult_Continue)
				return eResult;

			// new stats
			m_tSubTaskStats.IncreaseProcessedSize(spFileInfo->GetLength64());

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
				TLocalFilesystem::DeleteFile(spFileInfo->GetFullFilePath());	// there will be another try later, so we don't check
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

	m_tProgressInfo.SetCurrentIndex(stIndex);

	// new stats
	m_tSubTaskStats.SetProcessedCount(stIndex);
	m_tSubTaskStats.SetCurrentPath(TString());

	// delete buffer - it's not needed
	ccp.dbBuffer.Delete();

	// log
	rLog.logi(_T("Finished processing in ProcessFiles"));

	return TSubTaskBase::eSubResult_Continue;
}

void TSubTaskCopyMove::GetStatsSnapshot(TSubTaskStatsSnapshot& rStats) const
{
	m_tSubTaskStats.GetSnapshot(rStats);
}

int TSubTaskCopyMove::GetBufferIndex(const TFileInfoPtr& spFileInfo)
{
	if(!spFileInfo)
		THROW_CORE_EXCEPTION(eErr_InvalidArgument);

	TSmartPath pathSource = spFileInfo->GetFullFilePath();
	TSmartPath pathDestination = GetContext().GetTaskDefinition().GetDestinationPath();

	TLocalFilesystem::EPathsRelation eRelation = GetContext().GetLocalFilesystem().GetPathsRelation(pathSource, pathDestination);
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

TSubTaskBase::ESubOperationResult TSubTaskCopyMove::CustomCopyFileFB(CUSTOM_COPY_PARAMS* pData)
{
	TTaskDefinition& rTaskDefinition = GetContext().GetTaskDefinition();
	TWorkerThreadController& rThreadController = GetContext().GetThreadController();
	icpf::log_file& rLog = GetContext().GetLog();

	TLocalFilesystemFile fileSrc = TLocalFilesystem::CreateFileObject();
	TLocalFilesystemFile fileDst = TLocalFilesystem::CreateFileObject();

	TString strFormat;
	TSubTaskBase::ESubOperationResult eResult = TSubTaskBase::eSubResult_Continue;

	// calculate if we want to disable buffering for file transfer
	// NOTE: we are using here the file size read when scanning directories for files; it might be
	//       outdated at this point, but at present we don't want to re-read file size since it
	//       will cost additional disk access
	bool bNoBuffer = (GetTaskPropValue<eTO_DisableBuffering>(rTaskDefinition.GetConfiguration()) &&
		pData->spSrcFile->GetLength64() >= GetTaskPropValue<eTO_DisableBufferingMinSize>(rTaskDefinition.GetConfiguration()));

	bool bSkip = false;
	eResult = OpenSrcAndDstFilesFB(pData, fileSrc, fileDst, bNoBuffer, bSkip);
	if(eResult != TSubTaskBase::eSubResult_Continue)
		return eResult;
	else if(bSkip)
		return TSubTaskBase::eSubResult_Continue;

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
			strFormat = _T("Kill request while main copying file %srcpath -> %dstpath");
			strFormat.Replace(_T("%srcpath"), pData->spSrcFile->GetFullFilePath().ToString());
			strFormat.Replace(_T("%dstpath"), pData->pathDstFile.ToString());
			rLog.logi(strFormat);
			return TSubTaskBase::eSubResult_KillRequest;
		}

		// recreate buffer if needed
		RecreateBufferIfNeeded(pData->dbBuffer, false);

		// establish count of data to read
		if(GetTaskPropValue<eTO_UseOnlyDefaultBuffer>(rTaskDefinition.GetConfiguration()))
			iBufferIndex = TBufferSizes::eBuffer_Default;
		else
			iBufferIndex = GetBufferIndex(pData->spSrcFile);
		// new stats
		m_tSubTaskStats.SetCurrentBufferIndex(iBufferIndex);

		ulToRead = bNoBuffer ? ROUNDUP(pData->dbBuffer.GetSizes().GetSizeByType((TBufferSizes::EBufferType)iBufferIndex), MAXSECTORSIZE) : pData->dbBuffer.GetSizes().GetSizeByType((TBufferSizes::EBufferType)iBufferIndex);

		// read data from file to buffer
		eResult = ReadFileFB(fileSrc, pData->dbBuffer, ulToRead, ulRead, pData->spSrcFile->GetFullFilePath(), bSkip);
		if(eResult != TSubTaskBase::eSubResult_Continue)
			return eResult;
		else if(bSkip)
		{
			// new stats
			m_tSubTaskStats.IncreaseProcessedSize(pData->spSrcFile->GetLength64() - m_tProgressInfo.GetCurrentFileProcessedSize());

			pData->bProcessed = false;
			return TSubTaskBase::eSubResult_Continue;
		}

		bLastPart = (ulToRead != ulRead);

		if(ulRead > 0)
		{
			eResult = WriteFileExFB(fileDst, pData->dbBuffer, ulRead, ulWritten, pData->pathDstFile, bSkip, bNoBuffer);
			if(eResult != TSubTaskBase::eSubResult_Continue)
				return eResult;
			else if(bSkip)
			{
				// new stats
				m_tSubTaskStats.IncreaseProcessedSize(pData->spSrcFile->GetLength64() - m_tProgressInfo.GetCurrentFileProcessedSize());

				pData->bProcessed = false;
				return TSubTaskBase::eSubResult_Continue;
			}

			// increase count of processed data
			m_tProgressInfo.IncreaseCurrentFileProcessedSize(ulWritten);
			// new stats
			m_tSubTaskStats.IncreaseProcessedSize(ulWritten);
		}
	}
	while(ulRead != 0 && !bLastPart);

	pData->bProcessed = true;
	m_tProgressInfo.SetCurrentFileProcessedSize(0);

	return TSubTaskBase::eSubResult_Continue;
}

TSubTaskCopyMove::ESubOperationResult TSubTaskCopyMove::OpenSrcAndDstFilesFB(CUSTOM_COPY_PARAMS* pData, TLocalFilesystemFile &fileSrc, TLocalFilesystemFile &fileDst, bool bNoBuffer, bool& bSkip)
{
	TTaskDefinition& rTaskDefinition = GetContext().GetTaskDefinition();

	bSkip = false;

	// first open the source file and handle any failures
	TSubTaskCopyMove::ESubOperationResult eResult = OpenSourceFileFB(fileSrc, pData->spSrcFile->GetFullFilePath(), bNoBuffer);
	if(eResult != TSubTaskBase::eSubResult_Continue)
		return eResult;
	else if(!fileSrc.IsOpen())
	{
		// invalid handle = operation skipped by user
		// new stats
		m_tSubTaskStats.IncreaseProcessedSize(pData->spSrcFile->GetLength64() - m_tProgressInfo.GetCurrentFileProcessedSize());

		pData->bProcessed = false;
		bSkip = true;
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

	if(m_tProgressInfo.GetCurrentFileProcessedSize() == 0)
	{
		// open destination file for case, when we start operation on this file (i.e. it is not resume of the
		// old operation)
		eResult = OpenDestinationFileFB(fileDst, pData->pathDstFile, bNoBuffer, pData->spSrcFile, ullSeekTo, bDstFileFreshlyCreated);
		if(eResult != TSubTaskBase::eSubResult_Continue)
			return eResult;
		else if(!fileDst.IsOpen())
		{
			// new stats
			m_tSubTaskStats.IncreaseProcessedSize(pData->spSrcFile->GetLength64() - m_tProgressInfo.GetCurrentFileProcessedSize());

			pData->bProcessed = false;
			bSkip = true;
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
			// new stats
			m_tSubTaskStats.IncreaseProcessedSize(pData->spSrcFile->GetLength64() - m_tProgressInfo.GetCurrentFileProcessedSize());

			pData->bProcessed = false;
			bSkip = true;
			return TSubTaskBase::eSubResult_Continue;
		}

		ullSeekTo = m_tProgressInfo.GetCurrentFileProcessedSize();
	}

	if(pData->bOnlyCreate)
	{
		// we don't copy contents, but need to increase processed size
		// new stats
		m_tSubTaskStats.IncreaseProcessedSize(pData->spSrcFile->GetLength64() - m_tProgressInfo.GetCurrentFileProcessedSize());
		return TSubTaskBase::eSubResult_Continue;
	}

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
			// new stats
			m_tSubTaskStats.IncreaseProcessedSize(pData->spSrcFile->GetLength64() - m_tProgressInfo.GetCurrentFileProcessedSize());

			pData->bProcessed = false;
			return TSubTaskBase::eSubResult_Continue;
		}

		eResult = SetFilePointerFB(fileDst, ullMove, pData->pathDstFile, bSkip);
		if(eResult != TSubTaskBase::eSubResult_Continue)
			return eResult;
		else if(bSkip)
		{
			// with either first or second seek we got 'skip' answer...
			// new stats
			m_tSubTaskStats.IncreaseProcessedSize(pData->spSrcFile->GetLength64() - m_tProgressInfo.GetCurrentFileProcessedSize());

			pData->bProcessed = false;
			return TSubTaskBase::eSubResult_Continue;
		}

		m_tProgressInfo.IncreaseCurrentFileProcessedSize(ullMove);
		// new stats
		m_tSubTaskStats.IncreaseProcessedSize(ullMove);
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

	return eResult;
}

void TSubTaskCopyMove::RecreateBufferIfNeeded(TDataBuffer& rBuffer, bool bInitialCreate)
{
	TTaskConfigTracker& rCfgTracker = GetContext().GetCfgTracker();
	TTaskDefinition& rTaskDefinition = GetContext().GetTaskDefinition();
	icpf::log_file& rLog = GetContext().GetLog();

	if(bInitialCreate || (rCfgTracker.IsModified() && rCfgTracker.IsModified(TOptionsSet() % eTO_DefaultBufferSize % eTO_OneDiskBufferSize % eTO_TwoDisksBufferSize % eTO_CDBufferSize % eTO_LANBufferSize % eTO_UseOnlyDefaultBuffer, true)))
	{
		TBufferSizes bs;
		bs.SetOnlyDefault(GetTaskPropValue<eTO_UseOnlyDefaultBuffer>(rTaskDefinition.GetConfiguration()));
		bs.SetDefaultSize(GetTaskPropValue<eTO_DefaultBufferSize>(rTaskDefinition.GetConfiguration()));
		bs.SetOneDiskSize(GetTaskPropValue<eTO_OneDiskBufferSize>(rTaskDefinition.GetConfiguration()));
		bs.SetTwoDisksSize(GetTaskPropValue<eTO_TwoDisksBufferSize>(rTaskDefinition.GetConfiguration()));
		bs.SetCDSize(GetTaskPropValue<eTO_CDBufferSize>(rTaskDefinition.GetConfiguration()));
		bs.SetLANSize(GetTaskPropValue<eTO_LANBufferSize>(rTaskDefinition.GetConfiguration()));

		// log
		const TBufferSizes& rbs1 = rBuffer.GetSizes();

		TString strFormat;
		strFormat = _T("Changing buffer size from [Def:%defsize, One:%onesize, Two:%twosize, CD:%cdsize, LAN:%lansize] to [Def:%defsize2, One:%onesize2, Two:%twosize2, CD:%cdsize2, LAN:%lansize2]");

		strFormat.Replace(_T("%defsize"), boost::lexical_cast<std::wstring>(rbs1.GetDefaultSize()).c_str());
		strFormat.Replace(_T("%onesize"), boost::lexical_cast<std::wstring>(rbs1.GetOneDiskSize()).c_str());
		strFormat.Replace(_T("%twosize"), boost::lexical_cast<std::wstring>(rbs1.GetTwoDisksSize()).c_str());
		strFormat.Replace(_T("%cdsize"), boost::lexical_cast<std::wstring>(rbs1.GetCDSize()).c_str());
		strFormat.Replace(_T("%lansize"), boost::lexical_cast<std::wstring>(rbs1.GetLANSize()).c_str());
		strFormat.Replace(_T("%defsize2"), boost::lexical_cast<std::wstring>(bs.GetDefaultSize()).c_str());
		strFormat.Replace(_T("%onesize2"), boost::lexical_cast<std::wstring>(bs.GetOneDiskSize()).c_str());
		strFormat.Replace(_T("%twosize2"), boost::lexical_cast<std::wstring>(bs.GetTwoDisksSize()).c_str());
		strFormat.Replace(_T("%cdsize2"), boost::lexical_cast<std::wstring>(bs.GetCDSize()).c_str());
		strFormat.Replace(_T("%lansize2"), boost::lexical_cast<std::wstring>(bs.GetLANSize()).c_str());

		rLog.logi(strFormat);
		rBuffer.Create(bs);
	}
}

TSubTaskBase::ESubOperationResult TSubTaskCopyMove::OpenSourceFileFB(TLocalFilesystemFile& fileSrc, const TSmartPath& spPathToOpen, bool bNoBuffering)
{
	IFeedbackHandler* piFeedbackHandler = GetContext().GetFeedbackHandler();
	icpf::log_file& rLog = GetContext().GetLog();

	BOOST_ASSERT(!spPathToOpen.IsEmpty());
	if(spPathToOpen.IsEmpty())
		THROW_CORE_EXCEPTION(eErr_InvalidArgument);

	bool bRetry = false;

	fileSrc.Close();

	do
	{
		bRetry = false;

		if(!fileSrc.OpenExistingForReading(spPathToOpen, bNoBuffering))
		{
			DWORD dwLastError = GetLastError();

			FEEDBACK_FILEERROR feedStruct = { spPathToOpen.ToString(), NULL, eCreateError, dwLastError };
			IFeedbackHandler::EFeedbackResult frResult = (IFeedbackHandler::EFeedbackResult)piFeedbackHandler->RequestFeedback(IFeedbackHandler::eFT_FileError, &feedStruct);

			switch(frResult)
			{
			case IFeedbackHandler::eResult_Skip:
				break;	// will return INVALID_HANDLE_VALUE

			case IFeedbackHandler::eResult_Cancel:
				{
					// log
					TString strFormat = _T("Cancel request [error %errno] while opening source file %path (OpenSourceFileFB)");
					strFormat.Replace(_T("%errno"), boost::lexical_cast<std::wstring>(dwLastError).c_str());
					strFormat.Replace(_T("%path"), spPathToOpen.ToString());
					rLog.loge(strFormat);

					return TSubTaskBase::eSubResult_CancelRequest;
				}

			case IFeedbackHandler::eResult_Pause:
				return TSubTaskBase::eSubResult_PauseRequest;

			case IFeedbackHandler::eResult_Retry:
				{
					// log
					TString strFormat = _T("Retrying [error %errno] to open source file %path (OpenSourceFileFB)");
					strFormat.Replace(_T("%errno"), boost::lexical_cast<std::wstring>(dwLastError).c_str());
					strFormat.Replace(_T("%path"), spPathToOpen.ToString());
					rLog.loge(strFormat);

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

TSubTaskBase::ESubOperationResult TSubTaskCopyMove::OpenDestinationFileFB(TLocalFilesystemFile& fileDst, const TSmartPath& pathDstFile, bool bNoBuffering, const TFileInfoPtr& spSrcFileInfo, unsigned long long& ullSeekTo, bool& bFreshlyCreated)
{
	IFeedbackHandler* piFeedbackHandler = GetContext().GetFeedbackHandler();
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
				TFileInfoPtr spDstFileInfo(boost::make_shared<TFileInfo>());

				if(!TLocalFilesystem::GetFileInfo(pathDstFile, spDstFileInfo))
					THROW_CORE_EXCEPTION_WIN32(eErr_CannotGetFileInfo, GetLastError());

				// src and dst files are the same
				FEEDBACK_ALREADYEXISTS feedStruct = { spSrcFileInfo, spDstFileInfo };
				IFeedbackHandler::EFeedbackResult frResult = (IFeedbackHandler::EFeedbackResult)piFeedbackHandler->RequestFeedback(IFeedbackHandler::eFT_FileAlreadyExists, &feedStruct);
				// check for dialog result
				switch(frResult)
				{
				case IFeedbackHandler::eResult_Overwrite:
					ullSeekTo = 0;
					break;

				case IFeedbackHandler::eResult_CopyRest:
					ullSeekTo = spDstFileInfo->GetLength64();
					break;

				case IFeedbackHandler::eResult_Skip:
					return TSubTaskBase::eSubResult_Continue;

				case IFeedbackHandler::eResult_Cancel:
					{
						// log
						TString strFormat = _T("Cancel request while checking result of dialog before opening source file %path (CustomCopyFileFB)");
						strFormat.Replace(_T("%path"), pathDstFile.ToString());
						rLog.logi(strFormat);

						return TSubTaskBase::eSubResult_CancelRequest;
					}
				case IFeedbackHandler::eResult_Pause:
					return TSubTaskBase::eSubResult_PauseRequest;

				default:
					BOOST_ASSERT(FALSE);		// unknown result
					THROW_CORE_EXCEPTION(eErr_UnhandledCase);
				}
			}
			else
			{
				FEEDBACK_FILEERROR feedStruct = { pathDstFile.ToString(), NULL, eCreateError, dwLastError };
				IFeedbackHandler::EFeedbackResult frResult = (IFeedbackHandler::EFeedbackResult)piFeedbackHandler->RequestFeedback(IFeedbackHandler::eFT_FileError, &feedStruct);
				switch(frResult)
				{
				case IFeedbackHandler::eResult_Retry:
					{
						// log
						TString strFormat = _T("Retrying [error %errno] to open destination file %path (CustomCopyFileFB)");
						strFormat.Replace(_T("%errno"), boost::lexical_cast<std::wstring>(dwLastError).c_str());
						strFormat.Replace(_T("%path"), pathDstFile.ToString());
						rLog.loge(strFormat);

						bRetry = true;

						break;
					}
				case IFeedbackHandler::eResult_Cancel:
					{
						// log
						TString strFormat = _T("Cancel request [error %errno] while opening destination file %path (CustomCopyFileFB)");
						strFormat.Replace(_T("%errno"), boost::lexical_cast<std::wstring>(dwLastError).c_str());
						strFormat.Replace(_T("%path"), pathDstFile.ToString());
						rLog.loge(strFormat);

						return TSubTaskBase::eSubResult_CancelRequest;
					}

				case IFeedbackHandler::eResult_Skip:
					break;		// will return invalid handle value

				case IFeedbackHandler::eResult_Pause:
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

TSubTaskBase::ESubOperationResult TSubTaskCopyMove::OpenExistingDestinationFileFB(TLocalFilesystemFile& fileDst, const TSmartPath& pathDstFile, bool bNoBuffering)
{
	IFeedbackHandler* piFeedbackHandler = GetContext().GetFeedbackHandler();
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
			IFeedbackHandler::EFeedbackResult frResult = (IFeedbackHandler::EFeedbackResult)piFeedbackHandler->RequestFeedback(IFeedbackHandler::eFT_FileError, &feedStruct);
			switch (frResult)
			{
			case IFeedbackHandler::eResult_Retry:
				{
					// log
					TString strFormat = _T("Retrying [error %errno] to open destination file %path (CustomCopyFileFB)");
					strFormat.Replace(_T("%errno"), boost::lexical_cast<std::wstring>(dwLastError).c_str());
					strFormat.Replace(_t("%path"), pathDstFile.ToString());
					rLog.loge(strFormat);

					bRetry = true;

					break;
				}
			case IFeedbackHandler::eResult_Cancel:
				{
					// log
					TString strFormat = _T("Cancel request [error %errno] while opening destination file %path (CustomCopyFileFB)");
					strFormat.Replace(_T("%errno"), boost::lexical_cast<std::wstring>(dwLastError).c_str());
					strFormat.Replace(_T("%path"), pathDstFile.ToString());
					rLog.loge(strFormat);

					return TSubTaskBase::eSubResult_CancelRequest;
				}

			case IFeedbackHandler::eResult_Skip:
				break;		// will return invalid handle value

			case IFeedbackHandler::eResult_Pause:
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

TSubTaskBase::ESubOperationResult TSubTaskCopyMove::SetFilePointerFB(TLocalFilesystemFile& file, long long llDistance, const TSmartPath& pathFile, bool& bSkip)
{
	IFeedbackHandler* piFeedbackHandler = GetContext().GetFeedbackHandler();
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
			TString strFormat = _T("Error %errno while moving file pointer of %path to %pos");
			strFormat.Replace(_t("%errno"), boost::lexical_cast<std::wstring>(dwLastError).c_str());
			strFormat.Replace(_t("%path"), pathFile.ToString());
			strFormat.Replace(_t("%pos"), boost::lexical_cast<std::wstring>(llDistance).c_str());
			rLog.loge(strFormat);

			FEEDBACK_FILEERROR ferr = { pathFile.ToString(), NULL, eSeekError, dwLastError };
			IFeedbackHandler::EFeedbackResult frResult = (IFeedbackHandler::EFeedbackResult)piFeedbackHandler->RequestFeedback(IFeedbackHandler::eFT_FileError, &ferr);
			switch(frResult)
			{
			case IFeedbackHandler::eResult_Cancel:
				return TSubTaskBase::eSubResult_CancelRequest;

			case IFeedbackHandler::eResult_Retry:
				bRetry = true;
				break;

			case IFeedbackHandler::eResult_Pause:
				return TSubTaskBase::eSubResult_PauseRequest;

			case IFeedbackHandler::eResult_Skip:
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

TSubTaskBase::ESubOperationResult TSubTaskCopyMove::SetEndOfFileFB(TLocalFilesystemFile& file, const TSmartPath& pathFile, bool& bSkip)
{
	IFeedbackHandler* piFeedbackHandler = GetContext().GetFeedbackHandler();
	icpf::log_file& rLog = GetContext().GetLog();

	bSkip = false;

	bool bRetry = false;
	do
	{
		if(!file.SetEndOfFile())
		{
			// log
			DWORD dwLastError = GetLastError();

			TString strFormat = _T("Error %errno while setting size of file %path to 0");
			strFormat.Replace(_t("%errno"), boost::lexical_cast<std::wstring>(dwLastError).c_str());
			strFormat.Replace(_t("%path"), pathFile.ToString());
			rLog.loge(strFormat);

			FEEDBACK_FILEERROR ferr = { pathFile.ToString(), NULL, eResizeError, dwLastError };
			IFeedbackHandler::EFeedbackResult frResult = (IFeedbackHandler::EFeedbackResult)piFeedbackHandler->RequestFeedback(IFeedbackHandler::eFT_FileError, &ferr);
			switch(frResult)
			{
			case IFeedbackHandler::eResult_Cancel:
				return TSubTaskBase::eSubResult_CancelRequest;

			case IFeedbackHandler::eResult_Retry:
				bRetry = true;

			case IFeedbackHandler::eResult_Pause:
				return TSubTaskBase::eSubResult_PauseRequest;

			case IFeedbackHandler::eResult_Skip:
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

TSubTaskBase::ESubOperationResult TSubTaskCopyMove::ReadFileFB(TLocalFilesystemFile& file, TDataBuffer& rBuffer, DWORD dwToRead, DWORD& rdwBytesRead, const TSmartPath& pathFile, bool& bSkip)
{
	IFeedbackHandler* piFeedbackHandler = GetContext().GetFeedbackHandler();
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

			TString strFormat = _T("Error %errno while trying to read %count bytes from source file %path (CustomCopyFileFB)");
			strFormat.Replace(_t("%errno"), boost::lexical_cast<std::wstring>(dwLastError).c_str());
			strFormat.Replace(_t("%count"), boost::lexical_cast<std::wstring>(dwToRead).c_str());
			strFormat.Replace(_t("%path"), pathFile.ToString());
			rLog.loge(strFormat);

			FEEDBACK_FILEERROR ferr = { pathFile.ToString(), NULL, eReadError, dwLastError };
			IFeedbackHandler::EFeedbackResult frResult = (IFeedbackHandler::EFeedbackResult)piFeedbackHandler->RequestFeedback(IFeedbackHandler::eFT_FileError, &ferr);
			switch(frResult)
			{
			case IFeedbackHandler::eResult_Cancel:
				return TSubTaskBase::eSubResult_CancelRequest;

			case IFeedbackHandler::eResult_Retry:
				bRetry = true;
				break;

			case IFeedbackHandler::eResult_Pause:
				return TSubTaskBase::eSubResult_PauseRequest;

			case IFeedbackHandler::eResult_Skip:
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

TSubTaskBase::ESubOperationResult TSubTaskCopyMove::WriteFileFB(TLocalFilesystemFile& file, TDataBuffer& rBuffer, DWORD dwToWrite, DWORD& rdwBytesWritten, const TSmartPath& pathFile, bool& bSkip)
{
	IFeedbackHandler* piFeedbackHandler = GetContext().GetFeedbackHandler();
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

			TString strFormat = _T("Error %errno while trying to write %count bytes to destination file %path (CustomCopyFileFB)");
			strFormat.Replace(_t("%errno"), boost::lexical_cast<std::wstring>(dwLastError).c_str());
			strFormat.Replace(_t("%count"), boost::lexical_cast<std::wstring>(dwToWrite).c_str());
			strFormat.Replace(_t("%path"), pathFile.ToString());
			rLog.loge(strFormat);

			FEEDBACK_FILEERROR ferr = { pathFile.ToString(), NULL, eWriteError, dwLastError };
			IFeedbackHandler::EFeedbackResult frResult = (IFeedbackHandler::EFeedbackResult)piFeedbackHandler->RequestFeedback(IFeedbackHandler::eFT_FileError, &ferr);
			switch(frResult)
			{
			case IFeedbackHandler::eResult_Cancel:
				return TSubTaskBase::eSubResult_CancelRequest;

			case IFeedbackHandler::eResult_Retry:
				bRetry = true;
				break;

			case IFeedbackHandler::eResult_Pause:
				return TSubTaskBase::eSubResult_PauseRequest;

			case IFeedbackHandler::eResult_Skip:
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

TSubTaskBase::ESubOperationResult TSubTaskCopyMove::WriteFileExFB(TLocalFilesystemFile& file, TDataBuffer& rBuffer, DWORD dwToWrite, DWORD& rdwBytesWritten, const TSmartPath& pathFile, bool& bSkip, bool bNoBuffer)
{
	TString strFormat;
	TSubTaskBase::ESubOperationResult eResult = TSubTaskBase::eSubResult_Continue;

	rdwBytesWritten = 0;

	// copying
	unsigned long ulWritten = 0;
	bool bNonAlignedSize = (dwToWrite % MAXSECTORSIZE) != 0;

	// handle not aligned part at the end of file when no buffering is enabled
	if(bNoBuffer && bNonAlignedSize)
	{
		// count of data read from the file is less than requested - we're at the end of source file
		// and this is the operation with system buffering turned off

		// write as much as possible to the destination file with no buffering
		// NOTE: as an alternative, we could write more data to the destination file and then truncate the file
		unsigned long ulDataToWrite = ROUNDDOWN(dwToWrite, MAXSECTORSIZE);
		if(ulDataToWrite > 0)
		{
			eResult = WriteFileFB(file, rBuffer, ulDataToWrite, ulWritten, pathFile, bSkip);
			if(eResult != TSubTaskBase::eSubResult_Continue || bSkip)
				return eResult;

			// calculate count of bytes left to be written
			rdwBytesWritten = ulWritten;
			dwToWrite -= ulWritten;

			// now remove part of data from buffer (ulWritten bytes)
			rBuffer.CutDataFromBuffer(ulWritten);
		}

		// close and re-open the destination file with buffering option for append
		file.Close();

		// are there any more data to be written?
		if(dwToWrite != 0)
		{
			// re-open the destination file, this time with standard buffering to allow writing not aligned part of file data
			eResult = OpenExistingDestinationFileFB(file, pathFile, false);
			if(eResult != TSubTaskBase::eSubResult_Continue || !file.IsOpen())
				return eResult;

			// move file pointer to the end of destination file
			eResult = SetFilePointerFB(file, m_tProgressInfo.GetCurrentFileProcessedSize() + rdwBytesWritten, pathFile, bSkip);
			if(eResult != TSubTaskBase::eSubResult_Continue || bSkip)
				return eResult;
		}
	}

	// write
	if(dwToWrite != 0)
	{
		eResult = WriteFileFB(file, rBuffer, dwToWrite, ulWritten, pathFile, bSkip);
		if(eResult != TSubTaskBase::eSubResult_Continue || bSkip)
			return eResult;

		rdwBytesWritten += ulWritten;
	}

	return TSubTaskBase::eSubResult_Continue;
}

TSubTaskBase::ESubOperationResult TSubTaskCopyMove::CreateDirectoryFB(const TSmartPath& pathDirectory)
{
	icpf::log_file& rLog = GetContext().GetLog();
	IFeedbackHandler* piFeedbackHandler = GetContext().GetFeedbackHandler();

	bool bRetry = true;
	DWORD dwLastError = ERROR_SUCCESS;
	while(bRetry && !TLocalFilesystem::CreateDirectory(pathDirectory, false) && (dwLastError = GetLastError()) != ERROR_ALREADY_EXISTS)
	{
		// log
		TString strFormat;
		strFormat = _T("Error %errno while calling CreateDirectory %path (ProcessFiles)");
		strFormat.Replace(_T("%errno"), boost::lexical_cast<std::wstring>(dwLastError).c_str());
		strFormat.Replace(_T("%path"), pathDirectory.ToString());
		rLog.loge(strFormat);

		FEEDBACK_FILEERROR ferr = { pathDirectory.ToString(), NULL, eCreateError, dwLastError };
		IFeedbackHandler::EFeedbackResult frResult = (IFeedbackHandler::EFeedbackResult)piFeedbackHandler->RequestFeedback(IFeedbackHandler::eFT_FileError, &ferr);
		switch(frResult)
		{
		case IFeedbackHandler::eResult_Cancel:
			return TSubTaskBase::eSubResult_CancelRequest;

		case IFeedbackHandler::eResult_Retry:
			bRetry = false;
			break;

		case IFeedbackHandler::eResult_Pause:
			return TSubTaskBase::eSubResult_PauseRequest;

		case IFeedbackHandler::eResult_Skip:
			bRetry = false;
			break;		// just do nothing

		default:
			BOOST_ASSERT(FALSE);		// unknown result
			THROW_CORE_EXCEPTION(eErr_UnhandledCase);
		}
	}

	return TSubTaskBase::eSubResult_Continue;
}

TSubTaskBase::ESubOperationResult TSubTaskCopyMove::CheckForFreeSpaceFB()
{
	icpf::log_file& rLog = GetContext().GetLog();
	TTaskDefinition& rTaskDefinition = GetContext().GetTaskDefinition();
	IFeedbackHandler* piFeedbackHandler = GetContext().GetFeedbackHandler();
	TLocalFilesystem& rLocalFilesystem = GetContext().GetLocalFilesystem();
	TFileInfoArray& rFilesCache = GetContext().GetFilesCache();

	ull_t ullNeededSize = 0, ullAvailableSize = 0;
	bool bRetry = false;

	do
	{
		bRetry = false;

		rLog.logi(_T("Checking for free space on destination disk..."));

		ullNeededSize = rFilesCache.CalculateTotalSize() - rFilesCache.CalculatePartialSize(m_tProgressInfo.GetCurrentIndex()); // it'd be nice to round up to take cluster size into consideration

		// get free space
		bool bResult = rLocalFilesystem.GetDynamicFreeSpace(rTaskDefinition.GetDestinationPath(), ullAvailableSize);
		if(bResult && ullNeededSize > ullAvailableSize)
		{
			TString strFormat = _T("Not enough free space on disk - needed %needsize bytes for data, available: %availablesize bytes.");
			strFormat.Replace(_t("%needsize"), boost::lexical_cast<std::wstring>(ullNeededSize).c_str());
			strFormat.Replace(_t("%availablesize"), boost::lexical_cast<std::wstring>(ullAvailableSize).c_str());
			rLog.logw(strFormat);

			if(rTaskDefinition.GetSourcePathCount() > 0)
			{
				FEEDBACK_NOTENOUGHSPACE feedStruct = { ullNeededSize, rTaskDefinition.GetSourcePathAt(0).ToString(), rTaskDefinition.GetDestinationPath().ToString() };
				IFeedbackHandler::EFeedbackResult frResult = (IFeedbackHandler::EFeedbackResult)piFeedbackHandler->RequestFeedback(IFeedbackHandler::eFT_NotEnoughSpace, &feedStruct);

				// default
				switch(frResult)
				{
				case IFeedbackHandler::eResult_Cancel:
					rLog.logi(_T("Cancel request while checking for free space on disk."));
					return TSubTaskBase::eSubResult_CancelRequest;

				case IFeedbackHandler::eResult_Retry:
					rLog.logi(_T("Retrying to read drive's free space..."));
					bRetry = true;
					break;

				case IFeedbackHandler::eResult_Ignore:
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

END_CHCORE_NAMESPACE
