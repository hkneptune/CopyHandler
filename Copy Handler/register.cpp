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
#include "register.h"
#include "objbase.h"

DWORD RegisterShellExtDll(LPCTSTR lpszPath, bool bRegister)
{
	DWORD dwErr=0;
	CoInitialize(NULL);

	HINSTANCE hMod=LoadLibrary(lpszPath);	// load the dll
	if (hMod != NULL)
	{
		HRESULT (STDAPICALLTYPE *pfn)(void);
		
		(FARPROC&)pfn = GetProcAddress(hMod, (bRegister ? _T("DllRegisterServer") : _T("DllUnregisterServer")));
		if (pfn == NULL || (*pfn)() != S_OK)
		{
			dwErr=GetLastError();
			CoFreeLibrary(hMod);
			CoUninitialize();
			return dwErr;
		}
		else
		{
			CoFreeLibrary(hMod);
			
			// shut down the COM Library.
			CoUninitialize();
			return 0;
		}
	}
	else
	{
		dwErr=GetLastError();
		CoUninitialize();
		return dwErr;
	}
}
