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
	TOverlappedReaderFB::TOverlappedReaderFB(const TFilesystemFileFeedbackWrapperPtr& spSrcFile, const TSubTaskStatsInfoPtr& spStats,
		const TFileInfoPtr& spSrcFileInfo,
		const logger::TLogFileDataPtr& spLogFileData, const TBufferListPtr& spEmptyBuffers,
		unsigned long long ullFilePos, DWORD dwChunkSize) :
		m_spReader(std::make_shared<TOverlappedReader>(spLogFileData, spEmptyBuffers, ullFilePos, dwChunkSize)),
		m_spSrcFile(spSrcFile),
		m_spStats(spStats),
		m_spSrcFileInfo(spSrcFileInfo)
	{
		if(!spSrcFile)
			throw TCoreException(eErr_InvalidArgument, L"spSrcFile is NULL", LOCATION);
		if(!spStats)
			throw TCoreException(eErr_InvalidArgument, L"spStats is NULL", LOCATION);
		if(!spSrcFileInfo)
			throw TCoreException(eErr_InvalidArgument, L"spSrcFileInfo is NULL", LOCATION);
	}

	TOverlappedReaderFB::~TOverlappedReaderFB()
	{
	}

	TSubTaskBase::ESubOperationResult TOverlappedReaderFB::OnReadPossible(bool& bStopProcessing, bool& bProcessedFlag)
	{
		TOverlappedDataBuffer* pBuffer = m_spReader->GetEmptyBuffer();
		if(!pBuffer)
			throw TCoreException(eErr_InternalProblem, L"Read was possible, but no buffer is available", LOCATION);

		bool bSkip = false;
		TSubTaskBase::ESubOperationResult eResult = m_spSrcFile->ReadFileFB(*pBuffer, bSkip);
		if(eResult != TSubTaskBase::eSubResult_Continue)
		{
			m_spReader->AddEmptyBuffer(pBuffer, false);
			bStopProcessing = true;
		}
		else if(bSkip)
		{
			m_spReader->AddEmptyBuffer(pBuffer, false);

			m_spStats->AdjustProcessedSize(m_spStats->GetCurrentItemProcessedSize(), m_spSrcFileInfo->GetLength64());

			bProcessedFlag = false;
			bStopProcessing = true;
		}

		return eResult;
	}

	TSubTaskBase::ESubOperationResult TOverlappedReaderFB::OnReadFailed(bool& bStopProcessing, bool& bProcessedFlag)
	{
		TOverlappedDataBuffer* pBuffer = m_spReader->GetFailedReadBuffer();
		if(!pBuffer)
			throw TCoreException(eErr_InternalProblem, L"Cannot retrieve failed read buffer", LOCATION);

		// read error encountered - handle it
		bool bSkip = false;
		TSubTaskBase::ESubOperationResult eResult = m_spSrcFile->HandleReadError(*pBuffer, bSkip);
		if(eResult == TSubTaskBase::eSubResult_Retry)
			m_spReader->AddEmptyBuffer(pBuffer, true);
		else if(eResult != TSubTaskBase::eSubResult_Continue)
		{
			m_spReader->AddEmptyBuffer(pBuffer, false);
			bStopProcessing = true;
		}
		else if(bSkip)
		{
			m_spReader->AddEmptyBuffer(pBuffer, false);

			m_spStats->AdjustProcessedSize(m_spStats->GetCurrentItemProcessedSize(), m_spSrcFileInfo->GetLength64());

			bProcessedFlag = false;
			bStopProcessing = true;
		}

		return eResult;
	}
}
