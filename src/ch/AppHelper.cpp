/***************************************************************************
*   Copyright (C) 2001-2008 by Józef Starosczyk                           *
*   ixen@copyhandler.com                                                  *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU Library General Public License          *
*   (version 2) as published by the Free Software Foundation;             *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU Library General Public     *
*   License along with this program; if not, write to the                 *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/
#include "stdafx.h"
#include "AppHelper.h"
#include "shlobj.h"
#include "../common/version.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define CH_MUTEX_NAME _T("_Copy handler_ instance")

CAppHelper::CAppHelper()
{
	// read program paths
	RetrievePaths();

	// retrieve VERSION-based info
	RetrieveAppInfo();

	// single-instance protection
	m_bFirstInstance=true;
	m_hMutex=NULL;
}

CAppHelper::~CAppHelper()
{
	if (m_hMutex)
		ReleaseMutex(m_hMutex);

	delete [] m_pszProgramName;
}

void CAppHelper::RetrievePaths()
{
	// try to find '\\' in path to see if this is only exe name or fully qualified path
	TCHAR* pszArgv = __wargv[ 0 ];

	TCHAR* pszName = _tcsrchr(pszArgv, _T('\\'));
	if(pszName != NULL)
	{
		// copy name
		m_pszProgramName = new TCHAR[ _tcslen(pszName + 1) + 1 ];
		_tcscpy(m_pszProgramName, pszName + 1);
	}
	else
	{
		// copy name
		m_pszProgramName = new TCHAR[ _tcslen(pszArgv) + 1 ];
		_tcscpy(m_pszProgramName, pszArgv);
	}
}

// inits mutex app protection
void CAppHelper::InitProtection()
{
	m_hMutex=CreateMutex(NULL, TRUE, CH_MUTEX_NAME);
	m_bFirstInstance=(m_hMutex != NULL && GetLastError() != ERROR_ALREADY_EXISTS);
}

void CAppHelper::RetrieveAppInfo()
{
	m_pszAppName = _T(PRODUCT_NAME);
	m_pszAppNameVer = PRODUCT_FULL_VERSION_T;
	m_pszAppVersion = _T(PRODUCT_VERSION);
}

bool CAppHelper::GetProgramDataPath(CString& rStrPath)
{
	if(IsInPortableMode())
		rStrPath = m_pathProcessor.GetProgramPath();
	else
	{
		rStrPath = m_pathProcessor.GetAppDataPath();
		rStrPath += L"\\Copy Handler";

		// make sure to create the required directories if they does not exist
		if(!CreateDirectory(rStrPath, NULL) && GetLastError() != ERROR_ALREADY_EXISTS)
			return false;
	}

	// create directory for tasks
	if(!CreateDirectory(rStrPath + _T("\\Tasks"), NULL) && GetLastError() != ERROR_ALREADY_EXISTS)
		return false;

	return true;
}

CString CAppHelper::ExpandPath(CString strPath)
{
	return m_pathProcessor.ExpandPath(strPath);
}

CString CAppHelper::GetProgramPath() const
{
	return m_pathProcessor.GetProgramPath();
}

bool CAppHelper::IsInPortableMode()
{
	if(!m_optPortableMode.is_initialized())
	{
		// check if the ch.ini exists in the program's directory - it is the only way we can determine portable mode
		CString strPortableCfgPath = CString(m_pathProcessor.GetProgramPath()) + _T("\\ch.xml");
		if(GetFileAttributes(strPortableCfgPath) == INVALID_FILE_ATTRIBUTES)
			m_optPortableMode = false;
		else
			m_optPortableMode = true;
	}

	return m_optPortableMode.get();
}

bool CAppHelper::SetAutorun(bool bEnable)
{
	// check the current key value (to avoid irritating messages from some firewall software)
	HKEY hkeyRun = NULL;
	CString strValue;
	CString strKey;
	DWORD dwType = REG_SZ;
	DWORD dwCount = _MAX_PATH * sizeof(TCHAR);

	LSTATUS lStatus = RegOpenKeyEx(HKEY_CURRENT_USER, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"), 0, KEY_QUERY_VALUE, &hkeyRun);
	if(lStatus != ERROR_SUCCESS)
		return false;

	lStatus = RegQueryValueEx(hkeyRun, m_pszAppName, NULL, &dwType, (BYTE*)strValue.GetBufferSetLength(_MAX_PATH), &dwCount);
	RegCloseKey(hkeyRun);

	if(lStatus != ERROR_SUCCESS && lStatus != ERROR_FILE_NOT_FOUND)
	{
		strValue.ReleaseBuffer(0);
		return false;
	}
	if(lStatus == ERROR_FILE_NOT_FOUND)
	{
		strValue.ReleaseBuffer(0);

		// if there is no key in registry and we don't want it, then return with ok status
		if(!bEnable)
			return true;

		// format the data to be written to registry
		strKey.Format(_T("%s\\%s"), (PCTSTR)m_pathProcessor.GetProgramPath(), m_pszProgramName);
	}
	else
	{
		// key found
		strValue.ReleaseBuffer(dwCount / sizeof(TCHAR));

		if(bEnable)
		{
			// key exists in registry, check if the value is correct
			strKey.Format(_T("%s\\%s"), (PCTSTR)m_pathProcessor.GetProgramPath(), m_pszProgramName);

			if(strValue.CompareNoCase(strKey) == 0)
				return true;
		}
	}

	// we want to write information to the registry
	// storing key in registry
	lStatus = RegOpenKeyEx(HKEY_CURRENT_USER, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"), 0, KEY_ALL_ACCESS, &hkeyRun);
	if(lStatus != ERROR_SUCCESS)
		return false;
	
	if(bEnable)
		lStatus = RegSetValueEx(hkeyRun, m_pszAppName, 0, REG_SZ, (BYTE*)(PCTSTR)strKey, (DWORD)(strKey.GetLength() + 1) * sizeof(TCHAR));
	else
		lStatus = RegDeleteValue(hkeyRun, m_pszAppName);
	
	RegCloseKey(hkeyRun);

	return lStatus == ERROR_SUCCESS;
}
