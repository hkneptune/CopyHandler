// ============================================================================
//  Copyright (C) 2001-2016 by Jozef Starosczyk
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
#include "stdafx.h"
#include "TOverlappedWriterFB.h"
#include "TSubTaskStatsInfo.h"
#include "TFilesystemFileFeedbackWrapper.h"
#include "TFileInfo.h"
#include "TEventGuard.h"
#include "../libchcore/TWorkerThreadController.h"
#include "../libchcore/TCoreWin32Exception.h"
#include "../libchcore/RoundingFunctions.h"

namespace chengine
{
	TOverlappedWriterFB::TOverlappedWriterFB(const IFilesystemPtr& spFilesystem,
		const IFeedbackHandlerPtr& spFeedbackHandler,
		TWorkerThreadController& rThreadController,
		const TSubTaskStatsInfoPtr& spStats,
		const TFileInfoPtr& spSrcFileInfo,
		const TSmartPath& pathDst,
		const logger::TLogFileDataPtr& spLogFileData,
		const TOrderedBufferQueuePtr& spBuffersToWrite,
		const TOverlappedProcessorRangePtr& spRange,
		const TBufferListPtr& spEmptyBuffers,
		size_t stMaxOtfBuffers,
		bool bOnlyCreate,
		bool bNoBuffering,
		bool bProtectReadOnlyFiles,
		bool bUpdateFileAttributesAndTimes) :
		m_counterOnTheFly(),
		m_spWriter(std::make_shared<TOverlappedWriter>(spLogFileData, spBuffersToWrite, spRange, spEmptyBuffers, stMaxOtfBuffers, m_counterOnTheFly.GetSharedCount())),
		m_spStats(spStats),
		m_spSrcFileInfo(spSrcFileInfo),
		m_spDataRange(spRange),
		m_bOnlyCreate(bOnlyCreate),
		m_bUpdateFileAttributesAndTimes(bUpdateFileAttributesAndTimes),
		m_eventProcessingFinished(true, false),
		m_eventWritingFinished(true, false),
		m_eventLocalKill(true, false),
		m_rThreadController(rThreadController),
		m_spLog(logger::MakeLogger(spLogFileData, L"File-Writer"))
	{
		if(!spFilesystem)
			throw TCoreException(eErr_InvalidArgument, L"spFilesystem is NULL", LOCATION);
		if(!spFeedbackHandler)
			throw TCoreException(eErr_InvalidArgument, L"spFeedbackHandler is NULL", LOCATION);
		if(!spStats)
			throw TCoreException(eErr_InvalidArgument, L"spStats is NULL", LOCATION);
		if(!spSrcFileInfo)
			throw TCoreException(eErr_InvalidArgument, L"spSrcFileInfo is NULL", LOCATION);
		if(!spEmptyBuffers)
			throw TCoreException(eErr_InvalidArgument, L"spEmptyBuffers is NULL", LOCATION);
		if(!spRange)
			throw TCoreException(eErr_InvalidArgument, L"spRange is NULL", LOCATION);
		if(!spLogFileData)
			throw TCoreException(eErr_InvalidArgument, L"spLogFileData is NULL", LOCATION);
		if(!spBuffersToWrite)
			throw TCoreException(eErr_InvalidArgument, L"spBuffersToWrite is NULL", LOCATION);

		IFilesystemFilePtr fileDst = spFilesystem->CreateFileObject(IFilesystemFile::eMode_Write, pathDst, bNoBuffering, bProtectReadOnlyFiles);
		m_spDstFile = std::make_shared<TFilesystemFileFeedbackWrapper>(fileDst, spFeedbackHandler, spLogFileData, rThreadController, spFilesystem);
	}

	TOverlappedWriterFB::~TOverlappedWriterFB()
	{
	}

	TSubTaskBase::ESubOperationResult TOverlappedWriterFB::OnWritePossible()
	{
		TOverlappedDataBuffer* pBuffer = m_spWriter->GetWriteBuffer();
		if(!pBuffer)
			return TSubTaskBase::eSubResult_Continue;

		m_counterOnTheFly.Increase();

		pBuffer->SetParam(this);
		TSubTaskBase::ESubOperationResult eResult = m_spDstFile->WriteFileFB(*pBuffer);
		if(eResult != TSubTaskBase::eSubResult_Continue)
		{
			m_spWriter->AddEmptyBuffer(pBuffer);
			m_counterOnTheFly.Decrease();
		}

		return eResult;
	}

	TSubTaskBase::ESubOperationResult TOverlappedWriterFB::OnWriteFailed()
	{
		TOverlappedDataBuffer* pBuffer = m_spWriter->GetFailedWriteBuffer();
		if(!pBuffer)
			throw TCoreException(eErr_InternalProblem, L"Failed to retrieve write failed buffer", LOCATION);

		TSubTaskBase::ESubOperationResult eResult = m_spDstFile->HandleWriteError(*pBuffer);
		if(eResult == TSubTaskBase::eSubResult_Retry)
		{
			m_spDstFile->Close();
			m_spWriter->AddRetryBuffer(pBuffer);
			eResult = TSubTaskBase::eSubResult_Continue;
		}
		else if(eResult != TSubTaskBase::eSubResult_Continue)
			m_spWriter->AddEmptyBuffer(pBuffer);

		return eResult;
	}

	TSubTaskBase::ESubOperationResult TOverlappedWriterFB::OnWriteFinished(bool& bStopProcessing)
	{
		TOverlappedDataBuffer* pBuffer = m_spWriter->GetFinishedBuffer();
		if(!pBuffer)
			throw TCoreException(eErr_InternalProblem, L"Write finished was possible, but no buffer is available", LOCATION);

		file_size_t fsWritten = pBuffer->GetRealDataSize();

		bool bIsLastBuffer = pBuffer->IsLastPart();
		TSubTaskBase::ESubOperationResult eResult = TSubTaskBase::eSubResult_Continue;
		if(bIsLastBuffer)
		{
			eResult = m_spDstFile->FinalizeFileFB(*pBuffer);
			if(eResult == TSubTaskBase::eSubResult_Continue && m_bUpdateFileAttributesAndTimes)
				eResult = m_spDstFile->SetBasicInfo(m_spSrcFileInfo->GetAttributes(), m_spSrcFileInfo->GetCreationTime(), m_spSrcFileInfo->GetLastAccessTime(), m_spSrcFileInfo->GetLastWriteTime());

			if(eResult != TSubTaskBase::eSubResult_Continue)
			{
				m_spWriter->AddEmptyBuffer(pBuffer);
				return eResult;
			}
		}

		// in case we read past the original eof, try to get new file size from filesystem
		AdjustProcessedSize(fsWritten);

		// stop iterating through file
		if(bIsLastBuffer)
		{
			m_spWriter->MarkAsFinalized(pBuffer);

			// this is the end of copying of src file - in case it is smaller than expected fix the stats so that difference is accounted for
			eResult = AdjustFinalSize();
			if(eResult != TSubTaskBase::eSubResult_Continue)
			{
				m_spWriter->AddEmptyBuffer(pBuffer);
				return eResult;
			}
		}

		m_spWriter->AddEmptyBuffer(pBuffer);

		bStopProcessing = bIsLastBuffer;

		return eResult;
	}

	HANDLE TOverlappedWriterFB::GetEventWritingFinishedHandle() const
	{
		return m_eventWritingFinished.Handle();
	}

	HANDLE TOverlappedWriterFB::GetEventProcessingFinishedHandle() const
	{
		return m_eventProcessingFinished.Handle();
	}

	void TOverlappedWriterFB::QueueProcessedBuffer(TOverlappedDataBuffer* pBuffer)
	{
		if(pBuffer->HasError())
			m_spWriter->AddFailedWriteBuffer(pBuffer);
		else
			m_spWriter->AddFinishedBuffer(pBuffer);

		m_counterOnTheFly.Decrease();
	}

	void TOverlappedWriterFB::ClearBuffers()
	{
		m_spWriter->ClearBuffers();
	}

	void TOverlappedWriterFB::AdjustProcessedSize(file_size_t fsWritten)
	{
		// in case we read past the original eof, try to get new file size from filesystem
		if(m_spStats->WillAdjustProcessedSizeExceedTotalSize(0, fsWritten))
			throw TCoreException(eErr_InternalProblem, L"Read more data from file than it really contained. Possible destination file corruption.", LOCATION);

		m_spStats->AdjustProcessedSize(0, fsWritten);
	}

	TSubTaskBase::ESubOperationResult TOverlappedWriterFB::AdjustFinalSize()
	{
		TSubTaskBase::ESubOperationResult eResult = TSubTaskBase::eSubResult_Continue;

		unsigned long long ullCITotalSize = m_spStats->GetCurrentItemTotalSize();
		unsigned long long ullCIProcessedSize = m_spStats->GetCurrentItemProcessedSize();
		if(ullCIProcessedSize < ullCITotalSize)
		{
			if(m_spSrcFileInfo->GetLength64() != ullCIProcessedSize)
				throw TCoreException(eErr_InternalProblem, L"Updated file size still does not match the count of data read. Possible destination file corruption.", LOCATION);

			m_spStats->AdjustTotalSize(ullCITotalSize, m_spSrcFileInfo->GetLength64());
		}

		return eResult;
	}

	TSubTaskBase::ESubOperationResult TOverlappedWriterFB::Start()
	{
		// open destination file, handle the failures and possibly existence of the destination file
		unsigned long long ullProcessedSize = m_spStats->GetCurrentItemProcessedSize();
		unsigned long long ullSeekTo = ullProcessedSize;

		bool bDstFileFreshlyCreated = false;
		TSubTaskBase::ESubOperationResult eResult = m_spDstFile->IsFreshlyCreated(bDstFileFreshlyCreated);
		if(eResult != TSubTaskBase::eSubResult_Continue)
			return eResult;

		file_size_t fsDstFileSize = 0;
		eResult = m_spDstFile->GetFileSize(fsDstFileSize);
		if(eResult != TSubTaskBase::eSubResult_Continue)
			return eResult;

		// try to resume if possible
		bool bCanSilentResume = false;
		if(m_spStats->CanCurrentItemSilentResume())
		{
			if(fsDstFileSize == ullProcessedSize && fsDstFileSize <= m_spSrcFileInfo->GetLength64())
			{
				ullSeekTo = fsDstFileSize;
				bCanSilentResume = true;
			}
			else if(fsDstFileSize > ullProcessedSize && fsDstFileSize - ullProcessedSize == IFilesystemFile::MaxSectorSize && 
				m_spSrcFileInfo->GetLength64() > ullProcessedSize && m_spSrcFileInfo->GetLength64() < fsDstFileSize)
			{
				// special case - resuming file that was not finalized completely last time
				ullSeekTo = ullProcessedSize;
				bCanSilentResume = true;
			}
		}

		if(!bCanSilentResume && !bDstFileFreshlyCreated && fsDstFileSize > 0)
		{
			bool bShouldAppend = false;
			eResult = m_spDstFile->HandleFileAlreadyExistsFB(m_spSrcFileInfo, bShouldAppend);
			if(eResult != TSubTaskBase::eSubResult_Continue)
				return eResult;

			if(bShouldAppend)
				ullSeekTo = std::min(fsDstFileSize, m_spSrcFileInfo->GetLength64());
			else
				ullSeekTo = 0;
		}

		if(m_bOnlyCreate)
		{
			// we don't copy contents, but need to increase processed size
			m_spStats->AdjustProcessedSize(m_spStats->GetCurrentItemProcessedSize(), m_spSrcFileInfo->GetLength64());

			return TSubTaskBase::eSubResult_Continue;
		}

		// ullSeekTo contains the seek position in destination file; in case the destination is already
		// larger than source file all we can do is to perform truncation of destination file to the size of
		// source file.
		// NOTE: the truncation that will be the result of the following assignment might cause the end of destination file
		// to be overwritten by the end of source file.
		ullSeekTo = std::min(ullSeekTo, m_spSrcFileInfo->GetLength64());

		// seek to the position where copying will start
		file_size_t fsMoveTo = m_spDstFile->GetSeekPositionForResume(ullSeekTo);

		// sanity check
		if(bDstFileFreshlyCreated && ullSeekTo != 0)
			throw TCoreException(eErr_InternalProblem, L"Destination file was freshly created, but seek position is not 0", LOCATION);
		if(fsMoveTo > ullSeekTo)
			throw TCoreException(eErr_InternalProblem, L"File position to move to is placed after the end of file", LOCATION);

		// adjust the stats for the difference between what was already processed and what will now be considered processed
		m_spStats->AdjustProcessedSize(ullProcessedSize, fsMoveTo);

		m_spDataRange->SetResumePosition(fsMoveTo);

		// if the destination file already exists - truncate it to the current file position
		if(!bDstFileFreshlyCreated)
		{
			// if destination file was opened (as opposed to newly created)
			eResult = m_spDstFile->TruncateFileFB(fsMoveTo);
			if(eResult != TSubTaskBase::eSubResult_Continue)
				return eResult;
		}

		// at this point user already decided that he want to write data into destination file;
		// so if we're to resume copying after this point, we don't have to ask user for overwriting existing file
		m_spStats->SetCurrentItemSilentResume(true);

		return eResult;
	}

	void TOverlappedWriterFB::StartThreaded()
	{
		m_eventProcessingFinished.ResetEvent();
		TEventGuard guardProcessingFinished(m_eventProcessingFinished, true);

		m_eThreadResult = TSubTaskBase::eSubResult_Continue;

		enum { eKillThread, eLocalKill, eWriteFinished, eWriteFailed, eWritePossible };

		std::vector<HANDLE> vHandles = {
			m_rThreadController.GetKillThreadHandle(),
			m_eventLocalKill.Handle(),
			m_spWriter->GetEventWriteFinishedHandle(),
			m_spWriter->GetEventWriteFailedHandle(),
			m_spWriter->GetEventWritePossibleHandle()
		};

		bool bWrittenLastBuffer = false;
		try
		{
			while(!bWrittenLastBuffer && m_eThreadResult == TSubTaskBase::eSubResult_Continue)
			{
				DWORD dwResult = WaitForMultipleObjectsEx(boost::numeric_cast<DWORD>(vHandles.size()), vHandles.data(), false, INFINITE, true);
				switch(dwResult)
				{
				case STATUS_USER_APC:
					break;

				case WAIT_OBJECT_0 + eKillThread:
				case WAIT_OBJECT_0 + eLocalKill:
					m_eThreadResult = TSubTaskBase::eSubResult_KillRequest;
					break;

				case WAIT_OBJECT_0 + eWritePossible:
					m_eThreadResult = OnWritePossible();
					break;

				case WAIT_OBJECT_0 + eWriteFailed:
					m_eThreadResult = OnWriteFailed();
					break;

				case WAIT_OBJECT_0 + eWriteFinished:
					m_eThreadResult = OnWriteFinished(bWrittenLastBuffer);
					break;

				default:
					DWORD dwLastError = GetLastError();
					throw TCoreWin32Exception(eErr_UnhandledCase, dwLastError, L"Unknown result from async waiting function", LOCATION);
				}
			}
		}
		catch(const std::exception&)
		{
			m_eThreadResult = TSubTaskBase::eSubResult_Error;
		}

		m_spDstFile->CancelIo();

		WaitForOnTheFlyBuffers();
		ClearBuffers();
		
		// update stats with the written bytes info
		UpdateCurrentItemStatsFromFileSize(bWrittenLastBuffer);

		LOG_DEBUG(m_spLog) << L"Writer stopping processing. Max on-the-fly requests: " << m_counterOnTheFly.GetMaxUsed();

		if(m_eThreadResult == TSubTaskBase::eSubResult_Continue && bWrittenLastBuffer)
			m_eventWritingFinished.SetEvent();
	}

	void TOverlappedWriterFB::UpdateCurrentItemStatsFromFileSize(bool bFileWritingFinished)
	{
		if(bFileWritingFinished)
		{
			m_spStats->ResetCurrentItemProcessedSize();
		}
		else
		{
			file_size_t fsDstFileSize = 0;
			TSubTaskBase::ESubOperationResult eResult = m_spDstFile->GetFileSize(fsDstFileSize, true);
			if(eResult == TSubTaskBase::eSubResult_Continue)
			{
				unsigned long long ullTotalCISize = m_spSrcFileInfo->GetLength64(); //m_spStats->GetCurrentItemTotalSize();
				if(fsDstFileSize > ullTotalCISize)
				{
					// special case, when destination file size might be bigger than the expected source size;
					// that means the file was not correctly finalized and needs to be truncated after resume
					if(RoundUp<unsigned long long>(ullTotalCISize, IFilesystemFile::MaxSectorSize) == fsDstFileSize)
						fsDstFileSize -= IFilesystemFile::MaxSectorSize;
				}

				m_spStats->AdjustProcessedSize(m_spStats->GetCurrentItemProcessedSize(), fsDstFileSize);
			}
		}
	}

	TSubTaskBase::ESubOperationResult TOverlappedWriterFB::StopThreaded()
	{
		m_eventLocalKill.SetEvent();

		DWORD dwResult = WaitForSingleObjectEx(m_eventProcessingFinished.Handle(), INFINITE, FALSE);
		if(dwResult != WAIT_OBJECT_0)
			throw TCoreException(eErr_InternalProblem, L"Failed to wait writer processing to finish", LOCATION);

		return m_eThreadResult;
	}

	void TOverlappedWriterFB::WaitForOnTheFlyBuffers()
	{
		bool bStop = false;
		do
		{
			DWORD dwResult = WaitForSingleObjectEx(m_counterOnTheFly.GetEventHandle(), INFINITE, TRUE);
			switch(dwResult)
			{
			case STATUS_USER_APC:
				break;

			case WAIT_OBJECT_0:
				bStop = true;
				break;

			default:
				throw TCoreException(eErr_UnhandledCase, L"Unknown result from async waiting function", LOCATION);
			}
		}
		while(!bStop);
	}
}
