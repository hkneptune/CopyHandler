// ============================================================================
//  Copyright (C) 2001-2010 by Jozef Starosczyk
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
/// @file  TLocalFilesystem.cpp
/// @date  2011/03/24
/// @brief 
// ============================================================================
#include "stdafx.h"
#include "TLocalFilesystem.h"
#include "TAutoHandles.h"
#include "FileInfo.h"

void TLocalFilesystem::GetDriveData(const chcore::TSmartPath& spPath, int* piDrvNum, UINT* puiDrvType)
{
	TCHAR drv[_MAX_DRIVE + 1];

	_tsplitpath(spPath.ToString(), drv, NULL, NULL, NULL);
	if(lstrlen(drv) != 0)
	{
		// add '\\'
		lstrcat(drv, _T("\\"));
		_tcsupr(drv);

		// disk number
		if(piDrvNum)
			*piDrvNum=drv[0]-_T('A');

		// disk type
		if(puiDrvType)
		{
			*puiDrvType=GetDriveType(drv);
			if(*puiDrvType == DRIVE_NO_ROOT_DIR)
				*puiDrvType=DRIVE_UNKNOWN;
		}
	}
	else
	{
		// there's no disk in a path
		if(piDrvNum)
			*piDrvNum=-1;

		if(puiDrvType)
		{
			// check for unc path
			if(_tcsncmp(spPath.ToString(), _T("\\\\"), 2) == 0)
				*puiDrvType=DRIVE_REMOTE;
			else
				*puiDrvType=DRIVE_UNKNOWN;
		}
	}
}

bool TLocalFilesystem::PathExist(chcore::TSmartPath pathToCheck)
{
	WIN32_FIND_DATA fd;

	// search by exact name
	HANDLE hFind = FindFirstFile(pathToCheck.ToString(), &fd);
	if(hFind != INVALID_HANDLE_VALUE)
	{
		FindClose(hFind);
		return true;
	}

	// another try (add '\\' if needed and '*' for marking that we look for ie. c:\*
	// instead of c:\, which would never be found prev. way)
	pathToCheck.AppendIfNotExists(_T("*"), false);

	hFind = FindFirstFile(pathToCheck.ToString(), &fd);
	if(hFind != INVALID_HANDLE_VALUE)
	{
		::FindClose(hFind);
		return true;
	}
	else
		return false;
}

bool TLocalFilesystem::SetFileDirectoryTime(LPCTSTR lpszName, const FILETIME& ftCreationTime, const FILETIME& ftLastAccessTime, const FILETIME& ftLastWriteTime)
{
	TAutoFileHandle hFile = CreateFile(lpszName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS, NULL);
	if(hFile == INVALID_HANDLE_VALUE)
		return false;

	BOOL bResult = (!SetFileTime(hFile, &ftCreationTime, &ftLastAccessTime, &ftLastWriteTime));

	if(!hFile.Close())
		return false;

	return bResult != 0;
}

bool TLocalFilesystem::CreateDirectory(const chcore::TSmartPath& pathDirectory)
{
	if(pathDirectory.GetLength() > _MAX_PATH - 1)
	{
		std::wstring wstrPath = _T("\\\\?\\") + pathDirectory.ToWString();
		return ::CreateDirectory(wstrPath.c_str(), NULL) != FALSE;
	}
	else
		return ::CreateDirectory(pathDirectory.ToString(), NULL) != FALSE;
}

bool TLocalFilesystem::GetFileInfo(const chcore::TSmartPath& pathFile, CFileInfoPtr& rFileInfo, size_t stSrcIndex, const chcore::TPathContainer* pBasePaths)
{
	if(!rFileInfo)
		THROW(_T("Invalid argument"), 0, 0, 0);

	WIN32_FIND_DATA wfd;
	HANDLE hFind = INVALID_HANDLE_VALUE;

	if(pathFile.GetLength() > _MAX_PATH - 1)
	{
		std::wstring wstrPath = _T("\\\\?\\") + pathFile.ToWString();
		hFind = FindFirstFile(wstrPath.c_str(), &wfd);
	}
	else
		hFind = FindFirstFile(pathFile.ToString(), &wfd);

	if(hFind != INVALID_HANDLE_VALUE)
	{
		FindClose(hFind);

		// add data to members
		chcore::TSmartPath pathNew(pathFile);
		pathNew.DeleteFileName();

		// copy data from W32_F_D
		rFileInfo->Init(pathNew + chcore::PathFromString(wfd.cFileName), stSrcIndex, pBasePaths,
			wfd.dwFileAttributes, (((ULONGLONG) wfd.nFileSizeHigh) << 32) + wfd.nFileSizeLow, wfd.ftCreationTime,
			wfd.ftLastAccessTime, wfd.ftLastWriteTime, 0);

		return true;
	}
	else
	{
		FILETIME fi = { 0, 0 };
		rFileInfo->Init(chcore::TSmartPath(), std::numeric_limits<size_t>::max(), NULL, (DWORD)-1, 0, fi, fi, fi, 0);
		return false;
	}
}
