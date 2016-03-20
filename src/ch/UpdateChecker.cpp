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
/// @file UpdateChecker.cpp
/// @date 2009/04/18
/// @brief Contains an implementation of update checker class.
// ============================================================================
#include "stdafx.h"
#include "UpdateChecker.h"
#include "UpdateResponse.h"
#include <afxinet.h>
#include "../libchcore/TWin32ErrorFormatter.h"
#include "../common/version.h"
#include <boost/date_time/gregorian/gregorian_io.hpp>

// ============================================================================
/// CUpdateChecker::CUpdateChecker
/// @date 2009/04/18
///
/// @brief     Constructs the update checker object.
// ============================================================================
CUpdateChecker::CUpdateChecker() :
	m_hThread(NULL),
	m_hKillEvent(NULL),
	m_eResult(eResult_Undefined),
	m_eUpdateChannel(UpdateVersionInfo::eStable)
{
	m_hKillEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	BOOST_ASSERT(m_hKillEvent);
	::InitializeCriticalSection(&m_cs);
}

// ============================================================================
/// CUpdateChecker::~CUpdateChecker
/// @date 2009/04/18
///
/// @brief     Destroys the update checker object.
// ============================================================================
CUpdateChecker::~CUpdateChecker()
{
	Cleanup();

	if(m_hKillEvent)
		::CloseHandle(m_hKillEvent);

	::DeleteCriticalSection(&m_cs);
}

// ============================================================================
/// CUpdateChecker::AsyncCheckForUpdates
/// @date 2009/04/18
///
/// @brief     Starts the asynchronous checking for updates.
/// @param[in] pszSite	    Site where to search for updates (without file name).
/// @param[in] bCheckBeta   States if we are interested in beta products.
/// @return    True if operation started, false otherwise.
// ============================================================================
bool CUpdateChecker::AsyncCheckForUpdates(const wchar_t* pszSite, const wchar_t* pszLanguage, UpdateVersionInfo::EVersionType eUpdateChannel, bool bOnlyIfConnected, bool bSendHeaders)
{
	if(!pszSite)
		return false;

	Cleanup();

	DWORD dwConnectionFlags = 0;

	if(bOnlyIfConnected && !InternetGetConnectedState(&dwConnectionFlags, 0))
		return false;

	m_strSite = pszSite;
	m_eResult = eResult_Undefined;
	m_eUpdateChannel = eUpdateChannel;
	m_strLanguage = pszLanguage;
	m_bSendHeaders = bSendHeaders;

	::ResetEvent(m_hKillEvent);

	m_hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&CUpdateChecker::UpdateCheckThread, (void*)this, 0, NULL);
	if(!m_hThread)
	{
		m_strLanguage.Empty();
		m_strSite.Empty();
		m_eResult = eResult_Undefined;
		m_eUpdateChannel = UpdateVersionInfo::eStable;
		m_bSendHeaders = true;
		return false;
	}

	return true;
}

// ============================================================================
/// CUpdateChecker::Cleanup
/// @date 2009/04/18
///
/// @brief     Stops scanning for updates and clears the object.
// ============================================================================
void CUpdateChecker::Cleanup()
{
	if(m_hThread)
	{
		if(m_hKillEvent)
			::SetEvent(m_hKillEvent);
		WaitForSingleObject(m_hThread, 5000);
		m_hThread = NULL;
	}

	m_httpFile.Close();

	::EnterCriticalSection(&m_cs);
	m_strSite.Empty();
	m_eUpdateChannel = UpdateVersionInfo::eStable;
	m_strLanguage.Empty();
	m_strLastError.Empty();
	m_strNumericVersion.Empty();
	m_strReadableVersion.Empty();
	m_strReleaseDate.Empty();
	m_strDownloadAddress.Empty();
	m_strReleaseNotes.Empty();
	m_eResult = CUpdateChecker::eResult_Undefined;
	::LeaveCriticalSection(&m_cs);
}

// ============================================================================
/// CUpdateChecker::SetResult
/// @date 2009/04/18
///
/// @brief     Sets the result of checking.
/// @param[in] eCheckResult  Result to be set.
/// @param[in] dwError       Error code (if any).
// ============================================================================
void CUpdateChecker::SetResult(ECheckResult eCheckResult, DWORD dwError)
{
	chcore::TString strError;
	if(eCheckResult == eResult_Error && dwError != 0)
		strError = chcore::TWin32ErrorFormatter::FormatWin32ErrorCodeWithFallback(dwError, _T("wininet.dll"), true);

	::EnterCriticalSection(&m_cs);
	
	m_eResult = eCheckResult;
	m_strLastError = strError.c_str();

	::LeaveCriticalSection(&m_cs);
}

// ============================================================================
/// CUpdateChecker::SetLastError
/// @date 2009/04/18
///
/// @brief     Sets last error.
/// @param[in] pszError  String containing the last error description.
// ============================================================================
void CUpdateChecker::SetLastError(PCTSTR pszError)
{
	::EnterCriticalSection(&m_cs);
	m_strLastError = pszError;
	::LeaveCriticalSection(&m_cs);
}

// ============================================================================
/// CUpdateChecker::SetVersionsAndAddress
/// @date 2009/04/18
///
/// @brief     Sets the download address and version information.
/// @param[in] pszAddress            Download address.
/// @param[in] pszNumericVersion     Numeric version number.
/// @param[in] pszReadableVersion    Human readable version number.
// ============================================================================
void CUpdateChecker::SetVersionsAndAddress(PCTSTR pszAddress, PCTSTR pszNumericVersion, PCTSTR pszReadableVersion, PCTSTR pszReleaseDate, PCTSTR pszReleaseNotes)
{
	::EnterCriticalSection(&m_cs);
	m_strDownloadAddress = pszAddress;
	m_strNumericVersion = pszNumericVersion;
	m_strReadableVersion = pszReadableVersion;
	m_strReleaseDate = pszReleaseDate;
	m_strReleaseNotes = pszReleaseNotes;
	::LeaveCriticalSection(&m_cs);
}

void CUpdateChecker::SetSendHeaders(bool bSendHeaders)
{
	::EnterCriticalSection(&m_cs);
	m_bSendHeaders = bSendHeaders;
	::LeaveCriticalSection(&m_cs);
}

// ============================================================================
/// CUpdateChecker::GetSiteAddress
/// @date 2009/04/18
///
/// @brief     Retrieves the address of a site to check the updates at.
/// @param[out] rstrAddress  Receives the address.
// ============================================================================
CString CUpdateChecker::GetSiteAddress() const
{
	::EnterCriticalSection(&m_cs);
	CString strAddress = m_strSite;
	::LeaveCriticalSection(&m_cs);

	return strAddress;
}

// ============================================================================
/// CUpdateChecker::CheckForBeta
/// @date 2009/04/18
///
/// @brief     Returns information, if update should check for beta versions.
/// @return    True if beta versions should be processed, false otherwise.
// ============================================================================
UpdateVersionInfo::EVersionType CUpdateChecker::GetUpdateChannel()
{
	::EnterCriticalSection(&m_cs);
	UpdateVersionInfo::EVersionType eUpdateChannel = m_eUpdateChannel;
	::LeaveCriticalSection(&m_cs);

	return eUpdateChannel;
}

// ============================================================================
/// CUpdateChecker::GetResult
/// @date 2009/04/18
///
/// @brief     Retrieves the result of checking for updates.
/// @return    Check for updates result.
// ============================================================================
CUpdateChecker::ECheckResult CUpdateChecker::GetResult() const
{
	::EnterCriticalSection(&m_cs);
	ECheckResult eResult = m_eResult;
	::LeaveCriticalSection(&m_cs);
	return eResult;
}

// ============================================================================
/// CUpdateChecker::UpdateCheckThread
/// @date 2009/04/18
///
/// @brief     Main thread function.
/// @param[in] pParam  Pointer to the thread parameter (pointer to the CUpdateChecker object).
/// @return    Thread execution status.
// ============================================================================
DWORD CUpdateChecker::UpdateCheckThread(LPVOID pParam)
{
	CUpdateChecker* pUpdateChecker = (CUpdateChecker*)pParam;

	try
	{
		// mark as started
		pUpdateChecker->SetResult(eResult_Pending, 0);

		// get the real address of file to download
		CString strSite = pUpdateChecker->GetSiteAddress();

		CAsyncHttpFile::EWaitResult eWaitResult = CAsyncHttpFile::ePending;
		size_t stFileSize = 0;
		std::stringstream dataBuffer;

		// open the connection and try to get to the file
		std::wstring wstrUserAgent = pUpdateChecker->m_tUpdateHeaders.GetUserAgent();
		std::wstring wstrHeaders;
		if(pUpdateChecker->GetSendHeaders())
			wstrHeaders = pUpdateChecker->m_tUpdateHeaders.GetHeaders((PCTSTR)pUpdateChecker->m_strLanguage, pUpdateChecker->m_eUpdateChannel);

		HRESULT hResult = pUpdateChecker->m_httpFile.Open(strSite, wstrUserAgent.c_str(), wstrHeaders.c_str());
		if(SUCCEEDED(hResult))
		{
			eWaitResult = pUpdateChecker->m_httpFile.WaitForResult(pUpdateChecker->m_hKillEvent);
			switch(eWaitResult)
			{
			case CAsyncHttpFile::eFinished:
				break;
			case CAsyncHttpFile::eKilled:
				pUpdateChecker->SetResult(eResult_Killed, 0);
				return 1;
			case CAsyncHttpFile::eError:
				pUpdateChecker->SetResult(eResult_Error, pUpdateChecker->m_httpFile.GetErrorCode());
				return 1;
			case CAsyncHttpFile::eTimeout:
			case CAsyncHttpFile::ePending:
			default:
				pUpdateChecker->SetResult(eResult_Error, 0);
				return 1;
			}

			// get the file size
			hResult = pUpdateChecker->m_httpFile.GetFileSize(stFileSize);
		}

		if(SUCCEEDED(hResult))
		{
			bool bIsClosed = false;
			char* pbyBuffer = new char[ stFileSize ];
			do
			{
				hResult = pUpdateChecker->m_httpFile.RequestData(pbyBuffer, stFileSize);
				if(SUCCEEDED(hResult))
				{
					eWaitResult = pUpdateChecker->m_httpFile.WaitForResult(pUpdateChecker->m_hKillEvent);
					switch(eWaitResult)
					{
					case CAsyncHttpFile::eFinished:
						break;
					case CAsyncHttpFile::eKilled:
						pUpdateChecker->SetResult(eResult_Killed, 0);
						return 1;
						break;
					case CAsyncHttpFile::eError:
						pUpdateChecker->SetResult(eResult_Error, pUpdateChecker->m_httpFile.GetErrorCode());
						return 1;
					case CAsyncHttpFile::eTimeout:
					case CAsyncHttpFile::ePending:
					default:
						pUpdateChecker->SetResult(eResult_Error, 0);
						return 1;
					}
				}

				if(SUCCEEDED(hResult))
					hResult = pUpdateChecker->m_httpFile.GetRetrievedDataSize(stFileSize);

				if(SUCCEEDED(hResult) && stFileSize)
					dataBuffer.write(pbyBuffer, stFileSize);

				bIsClosed = pUpdateChecker->m_httpFile.IsClosed();
			}
			while(stFileSize && !bIsClosed && SUCCEEDED(hResult));

			delete[] pbyBuffer;
		}

		if(FAILED(hResult))
		{
			pUpdateChecker->SetResult(eResult_Error, pUpdateChecker->m_httpFile.GetErrorCode());
			return 1;
		}

		pUpdateChecker->m_httpFile.Close();

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////
		UpdateVersionInfo::EVersionType eUpdateChannel = pUpdateChecker->GetUpdateChannel();

		UpdateResponse response(dataBuffer);
		UpdateVersionInfo vi;
		bool bHasUpdates = response.GetVersions().FindUpdateInfo(eUpdateChannel, vi);
		if(bHasUpdates)
		{
			pUpdateChecker->SetVersionsAndAddress(vi.GetDownloadLink().c_str(), vi.GetNumericVersion().c_str(), vi.GetReadableVersion().c_str(),
				FormatDate(vi.GetDateRelease()).c_str(), vi.GetReleaseNotes().c_str());
			pUpdateChecker->SetResult(eResult_RemoteVersionNewer, 0);
		}
		else
			pUpdateChecker->SetResult(eResult_VersionCurrent, 0);
	}
	catch(const std::exception&)
	{
		pUpdateChecker->SetResult(eResult_Error, 0);
	}

	return 0;
}

std::wstring CUpdateChecker::FormatDate(const boost::gregorian::date& date)
{
	using namespace boost::gregorian;

	std::wstringstream ss;
	wdate_facet * fac = new wdate_facet(L"%Y-%m-%d");
	ss.imbue(std::locale(std::locale::classic(), fac));

	ss << date;
	return ss.str();
}
