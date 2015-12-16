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

namespace chcore
{
	TFilesystemFileFeedbackWrapper::TFilesystemFileFeedbackWrapper(icpf::log_file& rLog) :
		m_rLog(rLog)
	{
	}

	TSubTaskBase::ESubOperationResult TFilesystemFileFeedbackWrapper::OpenSourceFileFB(const IFeedbackHandlerPtr& spFeedbackHandler, const IFilesystemFilePtr& fileSrc)
	{
		bool bRetry = false;

		fileSrc->Close();

		do
		{
			bRetry = false;
			DWORD dwLastError = ERROR_SUCCESS;

			try
			{
				fileSrc->OpenExistingForReading();

				return TSubTaskBase::eSubResult_Continue;
			}
			catch (const TFileException& e)
			{
				dwLastError = e.GetNativeError();
			}

			EFeedbackResult frResult = spFeedbackHandler->FileError(fileSrc->GetFilePath().ToWString(), TString(), EFileError::eCreateError, dwLastError);
			switch (frResult)
			{
			case EFeedbackResult::eResult_Skip:
				break;	// will return INVALID_HANDLE_VALUE

			case EFeedbackResult::eResult_Cancel:
			{
				// log
				TString strFormat = _T("Cancel request [error %errno] while opening source file %path (OpenSourceFileFB)");
				strFormat.Replace(_T("%errno"), boost::lexical_cast<std::wstring>(dwLastError).c_str());
				strFormat.Replace(_T("%path"), fileSrc->GetFilePath().ToString());
				m_rLog.loge(strFormat.c_str());

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
				m_rLog.loge(strFormat.c_str());

				bRetry = true;
				break;
			}

			default:
				BOOST_ASSERT(FALSE);		// unknown result
				THROW_CORE_EXCEPTION(eErr_UnhandledCase);
			}
		} while (bRetry);

		return TSubTaskBase::eSubResult_Continue;
	}

	TSubTaskBase::ESubOperationResult TFilesystemFileFeedbackWrapper::OpenExistingDestinationFileFB(const IFeedbackHandlerPtr& spFeedbackHandler, const IFilesystemFilePtr& fileDst)
	{
		bool bRetry = false;

		fileDst->Close();

		do
		{
			bRetry = false;

			DWORD dwLastError = ERROR_SUCCESS;
			try
			{
				fileDst->OpenExistingForWriting();
				return TSubTaskBase::eSubResult_Continue;
			}
			catch (const TFileException& e)
			{
				dwLastError = e.GetNativeError();
			}

			EFeedbackResult frResult = spFeedbackHandler->FileError(fileDst->GetFilePath().ToWString(), TString(), EFileError::eCreateError, dwLastError);
			switch (frResult)
			{
			case EFeedbackResult::eResult_Retry:
			{
				// log
				TString strFormat = _T("Retrying [error %errno] to open destination file %path (CustomCopyFileFB)");
				strFormat.Replace(_T("%errno"), boost::lexical_cast<std::wstring>(dwLastError).c_str());
				strFormat.Replace(_t("%path"), fileDst->GetFilePath().ToString());
				m_rLog.loge(strFormat.c_str());

				bRetry = true;

				break;
			}
			case EFeedbackResult::eResult_Cancel:
			{
				// log
				TString strFormat = _T("Cancel request [error %errno] while opening destination file %path (CustomCopyFileFB)");
				strFormat.Replace(_T("%errno"), boost::lexical_cast<std::wstring>(dwLastError).c_str());
				strFormat.Replace(_T("%path"), fileDst->GetFilePath().ToString());
				m_rLog.loge(strFormat.c_str());

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
		} while (bRetry);

		return TSubTaskBase::eSubResult_Continue;
	}

	TSubTaskBase::ESubOperationResult TFilesystemFileFeedbackWrapper::TruncateFileFB(const IFeedbackHandlerPtr& spFeedbackHandler, const IFilesystemFilePtr& spFile, file_size_t fsNewSize, const TSmartPath& pathFile, bool& bSkip)
	{
		bSkip = false;

		bool bRetry = false;
		do
		{
			DWORD dwLastError = ERROR_SUCCESS;

			try
			{
				spFile->Truncate(fsNewSize);
				return TSubTaskBase::eSubResult_Continue;
			}
			catch (const TFileException& e)
			{
				dwLastError = e.GetNativeError();
			}

			TString strFormat = _T("Error %errno while truncating file %path to 0");
			strFormat.Replace(_t("%errno"), boost::lexical_cast<std::wstring>(dwLastError).c_str());
			strFormat.Replace(_t("%path"), pathFile.ToString());
			m_rLog.loge(strFormat.c_str());

			EFeedbackResult frResult = spFeedbackHandler->FileError(pathFile.ToWString(), TString(), EFileError::eResizeError, dwLastError);
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
		} while (bRetry);

		return TSubTaskBase::eSubResult_Continue;
	}

	TSubTaskBase::ESubOperationResult TFilesystemFileFeedbackWrapper::ReadFileFB(const IFeedbackHandlerPtr& spFeedbackHandler, const IFilesystemFilePtr& spFile, TOverlappedDataBuffer& rBuffer, const TSmartPath& pathFile, bool& bSkip)
	{
		bSkip = false;
		bool bRetry = false;
		do
		{
			bRetry = false;

			DWORD dwLastError = ERROR_SUCCESS;

			try
			{
				spFile->ReadFile(rBuffer);
				return TSubTaskBase::eSubResult_Continue;
			}
			catch (const TFileException& e)
			{
				dwLastError = e.GetNativeError();
			}

			TString strFormat = _T("Error %errno while requesting read of %count bytes from source file %path (CustomCopyFileFB)");
			strFormat.Replace(_t("%errno"), boost::lexical_cast<std::wstring>(dwLastError).c_str());
			strFormat.Replace(_t("%count"), boost::lexical_cast<std::wstring>(rBuffer.GetRequestedDataSize()).c_str());
			strFormat.Replace(_t("%path"), pathFile.ToString());
			m_rLog.loge(strFormat.c_str());

			EFeedbackResult frResult = spFeedbackHandler->FileError(pathFile.ToWString(), TString(), EFileError::eReadError, dwLastError);
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
		} while (bRetry);

		return TSubTaskBase::eSubResult_Continue;
	}

	TSubTaskBase::ESubOperationResult TFilesystemFileFeedbackWrapper::WriteFileFB(const IFeedbackHandlerPtr& spFeedbackHandler, const IFilesystemFilePtr& spFile, TOverlappedDataBuffer& rBuffer, const TSmartPath& pathFile, bool& bSkip)
	{
		bSkip = false;

		bool bRetry = false;
		do
		{
			bRetry = false;

			DWORD dwLastError = ERROR_SUCCESS;

			try
			{
				spFile->WriteFile(rBuffer);
				return TSubTaskBase::eSubResult_Continue;
			}
			catch (const TFileException& e)
			{
				dwLastError = e.GetNativeError();
			}

			TString strFormat = _T("Error %errno while trying to write %count bytes to destination file %path (CustomCopyFileFB)");
			strFormat.Replace(_t("%errno"), boost::lexical_cast<std::wstring>(dwLastError).c_str());
			strFormat.Replace(_t("%count"), boost::lexical_cast<std::wstring>(rBuffer.GetBytesTransferred()).c_str());
			strFormat.Replace(_t("%path"), pathFile.ToString());
			m_rLog.loge(strFormat.c_str());

			EFeedbackResult frResult = spFeedbackHandler->FileError(pathFile.ToWString(), TString(), EFileError::eWriteError, dwLastError);
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
		} while (bRetry);

		return TSubTaskBase::eSubResult_Continue;
	}

	TSubTaskBase::ESubOperationResult TFilesystemFileFeedbackWrapper::FinalizeFileFB(const IFeedbackHandlerPtr& spFeedbackHandler, const IFilesystemFilePtr& spFile, TOverlappedDataBuffer& rBuffer, const TSmartPath& pathFile, bool& bSkip)
	{
		bSkip = false;

		bool bRetry = false;
		do
		{
			bRetry = false;

			DWORD dwLastError = ERROR_SUCCESS;

			try
			{
				spFile->FinalizeFile(rBuffer);
				return TSubTaskBase::eSubResult_Continue;
			}
			catch (const TFileException& e)
			{
				dwLastError = e.GetNativeError();
			}

			TString strFormat = _T("Error %errno while trying to finalize file %path (CustomCopyFileFB)");
			strFormat.Replace(_t("%errno"), boost::lexical_cast<std::wstring>(dwLastError).c_str());
			strFormat.Replace(_t("%path"), pathFile.ToString());
			m_rLog.loge(strFormat.c_str());

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
		} while (bRetry);

		return TSubTaskBase::eSubResult_Continue;
	}

}
