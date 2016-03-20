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
#include "AsyncHttpFile.h"

// timeout used with waiting for events (avoiding hangs)
#define FORCE_TIMEOUT 60000

using details::CONTEXT_REQUEST;

// ============================================================================
/// CAsyncHttpFile::CAsyncHttpFile
/// @date 2009/04/18
///
/// @brief     Constructs the CAsyncHttpFile object.
// ============================================================================
CAsyncHttpFile::CAsyncHttpFile() :
	m_hInternet(NULL),
	m_hOpenUrl(NULL),
	m_hFinishedEvent(NULL),
	m_dwError(ERROR_SUCCESS)
{
	memset(&m_internetBuffers, 0, sizeof(INTERNET_BUFFERS));

	m_tOpenRequest.pHttpFile = this;
	m_tOpenRequest.eOperationType = CONTEXT_REQUEST::eInternetOpenUrl;

	m_tReadRequest.pHttpFile = this;
	m_tReadRequest.eOperationType = CONTEXT_REQUEST::eInternetReadFileEx;
}

// ============================================================================
/// CAsyncHttpFile::~CAsyncHttpFile
/// @date 2009/04/18
///
/// @brief     Destructs the CASyncHttpFile object.
// ============================================================================
CAsyncHttpFile::~CAsyncHttpFile()
{
	Close();
}

// ============================================================================
/// CAsyncHttpFile::Open
/// @date 2009/04/18
///
/// @brief     Opens the specified internet address (starts those operations).
/// @param[in] pszPath		Url to be opened (full path to file).
/// @return    S_OK if opened, S_FALSE if wait for result is needed, E_* for errors.
// ============================================================================
HRESULT CAsyncHttpFile::Open(const wchar_t* pszPath, const wchar_t* pszUserAgent, const wchar_t* pszHeaders)
{
	if(!pszPath)
	{
		SetErrorCode(ERROR_INTERNAL_ERROR);
		return E_INVALIDARG;
	}

	if(m_hInternet || m_hFinishedEvent)
	{
		SetErrorCode(ERROR_INTERNAL_ERROR);
		return E_FAIL;
	}

	// reset error code
	SetErrorCode(ERROR_SUCCESS);

	// create event
	m_hFinishedEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	if(!m_hFinishedEvent)
	{
		SetErrorCode(GetLastError());
		return E_FAIL;
	}

	m_hInternet = ::InternetOpen(pszUserAgent, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, INTERNET_FLAG_ASYNC);
	if(!m_hInternet)
	{
		DWORD dwError = GetLastError();
		ATLTRACE(L"InternetOpen failed with error: %lu\n", dwError);
		SetErrorCode(dwError);

		::CloseHandle(m_hFinishedEvent);
		m_hFinishedEvent = NULL;

		return E_FAIL;
	}

	if(::InternetSetStatusCallback(m_hInternet, (INTERNET_STATUS_CALLBACK)&CAsyncHttpFile::InternetStatusCallback) == INTERNET_INVALID_STATUS_CALLBACK)
	{
		DWORD dwError = GetLastError();
		ATLTRACE(L"InternetSetStatusCallback failed with error: %lu\n", dwError);
		SetErrorCode(dwError);

		::InternetCloseHandle(m_hInternet);
		::CloseHandle(m_hFinishedEvent);

		m_hFinishedEvent = NULL;
		return E_FAIL;
	}

	HINTERNET hOpenUrl = ::InternetOpenUrl(m_hInternet, pszPath, pszHeaders, (DWORD)-1, INTERNET_FLAG_NO_COOKIES | INTERNET_FLAG_NO_UI | INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_RELOAD, (DWORD_PTR)&m_tOpenRequest);
	if(!hOpenUrl)
	{
		DWORD dwError = GetLastError();
		ATLTRACE(L"InternetOpenUrl failed with error: %lu\n", dwError);

		SetErrorCode(dwError);
		if(GetErrorCode() != ERROR_IO_PENDING)
		{
			::InternetSetStatusCallback(m_hInternet, NULL);
			::InternetCloseHandle(m_hInternet);
			::CloseHandle(m_hFinishedEvent);

			m_hInternet = NULL;
			m_hFinishedEvent = NULL;

			return E_FAIL;
		}
	}
	else
	{
		::SetEvent(m_hFinishedEvent);
	}

	return hOpenUrl ? S_OK : S_FALSE;
}

// ============================================================================
/// CAsyncHttpFile::GetFileSize
/// @date 2009/04/18
///
/// @brief     Retrieves the size of file opened with CAsyncHttpFile::Open()
//				(if such information exists in http headers).
/// @param[out] stSize  Receives the size of file (receives 65536 if http headers
///                     did not contain the information).
/// @return		Result of the operation.
// ============================================================================
HRESULT CAsyncHttpFile::GetFileSize(size_t& stSize)
{
	if(!m_hInternet || !m_hOpenUrl)
	{
		SetErrorCode(ERROR_INTERNAL_ERROR);
		return E_FAIL;
	}

	DWORD dwContentLengthSize = sizeof(DWORD);
	if(!HttpQueryInfo(m_hOpenUrl, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, &stSize, &dwContentLengthSize, NULL) || stSize == 0 || stSize > 1 * 1024UL * 1024UL)
	{
		stSize = 65536;		// safe fallback
		return S_FALSE;
	}

	return S_OK;
}

// ============================================================================
/// CAsyncHttpFile::RequestData
/// @date 2009/04/18
///
/// @brief     Requests the data from already opened url.
/// @param[in]	pBuffer  Buffer for the data.
/// @param[in]  stSize   Buffer size.
/// @return	   S_OK if completed, S_FALSE if needs waiting, E_* on error.
// ============================================================================
HRESULT CAsyncHttpFile::RequestData(void* pBuffer, size_t stSize)
{
	if(!pBuffer)
	{
		SetErrorCode(ERROR_INTERNAL_ERROR);
		return E_INVALIDARG;
	}
	if(!m_hInternet || !m_hOpenUrl || !m_hFinishedEvent)
	{
		SetErrorCode(ERROR_INTERNAL_ERROR);
		return E_FAIL;
	}

	SetErrorCode(ERROR_SUCCESS);

	if(!::ResetEvent(m_hFinishedEvent))
	{
		SetErrorCode(GetLastError());
		return E_FAIL;
	}

	memset(&m_internetBuffers, 0, sizeof(INTERNET_BUFFERS));
	m_internetBuffers.dwStructSize = sizeof(INTERNET_BUFFERS);
	m_internetBuffers.dwBufferLength = (DWORD)stSize;
	m_internetBuffers.dwBufferTotal = (DWORD)stSize;
	m_internetBuffers.lpvBuffer = pBuffer;

	// #WinXP #workaround - in bare WinXP SP3 (i.e. without additional updates), InternetReadFileExW returns
	// error 120 (not implemented); it was implemented with some later update
	if(!::InternetReadFileExA(m_hOpenUrl, &m_internetBuffers, IRF_NO_WAIT, (DWORD_PTR)&m_tReadRequest))
	{
		DWORD dwError = GetLastError();
		ATLTRACE(L"InternetReadFileExA failed with error: %lu\n", dwError);

		SetErrorCode(dwError);
		if(GetErrorCode() == ERROR_IO_PENDING)
			return S_FALSE;
		else
			return E_FAIL;
	}

	if(!::SetEvent(m_hFinishedEvent))
	{
		SetErrorCode(GetLastError());
		return E_FAIL;
	}

	return S_OK;
}

// ============================================================================
/// CAsyncHttpFile::RetrieveRequestedData
/// @date 2009/04/18
///
/// @brief     Retrieves the size of data retrieved.
/// @param[out] stSize  Receives the size of data read from file.
/// @return    Result of the operation.
// ============================================================================
HRESULT CAsyncHttpFile::GetRetrievedDataSize(size_t& stSize)
{
	if(!m_hInternet)
	{
		SetErrorCode(ERROR_INTERNAL_ERROR);
		return E_FAIL;
	}

	stSize = m_internetBuffers.dwBufferLength;
	return S_OK;
}

// ============================================================================
/// CAsyncHttpFile::Close
/// @date 2009/04/18
///
/// @brief     Closes the file.
/// @return    Result of the operation.
// ============================================================================
HRESULT CAsyncHttpFile::Close()
{
	SetErrorCode(ERROR_SUCCESS);
	if(m_hOpenUrl)
	{
		if(!::InternetCloseHandle(m_hOpenUrl))
		{
			DWORD dwError = GetLastError();
			ATLTRACE(L"InternetCloseHandle failed with error: %lu\n", dwError);

			SetErrorCode(dwError);
			if(GetErrorCode() == ERROR_IO_PENDING)
				return S_FALSE;
			else
				return E_FAIL;
		}

		SetUrlHandle(NULL);
	}

	if(m_hInternet != nullptr)
	{
		::InternetCloseHandle(m_hInternet);
		m_hInternet = nullptr;
	}

	if(m_hFinishedEvent)
	{
		::CloseHandle(m_hFinishedEvent);
		m_hFinishedEvent = nullptr;
	}

	return S_OK;
}

// ============================================================================
/// CAsyncHttpFile::GetResult
/// @date 2009/04/18
///
/// @brief     Retrieves the last call result (blocking call).
/// @return    Result of the last call.
// ============================================================================
CAsyncHttpFile::EWaitResult CAsyncHttpFile::GetResult()
{
	HANDLE hHandles[] = { m_hFinishedEvent };
	DWORD dwEffect = WaitForMultipleObjects(1, hHandles, FALSE, 0);
	if(dwEffect == WAIT_OBJECT_0 + 0 || dwEffect == WAIT_ABANDONED_0 + 0)
		return GetErrorCode() == ERROR_SUCCESS ? CAsyncHttpFile::eFinished : CAsyncHttpFile::eError;
	else
		return CAsyncHttpFile::ePending;
}

// ============================================================================
/// CAsyncHttpFile::WaitForResult
/// @date 2009/04/18
///
/// @brief     Waits for the result with additional 'kill' event.
/// @param[in] hKillEvent  Event handle that would break waiting for result.
/// @return    Result of waiting.
// ============================================================================
CAsyncHttpFile::EWaitResult CAsyncHttpFile::WaitForResult(HANDLE hKillEvent)
{
	HANDLE hHandles[] = { hKillEvent, m_hFinishedEvent };
	DWORD dwEffect = WaitForMultipleObjects(2, hHandles, FALSE, FORCE_TIMEOUT);
	if(dwEffect == 0xffffffff)
	{
		DWORD dwError = GetLastError();
		ATLTRACE(L"WaitForMultipleObjects failed with error: %lu\n", dwError);

		SetErrorCode(dwError);
		return CAsyncHttpFile::eError;
	}
	else if(dwEffect == WAIT_OBJECT_0 + 0 || dwEffect == WAIT_ABANDONED_0 + 0)
		return CAsyncHttpFile::eKilled;
	else if(dwEffect == WAIT_OBJECT_0 + 1 || dwEffect == WAIT_ABANDONED_0 + 1)
		return GetErrorCode() == ERROR_SUCCESS ? CAsyncHttpFile::eFinished : CAsyncHttpFile::eError;
	else
		return CAsyncHttpFile::eTimeout;
}

// ============================================================================
/// CAsyncHttpFile::InternetStatusCallback
/// @date 2009/04/18
///
/// @brief     Callback for use with internet API.
/// @param[in] hInternet				Internet handle.
/// @param[in] dwContext				Context value.
/// @param[in] dwInternetStatus			Internet status.
/// @param[in] lpvStatusInformation		Additional status information.
/// @param[in] dwStatusInformationLength Length of lpvStatusInformation.
// ============================================================================
void CALLBACK CAsyncHttpFile::InternetStatusCallback(HINTERNET hInternet, DWORD_PTR dwContext, DWORD dwInternetStatus, LPVOID lpvStatusInformation, DWORD dwStatusInformationLength)
{
	CONTEXT_REQUEST* pRequest = (CONTEXT_REQUEST*)dwContext;
	BOOST_ASSERT(pRequest && pRequest->pHttpFile);
	if(!pRequest || !pRequest->pHttpFile)
		return;

	CString strMsg;
	strMsg.Format(_T("[CAsyncHttpFile::InternetStatusCallback] hInternet: %p, dwContext: %Iu (operation: %lu), dwInternetStatus: %lu, lpvStatusInformation: %p, dwStatusInformationLength: %lu\n"),
		hInternet, (size_t)dwContext, pRequest ? pRequest->eOperationType : CONTEXT_REQUEST::eNone, dwInternetStatus, lpvStatusInformation, dwStatusInformationLength);
	ATLTRACE(L"%s\n", strMsg);
	LOG_DEBUG(strMsg);

	switch(dwInternetStatus)
	{
	case INTERNET_STATUS_HANDLE_CREATED:
	{
		INTERNET_ASYNC_RESULT* pRes = (INTERNET_ASYNC_RESULT*)lpvStatusInformation;
		ATLTRACE(L"INTERNET_STATUS_HANDLE_CREATED error code: %lu\n", pRes->dwError);

		pRequest->pHttpFile->SetUrlHandle((HINTERNET)(pRes->dwResult));
		break;
	}
	case INTERNET_STATUS_RESPONSE_RECEIVED:
	{
		ATLTRACE(_T("INTERNET_STATUS_RESPONSE_RECEIVED; received %lu bytes."), *(DWORD*)lpvStatusInformation);
		break;
	}
	case INTERNET_STATUS_REQUEST_COMPLETE:
	{
		INTERNET_ASYNC_RESULT* pResult = (INTERNET_ASYNC_RESULT*)lpvStatusInformation;
		ATLTRACE(L"INTERNET_STATUS_REQUEST_COMPLETE error code: %lu\n", pResult->dwError);
		pRequest->pHttpFile->SetErrorCode(pResult->dwError);
		break;
	}
	case INTERNET_STATUS_CLOSING_CONNECTION:
	{
		pRequest->pHttpFile->SetUrlHandle(NULL);
		break;
	}
	case INTERNET_STATUS_CONNECTION_CLOSED:
	{
		break;
	}

	default:
		TRACE(_T("[CAsyncHttpFile::InternetStatusCallback()] Unhandled status: %lu\n"), dwInternetStatus);
	}

	pRequest->pHttpFile->SetCompletionStatus(dwInternetStatus);
}


// ============================================================================
/// CAsyncHttpFile::SetUrlHandle
/// @date 2009/04/18
///
/// @brief     Sets the url handle.
/// @param[in] hOpenUrl  Handle to be set.
// ============================================================================
void CAsyncHttpFile::SetUrlHandle(HANDLE hOpenUrl)
{
	m_hOpenUrl = hOpenUrl;
}

// ============================================================================
/// CAsyncHttpFile::SetErrorCode
/// @date 2009/04/18
///
/// @brief     Sets the error code.
/// @param[in] dwError  Error code to be set.
// ============================================================================
void CAsyncHttpFile::SetErrorCode(DWORD dwError)
{
	ATLTRACE(L"Setting error code : %lu\n", dwError);

	m_dwError = dwError;
}

// ============================================================================
/// CAsyncHttpFile::SetCompletionStatus
/// @date 2009/04/18
///
/// @brief     Sets the completion status.
/// @param[in] dwCurrentState  State to be set.
/// @return    Result of the operation.
// ============================================================================
HRESULT CAsyncHttpFile::SetCompletionStatus(DWORD dwCurrentState)
{
	if(!m_hFinishedEvent)
		return E_FAIL;

	if(dwCurrentState == INTERNET_STATUS_REQUEST_COMPLETE)
		return ::SetEvent(m_hFinishedEvent) ? S_OK : E_FAIL;
	return S_FALSE;
}
