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
		return true;

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
