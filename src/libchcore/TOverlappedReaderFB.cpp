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
#include "TOverlappedReaderFB.h"
#include "TCoreException.h"
#include "TFileInfo.h"

namespace chcore
{
	TOverlappedReaderFB::TOverlappedReaderFB(const IFilesystemPtr& spFilesystem,
		const IFeedbackHandlerPtr& spFeedbackHandler,
		TWorkerThreadController& rThreadController,
		const TSubTaskStatsInfoPtr& spStats,
		const TFileInfoPtr& spSrcFileInfo,
		const logger::TLogFileDataPtr& spLogFileData,
		const TBufferListPtr& spEmptyBuffers,
		const TOverlappedProcessorRangePtr& spDataRange,
		DWORD dwChunkSize,
		bool bNoBuffering,
		bool bProtectReadOnlyFiles) :
		m_spReader(std::make_shared<TOverlappedReader>(spLogFileData, spEmptyBuffers, spDataRange, dwChunkSize)),
		m_spFilesystem(spFilesystem),
		m_spSrcFile(),
		m_spStats(spStats),
		m_spSrcFileInfo(spSrcFileInfo)
	{
		if(!spFeedbackHandler)
			throw TCoreException(eErr_InvalidArgument, L"spFeedbackHandler is NULL", LOCATION);
		if(!spFilesystem)
			throw TCoreException(eErr_InvalidArgument, L"spFilesystem is NULL", LOCATION);
		if(!spStats)
			throw TCoreException(eErr_InvalidArgument, L"spStats is NULL", LOCATION);
		if(!spSrcFileInfo)
			throw TCoreException(eErr_InvalidArgument, L"spSrcFileInfo is NULL", LOCATION);

		IFilesystemFilePtr fileSrc = m_spFilesystem->CreateFileObject(IFilesystemFile::eMode_Read, m_spSrcFileInfo->GetFullFilePath(), bNoBuffering, bProtectReadOnlyFiles);
		m_spSrcFile = std::make_shared<TFilesystemFileFeedbackWrapper>(fileSrc, spFeedbackHandler, spLogFileData, rThreadController, spFilesystem);
	}

	TOverlappedReaderFB::~TOverlappedReaderFB()
	{
	}

	TSubTaskBase::ESubOperationResult TOverlappedReaderFB::Start()
	{
		TSubTaskBase::ESubOperationResult eResult = UpdateFileStats();
		return eResult;
	}

	TOverlappedReaderPtr TOverlappedReaderFB::GetReader() const
	{
		return m_spReader;
	}

	TSubTaskBase::ESubOperationResult TOverlappedReaderFB::UpdateFileStats()
	{
		// update the source file size (it might differ from the time this file was originally scanned).
		// NOTE: this kind of update could be also done when copying chunks of data beyond the original end-of-file,
		//       but it would require frequent total size updates and thus - serializations).
		// NOTE2: the by-chunk corrections of stats are still applied when copying to ensure even further size
		//        matching; this update however still allows for better serialization management.
		file_size_t fsOldSize = m_spSrcFileInfo->GetLength64();
		file_size_t fsNewSize = 0;

		TSubTaskBase::ESubOperationResult eResult = m_spSrcFile->GetFileSize(fsNewSize);
		if(eResult != TSubTaskBase::eSubResult_Continue)
			return eResult;

		if(fsNewSize != fsOldSize)
		{
			m_spStats->AdjustTotalSize(fsOldSize, fsNewSize);
			m_spSrcFileInfo->SetLength64(fsNewSize);
		}

		return eResult;
	}

	void TOverlappedReaderFB::SetReleaseMode()
	{
		m_spReader->ReleaseBuffers();
	}

	TSubTaskBase::ESubOperationResult TOverlappedReaderFB::OnReadPossible()
	{
		TOverlappedDataBuffer* pBuffer = m_spReader->GetEmptyBuffer();
		if(!pBuffer)
			throw TCoreException(eErr_InternalProblem, L"Read was possible, but no buffer is available", LOCATION);

		TSubTaskBase::ESubOperationResult eResult = m_spSrcFile->ReadFileFB(*pBuffer);
		if(eResult != TSubTaskBase::eSubResult_Continue)
			m_spReader->AddEmptyBuffer(pBuffer);

		return eResult;
	}

	TSubTaskBase::ESubOperationResult TOverlappedReaderFB::OnReadFailed()
	{
		TOverlappedDataBuffer* pBuffer = m_spReader->GetFailedReadBuffer();
		if(!pBuffer)
			throw TCoreException(eErr_InternalProblem, L"Cannot retrieve failed read buffer", LOCATION);

		// read error encountered - handle it
		TSubTaskBase::ESubOperationResult eResult = m_spSrcFile->HandleReadError(*pBuffer);
		if(eResult == TSubTaskBase::eSubResult_Retry)
		{
			m_spSrcFile->Close();
			m_spReader->AddRetryBuffer(pBuffer);
			eResult = TSubTaskBase::eSubResult_Continue;
		}
		else if(eResult != TSubTaskBase::eSubResult_Continue)
			m_spReader->AddEmptyBuffer(pBuffer);

		return eResult;
	}
}
