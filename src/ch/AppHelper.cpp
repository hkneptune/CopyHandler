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
	m_hMutex=nullptr;
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
	if(pszName != nullptr)
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
	m_hMutex=CreateMutex(nullptr, TRUE, CH_MUTEX_NAME);
	m_bFirstInstance=(m_hMutex != nullptr && GetLastError() != ERROR_ALREADY_EXISTS);
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
		if(!CreateDirectory(rStrPath, nullptr) && GetLastError() != ERROR_ALREADY_EXISTS)
			return false;
	}

	// create directory for tasks
	if(!CreateDirectory(rStrPath + _T("\\Tasks"), nullptr) && GetLastError() != ERROR_ALREADY_EXISTS)
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

CString CAppHelper::GetFullProgramPath() const
{
	CString strKey;
	strKey.Format(_T("%s\\%s"), (PCTSTR)m_pathProcessor.GetProgramPath(), m_pszProgramName);

	return strKey;
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
