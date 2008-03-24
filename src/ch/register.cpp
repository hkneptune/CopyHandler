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

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

HRESULT RegisterShellExtDll(LPCTSTR lpszPath, bool bRegister)
{
	// first try - load dll and register it manually.
	HRESULT hResult = S_OK;
	// if failed - try by loading extension manually (would fail on vista when running as user)
	hResult = CoInitialize(NULL);
	if(SUCCEEDED(hResult))
	{
		HRESULT (STDAPICALLTYPE *pfn)(void);
		HINSTANCE hMod = LoadLibrary(lpszPath);	// load the dll
		if(hMod == NULL)
			hResult = HRESULT_FROM_WIN32(GetLastError());
		if(SUCCEEDED(hResult) && !hMod)
			hResult = E_FAIL;
		if(SUCCEEDED(hResult))
		{
			(FARPROC&)pfn = GetProcAddress(hMod, (bRegister ? "DllRegisterServer" : "DllUnregisterServer"));
			if(pfn == NULL)
				hResult = E_FAIL;
			if(SUCCEEDED(hResult))
				hResult = (*pfn)();

			CoFreeLibrary(hMod);
		}
		CoUninitialize();
	}

	// if previous operation failed (ie. vista system) - try running regsvr32 with elevated privileges
	if(SCODE_CODE(hResult) == ERROR_ACCESS_DENIED)
	{
		hResult = S_FALSE;
		// try with regsvr32
		SHELLEXECUTEINFO sei;
		memset(&sei, 0, sizeof(sei));
		sei.cbSize = sizeof(sei);
		sei.fMask = SEE_MASK_UNICODE;
		sei.lpVerb = _T("runas");
		sei.lpFile = _T("regsvr32.exe");
		CString strParams;
		if(bRegister)
			strParams = CString(_T(" \"")) + lpszPath + CString(_T("\""));
		else
			strParams = CString(_T("/u \"")) + lpszPath + CString(_T("\""));
		sei.lpParameters = strParams;
		sei.nShow = SW_SHOW;

		if(!ShellExecuteEx(&sei))
			hResult = E_FAIL;
	}

	return hResult;
}
