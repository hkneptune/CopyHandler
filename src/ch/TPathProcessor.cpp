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
#include "TPathProcessor.h"

TPathProcessor::TPathProcessor()
{
	RetrievePaths();
}

CString TPathProcessor::ExpandPath(CString strPath)
{
	if(strPath[ 0 ] != _T('<'))
		return strPath;

	// search for string to replace
	// _T("<WINDOWS>"), _T("<TEMP>"), _T("<SYSTEM>"), _T("<APPDATA>"), _T("<DESKTOP>"), 
	// _T("<PERSONAL>"), _T("<PROGRAM>")
	if(StartsWith(strPath, _T("<PROGRAM>")))
		strPath.Replace(_T("<PROGRAM>"), m_strProgramPath);
	else if(StartsWith(strPath, _T("<WINDOWS>")))
		strPath.Replace(_T("<WINDOWS>"), GetWindowsPath());
	else if(StartsWith(strPath, _T("<TEMP>")))	// temp dir
		strPath.Replace(_T("<TEMP>"), GetTempPath());
	else if(StartsWith(strPath, _T("<SYSTEM>")))	// system
		strPath.Replace(_T("<SYSTEM>"), GetSystemPath());
	else if(StartsWith(strPath, _T("<APPDATA>")))	// app data
		strPath.Replace(_T("<APPDATA>"), GetFolderLocation(CSIDL_LOCAL_APPDATA));
	else if(StartsWith(strPath, _T("<DESKTOP>")))	// desktop
		strPath.Replace(_T("<DESKTOP>"), GetFolderLocation(CSIDL_DESKTOPDIRECTORY));
	else if(StartsWith(strPath, _T("<PERSONAL>")))	// personal...
		strPath.Replace(_T("<DESKTOP>"), GetFolderLocation(CSIDL_PERSONAL));

	return strPath;
}

CString TPathProcessor::GetProgramPath() const
{
	return m_strProgramPath;
}

CString TPathProcessor::GetAppDataPath() const
{
	return GetFolderLocation(CSIDL_LOCAL_APPDATA);
}

bool TPathProcessor::StartsWith(const CString& strWhere, const CString& strWhat)
{
	int iLen = strWhat.GetLength();
	if(iLen <= 0)
		return false;

	return (strWhere.Left(iLen) == strWhat);
}

CString TPathProcessor::GetWindowsPath()
{
	// get windows path
	wchar_t szData[ _MAX_PATH + 1 ];
	UINT uiSize = GetWindowsDirectory(szData, _MAX_PATH);
	if(uiSize == 0 || uiSize > _MAX_PATH)
		return CString();

	if(szData[ uiSize - 1 ] == _T('\\'))
		szData[ uiSize - 1 ] = _T('\0');

	return szData;
}

CString TPathProcessor::GetTempPath()
{
	// get windows path
	wchar_t szData[ _MAX_PATH + 1 ];
	UINT uiSize = ::GetTempPath(_MAX_PATH, szData);
	if(uiSize == 0 || uiSize > _MAX_PATH)
		return CString();

	if(szData[ uiSize - 1 ] == _T('\\'))
		szData[ uiSize - 1 ] = _T('\0');

	return szData;
}

CString TPathProcessor::GetSystemPath()
{
	// get windows path
	wchar_t szData[ _MAX_PATH + 1 ];
	UINT uiSize = GetSystemDirectory(szData, _MAX_PATH);
	if(uiSize == 0 || uiSize > _MAX_PATH)
		return CString();

	if(szData[ uiSize - 1 ] == _T('\\'))
		szData[ uiSize - 1 ] = _T('\0');

	return szData;
}

CString TPathProcessor::GetFolderLocation(int iFolder)
{
	LPITEMIDLIST piid = nullptr;
	HRESULT hResult = SHGetSpecialFolderLocation(NULL, iFolder, &piid);
	if(!SUCCEEDED(hResult))
		return CString();

	// get path
	wchar_t szData[ _MAX_PATH ];
	BOOL bRes = SHGetPathFromIDList(piid, szData);

	// free piid
	LPMALLOC lpm = nullptr;
	if(!SUCCEEDED(SHGetMalloc(&lpm)))
		return CString();

	lpm->Free((void*)piid);
	lpm->Release();

	// check for error
	if(!bRes)
		return CString();

	// strip the last '\\'
	CString strPath = szData;
	strPath.TrimRight(L'\\');

	return strPath;
}

void TPathProcessor::RetrievePaths()
{
	// try to find '\\' in path to see if this is only exe name or fully qualified path
	TCHAR* pszArgv = __wargv[ 0 ];

	CString strName = pszArgv;
	int iPos = strName.ReverseFind(_T('\\'));

	if(iPos != -1)
		m_strProgramPath = strName.Left(iPos + 1);
	else
	{
		// path
		TCHAR szPath[ _MAX_PATH ];
		UINT uiSize = GetCurrentDirectory(_MAX_PATH, szPath);
		if(uiSize == 0)
			m_strProgramPath.Empty();
		else
			m_strProgramPath = szPath;
	}

	m_strProgramPath.TrimRight(L'\\');
}
