// ============================================================================
//  Copyright (C) 2001-2015 by Jozef Starosczyk
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
#include "TFilesystemFileFeedbackWrapper.h"
#include "TFileException.h"
#include <boost/lexical_cast.hpp>
#include "TFileInfo.h"
#include "TWorkerThreadController.h"
#include "TOverlappedDataBuffer.h"

namespace chcore
{
	TFilesystemFileFeedbackWrapper::TFilesystemFileFeedbackWrapper(const IFilesystemFilePtr& spFile, 
		const IFeedbackHandlerPtr& spFeedbackHandler, const logger::TLogFileDataPtr& spLogFileData,
		TWorkerThreadController& rThreadController, const IFilesystemPtr& spFilesystem) :
		m_spFile(spFile),
		m_spFeedbackHandler(spFeedbackHandler),
		m_spLog(std::make_unique<logger::TLogger>(spLogFileData, L"Filesystem-File")),
		m_rThreadController(rThreadController),
		m_spFilesystem(spFilesystem)
	{
		if (!spFeedbackHandler)
			throw TCoreException(eErr_InvalidArgument, L"spFeedbackHandler is NULL", LOCATION);
		if (!spFile)
			throw TCoreException(eErr_InvalidArgument, L"spFile is NULL", LOCATION);
		if (!spFilesystem)
			throw TCoreException(eErr_InvalidArgument, L"spFilesystem is NULL", LOCATION);
	}

	TSubTaskBase::ESubOperationResult TFilesystemFileFeedbackWrapper::HandleFileAlreadyExistsFB(const TFileInfoPtr& spSrcFileInfo, bool& bShouldAppend)
	{
		bShouldAppend = false;

		// read info about the existing destination file,
		TFileInfo tDstFileInfo;
		m_spFile->GetFileInfo(tDstFileInfo);

		// src and dst files are the same
		TFeedbackResult frResult = m_spFeedbackHandler->FileAlreadyExists(*spSrcFileInfo, tDstFileInfo);
		switch(frResult.GetResult())
		{
		case eResult_Overwrite:
			bShouldAppend = false;
			return TSubTaskBase::eSubResult_Continue;

		case eResult_CopyRest:
			bShouldAppend = true;
			return TSubTaskBase::eSubResult_Continue;

		case eResult_Skip:
			return TSubTaskBase::eSubResult_SkipFile;

		case eResult_Cancel:
		{
			// log
			TString strFormat = _T("Cancel request when handling already existing file %path");
			strFormat.Replace(_T("%path"), m_spFile->GetFilePath().ToString());
			LOG_INFO(m_spLog) << strFormat.c_str();

			return TSubTaskBase::eSubResult_CancelRequest;
		}
		case eResult_Pause:
			return TSubTaskBase::eSubResult_PauseRequest;

		default:
			BOOST_ASSERT(FALSE);		// unknown result
			throw TCoreException(eErr_UnhandledCase, L"Feedback result unknown", LOCATION);
		}
	}

	TSubTaskBase::ESubOperationResult TFilesystemFileFeedbackWrapper::TruncateFileFB(file_size_t fsNewSize)
	{
		bool bRetry = false;
		do
		{
			DWORD dwLastError = ERROR_SUCCESS;

			try
			{
				m_spFile->Truncate(fsNewSize);
				return TSubTaskBase::eSubResult_Continue;
			}
			catch (const TFileException& e)
			{
				dwLastError = e.GetNativeError();
			}

			TString strFormat = _T("Error %errno while truncating file %path to 0");
			strFormat.Replace(_T("%errno"), boost::lexical_cast<std::wstring>(dwLastError).c_str());
			strFormat.Replace(_T("%path"), m_spFile->GetFilePath().ToString());
			LOG_ERROR(m_spLog) << strFormat.c_str();

			TFeedbackResult frResult = m_spFeedbackHandler->FileError(m_spFile->GetFilePath().ToWString(), TString(), EFileError::eResizeError, dwLastError);
			switch (frResult.GetResult())
			{
			case eResult_Cancel:
				return TSubTaskBase::eSubResult_CancelRequest;

			case eResult_Retry:
				bRetry = true;
				break;

			case eResult_Pause:
				return TSubTaskBase::eSubResult_PauseRequest;

			case eResult_Skip:
				return TSubTaskBase::eSubResult_SkipFile;

			default:
				BOOST_ASSERT(FALSE);		// unknown result
				throw TCoreException(eErr_UnhandledCase, L"Feedback result unknown", LOCATION);
			}

			if(WasKillRequested(frResult))
				return TSubTaskBase::eSubResult_KillRequest;
		}
		while(bRetry);

		return TSubTaskBase::eSubResult_Continue;
	}

	TSubTaskBase::ESubOperationResult TFilesystemFileFeedbackWrapper::ReadFileFB(TOverlappedDataBuffer& rBuffer)
	{
		bool bRetry = false;
		do
		{
			bRetry = false;

			DWORD dwLastError = ERROR_SUCCESS;

			try
			{
				m_spFile->ReadFile(rBuffer);
				return TSubTaskBase::eSubResult_Continue;
			}
			catch (const TFileException& e)
			{
				dwLastError = e.GetNativeError();
			}

			TString strFormat = _T("Error %errno while requesting read of %count bytes from source file %path");
			strFormat.Replace(_T("%errno"), boost::lexical_cast<std::wstring>(dwLastError).c_str());
			strFormat.Replace(_T("%count"), boost::lexical_cast<std::wstring>(rBuffer.GetRequestedDataSize()).c_str());
			strFormat.Replace(_T("%path"), m_spFile->GetFilePath().ToString());
			LOG_ERROR(m_spLog) << strFormat.c_str();

			TFeedbackResult frResult = m_spFeedbackHandler->FileError(m_spFile->GetFilePath().ToWString(), TString(), EFileError::eReadError, dwLastError);
			switch (frResult.GetResult())
			{
			case eResult_Cancel:
				return TSubTaskBase::eSubResult_CancelRequest;

			case eResult_Retry:
				bRetry = true;
				break;

			case eResult_Pause:
				return TSubTaskBase::eSubResult_PauseRequest;

			case eResult_Skip:
				return TSubTaskBase::eSubResult_SkipFile;

			default:
				BOOST_ASSERT(FALSE);		// unknown result
				throw TCoreException(eErr_UnhandledCase, L"Feedback result unknown", LOCATION);
			}

			if(WasKillRequested(frResult))
				return TSubTaskBase::eSubResult_KillRequest;
		}
		while(bRetry);

		return TSubTaskBase::eSubResult_Continue;
	}

	TSubTaskBase::ESubOperationResult TFilesystemFileFeedbackWrapper::WriteFileFB(TOverlappedDataBuffer& rBuffer)
	{
		bool bRetry = false;
		do
		{
			bRetry = false;

			DWORD dwLastError = ERROR_SUCCESS;

			try
			{
				m_spFile->WriteFile(rBuffer);
				return TSubTaskBase::eSubResult_Continue;
			}
			catch (const TFileException& e)
			{
				dwLastError = e.GetNativeError();
			}

			TString strFormat = _T("Error %errno while trying to write %count bytes to destination file %path");
			strFormat.Replace(_T("%errno"), boost::lexical_cast<std::wstring>(dwLastError).c_str());
			strFormat.Replace(_T("%count"), boost::lexical_cast<std::wstring>(rBuffer.GetBytesTransferred()).c_str());
			strFormat.Replace(_T("%path"), m_spFile->GetFilePath().ToString());
			LOG_ERROR(m_spLog) << strFormat.c_str();

			TFeedbackResult frResult = m_spFeedbackHandler->FileError(m_spFile->GetFilePath().ToWString(), TString(), EFileError::eWriteError, dwLastError);
			switch (frResult.GetResult())
			{
			case eResult_Cancel:
				return TSubTaskBase::eSubResult_CancelRequest;

			case eResult_Retry:
				bRetry = true;
				break;

			case eResult_Pause:
				return TSubTaskBase::eSubResult_PauseRequest;

			case eResult_Skip:
				return TSubTaskBase::eSubResult_SkipFile;

			default:
				BOOST_ASSERT(FALSE);		// unknown result
				throw TCoreException(eErr_UnhandledCase, L"Feedback result unknown", LOCATION);
			}

			if(WasKillRequested(frResult))
				return TSubTaskBase::eSubResult_KillRequest;
		}
		while(bRetry);

		return TSubTaskBase::eSubResult_Continue;
	}

	TSubTaskBase::ESubOperationResult TFilesystemFileFeedbackWrapper::FinalizeFileFB(TOverlappedDataBuffer& rBuffer)
	{
		bool bRetry = false;
		do
		{
			bRetry = false;

			DWORD dwLastError = ERROR_SUCCESS;

			try
			{
				m_spFile->FinalizeFile(rBuffer);
				return TSubTaskBase::eSubResult_Continue;
			}
			catch (const TFileException& e)
			{
				dwLastError = e.GetNativeError();
			}

			TString strFormat = _T("Error %errno while trying to finalize file %path");
			strFormat.Replace(_T("%errno"), boost::lexical_cast<std::wstring>(dwLastError).c_str());
			strFormat.Replace(_T("%path"), m_spFile->GetFilePath().ToString());
			LOG_ERROR(m_spLog) << strFormat.c_str();

			TFeedbackResult frResult = m_spFeedbackHandler->FileError(m_spFile->GetFilePath().ToWString(), TString(), EFileError::eFinalizeError, dwLastError);
			switch (frResult.GetResult())
			{
			case eResult_Cancel:
				return TSubTaskBase::eSubResult_CancelRequest;

			case eResult_Retry:
				bRetry = true;
				break;

			case eResult_Pause:
				return TSubTaskBase::eSubResult_PauseRequest;

			case eResult_Skip:
				return TSubTaskBase::eSubResult_SkipFile;

			default:
				BOOST_ASSERT(FALSE);		// unknown result
				throw TCoreException(eErr_UnhandledCase, L"Feedback result unknown", LOCATION);
			}

			if(WasKillRequested(frResult))
				return TSubTaskBase::eSubResult_KillRequest;
		}
		while(bRetry);

		return TSubTaskBase::eSubResult_Continue;
	}

	TSubTaskBase::ESubOperationResult TFilesystemFileFeedbackWrapper::HandleReadError(TOverlappedDataBuffer& rBuffer)
	{
		DWORD dwLastError = rBuffer.GetErrorCode();

		// log
		TString strFormat = _T("Error %errno while requesting read of %count bytes from source file %path");
		strFormat.Replace(_T("%errno"), boost::lexical_cast<std::wstring>(dwLastError).c_str());
		strFormat.Replace(_T("%count"), boost::lexical_cast<std::wstring>(rBuffer.GetRequestedDataSize()).c_str());
		strFormat.Replace(_T("%path"), m_spFile->GetFilePath().ToString());
		LOG_ERROR(m_spLog) << strFormat.c_str();

		TFeedbackResult frResult = m_spFeedbackHandler->FileError(m_spFile->GetFilePath().ToWString(), TString(), EFileError::eReadError, dwLastError);
		switch(frResult.GetResult())
		{
		case eResult_Cancel:
			return TSubTaskBase::eSubResult_CancelRequest;

		case eResult_Retry:
			return TSubTaskBase::eSubResult_Retry;

		case eResult_Pause:
			return TSubTaskBase::eSubResult_PauseRequest;

		case eResult_Skip:
			return TSubTaskBase::eSubResult_SkipFile;

		default:
			BOOST_ASSERT(FALSE);		// unknown result
			throw TCoreException(eErr_UnhandledCase, L"Unknown feedback result", LOCATION);
		}
	}

	TSubTaskBase::ESubOperationResult TFilesystemFileFeedbackWrapper::HandleWriteError(TOverlappedDataBuffer& rBuffer)
	{
		DWORD dwLastError = rBuffer.GetErrorCode();

		// log
		TString strFormat = _T("Error %errno while trying to write %count bytes to destination file %path");
		strFormat.Replace(_T("%errno"), boost::lexical_cast<std::wstring>(rBuffer.GetErrorCode()).c_str());
		strFormat.Replace(_T("%count"), boost::lexical_cast<std::wstring>(rBuffer.GetBytesTransferred()).c_str());
		strFormat.Replace(_T("%path"), m_spFile->GetFilePath().ToString());
		LOG_ERROR(m_spLog) << strFormat.c_str();

		TFeedbackResult frResult = m_spFeedbackHandler->FileError(m_spFile->GetFilePath().ToWString(), TString(), EFileError::eWriteError, dwLastError);
		switch(frResult.GetResult())
		{
		case eResult_Cancel:
			return TSubTaskBase::eSubResult_CancelRequest;

		case eResult_Retry:
			return TSubTaskBase::eSubResult_Retry;

		case eResult_Pause:
			return TSubTaskBase::eSubResult_PauseRequest;

		case eResult_Skip:
			return TSubTaskBase::eSubResult_SkipFile;

		default:
			BOOST_ASSERT(FALSE);		// unknown result
			throw TCoreException(eErr_UnhandledCase, L"Unknown feedback result", LOCATION);
		}
	}

	TSubTaskBase::ESubOperationResult TFilesystemFileFeedbackWrapper::IsFreshlyCreated(bool& bIsFreshlyCreated) const
	{
		bool bRetry = false;
		do
		{
			bRetry = false;

			DWORD dwLastError = ERROR_SUCCESS;

			try
			{
				bIsFreshlyCreated = m_spFile->IsFreshlyCreated();
				return TSubTaskBase::eSubResult_Continue;
			}
			catch(const TFileException& e)
			{
				dwLastError = e.GetNativeError();
			}

			TString strFormat = _T("Error %errno while trying to retrieve freshly-created flag for file %path");
			strFormat.Replace(_T("%errno"), boost::lexical_cast<std::wstring>(dwLastError).c_str());
			strFormat.Replace(_T("%path"), m_spFile->GetFilePath().ToString());
			LOG_ERROR(m_spLog) << strFormat.c_str();

			TFeedbackResult frResult = m_spFeedbackHandler->FileError(m_spFile->GetFilePath().ToWString(), TString(), EFileError::eCreateError, dwLastError);
			switch(frResult.GetResult())
			{
			case eResult_Cancel:
				return TSubTaskBase::eSubResult_CancelRequest;

			case eResult_Retry:
				bRetry = true;
				break;

			case eResult_Pause:
				return TSubTaskBase::eSubResult_PauseRequest;

			case eResult_Skip:
				return TSubTaskBase::eSubResult_SkipFile;

			default:
				BOOST_ASSERT(FALSE);		// unknown result
				throw TCoreException(eErr_UnhandledCase, L"Feedback result unknown", LOCATION);
			}

			if(WasKillRequested(frResult))
				return TSubTaskBase::eSubResult_KillRequest;
		}
		while(bRetry);

		return TSubTaskBase::eSubResult_Continue;
	}

	TSmartPath TFilesystemFileFeedbackWrapper::GetFilePath() const
	{
		return m_spFile->GetFilePath();
	}

	TSubTaskBase::ESubOperationResult TFilesystemFileFeedbackWrapper::GetFileSize(file_size_t& fsSize) const
	{
		bool bRetry = false;
		do
		{
			bRetry = false;

			DWORD dwLastError = ERROR_SUCCESS;

			try
			{
				fsSize = m_spFile->GetFileSize();
				return TSubTaskBase::eSubResult_Continue;
			}
			catch(const TFileException& e)
			{
				dwLastError = e.GetNativeError();
			}

			TString strFormat = _T("Error %errno while trying to retrieve file size of file %path");
			strFormat.Replace(_T("%errno"), boost::lexical_cast<std::wstring>(dwLastError).c_str());
			strFormat.Replace(_T("%path"), m_spFile->GetFilePath().ToString());
			LOG_ERROR(m_spLog) << strFormat.c_str();

			TFeedbackResult frResult = m_spFeedbackHandler->FileError(m_spFile->GetFilePath().ToWString(), TString(), EFileError::eRetrieveFileInfo, dwLastError);
			switch(frResult.GetResult())
			{
			case eResult_Cancel:
				return TSubTaskBase::eSubResult_CancelRequest;

			case eResult_Retry:
				bRetry = true;
				break;

			case eResult_Pause:
				return TSubTaskBase::eSubResult_PauseRequest;

			case eResult_Skip:
				return TSubTaskBase::eSubResult_SkipFile;

			default:
				BOOST_ASSERT(FALSE);		// unknown result
				throw TCoreException(eErr_UnhandledCase, L"Feedback result unknown", LOCATION);
			}

			if(WasKillRequested(frResult))
				return TSubTaskBase::eSubResult_KillRequest;
		}
		while(bRetry);

		return TSubTaskBase::eSubResult_Continue;
	}

	file_size_t TFilesystemFileFeedbackWrapper::GetSeekPositionForResume(file_size_t fsLastAvailablePosition)
	{
		return m_spFile->GetSeekPositionForResume(fsLastAvailablePosition);
	}

	bool TFilesystemFileFeedbackWrapper::WasKillRequested(const TFeedbackResult& rFeedbackResult) const
	{
		if(m_rThreadController.KillRequested(rFeedbackResult.IsAutomatedReply() ? m_spFeedbackHandler->GetRetryInterval() : 0))
			return true;
		return false;
	}
}
