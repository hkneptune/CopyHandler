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

namespace chcore
{
	TOverlappedWriterFB::TOverlappedWriterFB(const TFilesystemFileFeedbackWrapperPtr& spSrcFile, const TFilesystemFileFeedbackWrapperPtr& spDstFile,
		const TSubTaskStatsInfoPtr& spStats,
		const TFileInfoPtr& spSrcFileInfo,
		const logger::TLogFileDataPtr& spLogFileData, const TOrderedBufferQueuePtr& spBuffersToWrite,
		unsigned long long ullFilePos, const TBufferListPtr& spEmptyBuffers) :
		m_spWriter(std::make_shared<TOverlappedWriter>(spLogFileData, spBuffersToWrite, ullFilePos, spEmptyBuffers)),
		m_spSrcFile(spSrcFile),
		m_spDstFile(spDstFile),
		m_spStats(spStats),
		m_spSrcFileInfo(spSrcFileInfo),
		m_spEmptyBuffers(spEmptyBuffers)
	{
		if(!spDstFile)
			throw TCoreException(eErr_InvalidArgument, L"spDstFile is NULL", LOCATION);
		if(!spStats)
			throw TCoreException(eErr_InvalidArgument, L"spStats is NULL", LOCATION);
		if(!spSrcFileInfo)
			throw TCoreException(eErr_InvalidArgument, L"spSrcFileInfo is NULL", LOCATION);
		if(!spEmptyBuffers)
			throw TCoreException(eErr_InvalidArgument, L"spEmptyBuffers is NULL", LOCATION);
	}

	TOverlappedWriterFB::~TOverlappedWriterFB()
	{
	}

	TSubTaskBase::ESubOperationResult TOverlappedWriterFB::OnWritePossible(bool& bStopProcessing, bool& bProcessedFlag)
	{
		TOverlappedDataBuffer* pBuffer = m_spWriter->GetWriteBuffer();
		if(!pBuffer)
			throw TCoreException(eErr_InternalProblem, L"Write was possible, but no buffer is available", LOCATION);

		if(m_bReleaseMode)
		{
			m_spEmptyBuffers->Push(pBuffer);
			return TSubTaskBase::eSubResult_Continue;
		}

		bool bSkip = false;
		TSubTaskBase::ESubOperationResult eResult = m_spDstFile->WriteFileFB(*pBuffer, bSkip);
		if(eResult != TSubTaskBase::eSubResult_Continue)
		{
			m_spEmptyBuffers->Push(pBuffer);
			bStopProcessing = true;
		}
		else if(bSkip)
		{
			m_spEmptyBuffers->Push(pBuffer);

			m_spStats->AdjustProcessedSize(m_spStats->GetCurrentItemProcessedSize(), m_spSrcFileInfo->GetLength64());

			bProcessedFlag = false;
			bStopProcessing = true;
		}

		return eResult;
	}

	TSubTaskBase::ESubOperationResult TOverlappedWriterFB::OnWriteFailed(bool& bStopProcessing, bool& bProcessedFlag)
	{
		TOverlappedDataBuffer* pBuffer = m_spWriter->GetFailedWriteBuffer();
		if(!pBuffer)
			throw TCoreException(eErr_InternalProblem, L"Failed to retrieve write failed buffer", LOCATION);

		if(m_bReleaseMode)
		{
			m_spEmptyBuffers->Push(pBuffer);
			return TSubTaskBase::eSubResult_Continue;
		}

		bool bSkip = false;
		TSubTaskBase::ESubOperationResult eResult = m_spDstFile->HandleWriteError(*pBuffer, bSkip);
		if(eResult == TSubTaskBase::eSubResult_Retry)
			m_spWriter->AddFailedWriteBuffer(pBuffer);
		else if(eResult != TSubTaskBase::eSubResult_Continue)
		{
			m_spEmptyBuffers->Push(pBuffer);
			bStopProcessing = true;
		}
		else if(bSkip)
		{
			m_spEmptyBuffers->Push(pBuffer);

			m_spStats->AdjustProcessedSize(m_spStats->GetCurrentItemProcessedSize(), m_spSrcFileInfo->GetLength64());

			bProcessedFlag = false;
			bStopProcessing = true;
		}

		return eResult;
	}

	TSubTaskBase::ESubOperationResult TOverlappedWriterFB::OnWriteFinished(bool& bStopProcessing, bool& bProcessedFlag)
	{
		TOverlappedDataBuffer* pBuffer = m_spWriter->GetFinishedBuffer();
		if(!pBuffer)
			throw TCoreException(eErr_InternalProblem, L"Write finished was possible, but no buffer is available", LOCATION);

		file_size_t fsWritten = pBuffer->GetRealDataSize();

		if(m_bReleaseMode)
		{
			AdjustProcessedSize(fsWritten);

			m_spEmptyBuffers->Push(pBuffer);
			bProcessedFlag = pBuffer->IsLastPart() && (pBuffer->GetBytesTransferred() == fsWritten);

			return TSubTaskBase::eSubResult_Continue;
		}

		TSubTaskBase::ESubOperationResult eResult = TSubTaskBase::eSubResult_Continue;
		if(pBuffer->IsLastPart())
		{
			bool bSkip = false;
			eResult = m_spDstFile->FinalizeFileFB(*pBuffer, bSkip);
			if(eResult != TSubTaskBase::eSubResult_Continue)
			{
				m_spEmptyBuffers->Push(pBuffer);
				bStopProcessing = true;
				return eResult;
			}
			else if(bSkip)
			{
				m_spEmptyBuffers->Push(pBuffer);

				m_spStats->AdjustProcessedSize(m_spStats->GetCurrentItemProcessedSize(), m_spSrcFileInfo->GetLength64());

				bProcessedFlag = false;
				bStopProcessing = true;
				return eResult;
			}
		}

		// in case we read past the original eof, try to get new file size from filesystem
		AdjustProcessedSize(fsWritten);

		// stop iterating through file
		bStopProcessing = pBuffer->IsLastPart();
		if(bStopProcessing)
		{
			m_spWriter->MarkAsFinalized(pBuffer);

			// this is the end of copying of src file - in case it is smaller than expected fix the stats so that difference is accounted for
			AdjustFinalSize();

			bProcessedFlag = true;
			m_spStats->ResetCurrentItemProcessedSize();
		}

		m_spEmptyBuffers->Push(pBuffer);
		return eResult;
	}

	void TOverlappedWriterFB::AdjustProcessedSize(file_size_t fsWritten)
	{
		// in case we read past the original eof, try to get new file size from filesystem
		if(m_spStats->WillAdjustProcessedSizeExceedTotalSize(0, fsWritten))
		{
			file_size_t fsNewSize = m_spSrcFile->GetFileSize();
			if(fsNewSize == m_spSrcFileInfo->GetLength64())
				throw TCoreException(eErr_InternalProblem, L"Read more data from file than it really contained. Possible destination file corruption.", LOCATION);

			m_spStats->AdjustTotalSize(m_spSrcFileInfo->GetLength64(), fsNewSize);
			m_spSrcFileInfo->SetLength64(m_spStats->GetCurrentItemTotalSize());
		}

		m_spStats->AdjustProcessedSize(0, fsWritten);
	}

	void TOverlappedWriterFB::AdjustFinalSize()
	{
		unsigned long long ullCITotalSize = m_spStats->GetCurrentItemTotalSize();
		unsigned long long ullCIProcessedSize = m_spStats->GetCurrentItemProcessedSize();
		if(ullCIProcessedSize < ullCITotalSize)
		{
			file_size_t fsNewSize = m_spSrcFile->GetFileSize();
			if(fsNewSize == m_spSrcFileInfo->GetLength64())
				throw TCoreException(eErr_InternalProblem, L"Read less data from file than it really contained. Possible destination file corruption.", LOCATION);

			if(fsNewSize != ullCIProcessedSize)
				throw TCoreException(eErr_InternalProblem, L"Updated file size still does not match the count of data read. Possible destination file corruption.", LOCATION);

			m_spStats->AdjustTotalSize(ullCITotalSize, fsNewSize);
			m_spSrcFileInfo->SetLength64(fsNewSize);
		}
	}
}
