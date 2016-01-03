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
/// @file UpdateChecker.h
/// @date 2009/04/18
/// @brief Contains declaration of update checker class.
// ============================================================================
#ifndef __UPDATECHECKER_H__
#define __UPDATECHECKER_H__

#include "WindowsVersion.h"
#include "AsyncHttpFile.h"
#include "UpdateHeaders.h"
#include "UpdateVersionInfo.h"

class CUpdateChecker : protected CInternetSession
{
public:
	enum ECheckResult
	{
		eResult_Undefined,
		eResult_Pending,
		eResult_Killed,
		eResult_Error,
		eResult_VersionCurrent,
		eResult_RemoteVersionNewer
	};

public:
	/// Constructs the update checker object
	CUpdateChecker();
	/// Destructs the update checker object
	~CUpdateChecker();

	/// Starts the 'check for updates' thread
	bool AsyncCheckForUpdates(const wchar_t* pszSite, const wchar_t* pszLanguage, UpdateVersionInfo::EVersionType bCheckBeta, bool bOnlyIfConnected);

	/// Stops checking and cleanups the object
	void Cleanup();

	/// Retrieves the update result
	ECheckResult GetResult() const;

	// methods for retrieving state
	const wchar_t* GetNumericVersion() const { return (const wchar_t*)m_strNumericVersion; }
	const wchar_t* GetReadableVersion() const { return (const wchar_t*)m_strReadableVersion; }
	const wchar_t* GetLastError() const { return (const wchar_t*)m_strLastError; }
	const wchar_t* GetDownloadAddress() const { return m_strDownloadAddress; }
	const wchar_t* GetReleaseDate() const { return m_strReleaseDate; }
	const wchar_t* GetReleaseNotes() const { return m_strReleaseNotes; }

protected:
	/// Thread function (handles most of the internet connection operation)
	static DWORD WINAPI UpdateCheckThread(LPVOID pParam);

	/// Sets the result in mt-safe way
	void SetResult(ECheckResult eCheckResult, DWORD dwError);
	/// Sets the last error
	void SetLastError(PCTSTR pszError);
	/// Sets the versions and download address
	void SetVersionsAndAddress(PCTSTR pszAddress, PCTSTR pszNumericVersion, PCTSTR pszReadableVersion, PCTSTR pszReleaseDate, PCTSTR pszReleaseNotes);
	/// Retrieves the site address
	void GetSiteAddress(CString& rstrAddress) const;

	/// Returns information if we're interested in beta versions
	UpdateVersionInfo::EVersionType GetUpdateChannel();
	static std::wstring FormatDate(const boost::gregorian::date& date);

protected:
	CString m_strSite;
	UpdateVersionInfo::EVersionType m_eUpdateChannel;
	CString m_strLanguage;
	CString m_strLastError;
	CString m_strNumericVersion;
	CString m_strReadableVersion;
	CString m_strDownloadAddress;
	CString m_strReleaseDate;
	CString m_strReleaseNotes;
	
	ECheckResult m_eResult;

	CAsyncHttpFile m_httpFile;
	UpdateHeaders m_tUpdateHeaders;


	HANDLE m_hThread;
	HANDLE m_hKillEvent;
	mutable CRITICAL_SECTION m_cs;
};

#endif
