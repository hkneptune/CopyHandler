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
#ifndef __ASYNCHTTPFILE_H__
#define __ASYNCHTTPFILE_H__

class CAsyncHttpFile;

namespace details
{
	struct CONTEXT_REQUEST
	{
		enum EOperation
		{
			eNone,
			eInternetOpenUrl = 1,
			eInternetReadFileEx = 2,
		};

		CAsyncHttpFile* pHttpFile;
		EOperation eOperationType;
	};
}

class CAsyncHttpFile
{
public:
	enum EWaitResult
	{
		eKilled,
		eFinished,
		eTimeout,
		ePending,
		eError
	};

public:
	CAsyncHttpFile();
	~CAsyncHttpFile();

	HRESULT Open(const wchar_t* pszPath, const wchar_t* pszUserAgent, const wchar_t* pszHeaders);
	HRESULT GetFileSize(size_t& stSize);

	HRESULT RequestData(void* pBuffer, size_t stSize);
	HRESULT GetRetrievedDataSize(size_t& stSize);

	HRESULT Close();

	EWaitResult GetResult();
	DWORD GetErrorCode()
	{
		return m_dwError;
	}

	EWaitResult WaitForResult(HANDLE hKillEvent);

	bool IsClosed() const
	{
		return m_hOpenUrl == NULL;
	}

protected:
	static void CALLBACK InternetStatusCallback(HINTERNET hInternet, DWORD_PTR dwContext, DWORD dwInternetStatus, LPVOID lpvStatusInformation, DWORD dwStatusInformationLength);

	void SetUrlHandle(HANDLE hOpenUrl);
	void SetErrorCode(DWORD dwError);

	/// Sets the completion event
	HRESULT SetCompletionStatus(DWORD dwCurrentState);

protected:
	HINTERNET m_hInternet;
	HINTERNET m_hOpenUrl;

	DWORD m_dwExpectedState;		///< State we are expecting
	HANDLE m_hFinishedEvent;

	INTERNET_BUFFERSA m_internetBuffers;
	details::CONTEXT_REQUEST m_tOpenRequest;
	details::CONTEXT_REQUEST m_tReadRequest;

	DWORD m_dwError;
};

#endif
