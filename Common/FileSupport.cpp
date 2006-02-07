/************************************************************************
	Copy Handler 1.x - program for copying data in Microsoft Windows
						 systems.
	Copyright (C) 2001-2004 Ixen Gerthannes (copyhandler@o2.pl)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*************************************************************************/
#include "stdafx.h"
#include "wtypes.h"
#include "FileSupport.h"
//#include "tchar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma warning (disable: 4711) 

__int64 SetFilePointer64(HANDLE hFile, __int64 llDistance, DWORD dwMoveMethod)
{
   LARGE_INTEGER li;

   li.QuadPart = llDistance;

   li.LowPart = SetFilePointer(hFile, li.LowPart, &li.HighPart, dwMoveMethod);

   if (li.LowPart == 0xFFFFFFFF && GetLastError() != NO_ERROR)
      li.QuadPart = -1;

   return li.QuadPart;
}

__int64 GetFilePointer64(HANDLE hFile)
{
	return SetFilePointer64(hFile, 0, FILE_CURRENT);
}

__int64 GetFileSize64(HANDLE hFile)
{
	ULARGE_INTEGER li;

	li.LowPart = GetFileSize(hFile, &li.HighPart); 
 
	// If we failed ... 
	if (li.LowPart == 0xFFFFFFFF && GetLastError() != NO_ERROR)
		li.QuadPart=static_cast<unsigned __int64>(-1);

	return li.QuadPart;
}

bool SetFileSize64(LPCTSTR lpszFilename, __int64 llSize)
{
	HANDLE hFile=CreateFile(lpszFilename, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return false;

	if (SetFilePointer64(hFile, llSize, FILE_BEGIN) == -1)
	{
		CloseHandle(hFile);
		return false;
	}

	if (!SetEndOfFile(hFile))
	{
		CloseHandle(hFile);
		return false;
	}

	if (!CloseHandle(hFile))
		return false;
	
	return true;
}

// disk support routines

bool GetDynamicFreeSpace(LPCTSTR lpszPath, __int64* pFree, __int64* pTotal)
{
	typedef BOOL(__stdcall *PGETDISKFREESPACEEX)(LPCTSTR lpDirectoryName, PULARGE_INTEGER lpFreeBytesAvailable, PULARGE_INTEGER lpTotalNumberOfBytes, PULARGE_INTEGER lpTotalNumberOfFreeBytes);

	ULARGE_INTEGER ui64Available, ui64Total;
	PGETDISKFREESPACEEX pGetDiskFreeSpaceEx;
	pGetDiskFreeSpaceEx = (PGETDISKFREESPACEEX)GetProcAddress(GetModuleHandle(_T("kernel32.dll")), _T("GetDiskFreeSpaceExA"));
	if (pGetDiskFreeSpaceEx)
	{
		if (!pGetDiskFreeSpaceEx(lpszPath, &ui64Available, &ui64Total, NULL))
		{
			if (pFree)
				*pFree=-1;
			if (pTotal)
				*pTotal=-1;
			return false;
		}
		else
		{
			if (pFree)
				*pFree=ui64Available.QuadPart;
			if (pTotal)
				*pTotal=ui64Total.QuadPart;
			return true;
		}
	}
	else 
	{
		// support for win95 (not osr2)
		// set the root for path and correct '\\' at the end
		TCHAR szDisk[_MAX_DRIVE];
		_tsplitpath(lpszPath, szDisk, NULL, NULL, NULL);
		if (_tcslen(szDisk) != 0 && szDisk[_tcslen(szDisk)-1] != _T('\\'))
			_tcscat(szDisk, _T("\\"));

		// std func
		DWORD dwSectPerClust, dwBytesPerSect, dwFreeClusters, dwTotalClusters;
		if (!GetDiskFreeSpace(szDisk, &dwSectPerClust, &dwBytesPerSect, &dwFreeClusters, &dwTotalClusters))
		{
			if (pFree)
				*pFree=-1;
			if (pTotal)
				*pTotal=-1;
			return false;
		}
		else
		{
			if (pFree)
				*pFree=((__int64)dwFreeClusters*(__int64)dwSectPerClust)*(__int64)dwBytesPerSect;
			if (pTotal)
				*pTotal=((__int64)dwTotalClusters*(__int64)dwSectPerClust)*(__int64)dwBytesPerSect;
			return true;
		}
	}
}
