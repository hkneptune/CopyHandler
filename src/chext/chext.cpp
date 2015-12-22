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
// Note: Proxy/Stub Information
//      To build a separate proxy/stub DLL, 
//      run nmake -f CopyHandlerShellExtps.mk in the project directory.

#include "stdafx.h"
#include "resource.h"
#include "chext.h"
#include "dllmain.h"
#include "TLogger.h"
#include "GuidFormatter.h"

/////////////////////////////////////////////////////////////////////////////
// Used to determine whether the DLL can be unloaded by OLE

STDAPI DllCanUnloadNow()
{
	TLogger& rLogger = Logger::get();
	BOOST_LOG_SEV(rLogger, debug) << L"DllCanUnloadNow()";

	HRESULT hResult = _AtlModule.DllCanUnloadNow();

	BOOST_LOG_SEV(rLogger, debug) << L"DllCanUnloadNow: hResult = " << hResult;

	return hResult;
}

/////////////////////////////////////////////////////////////////////////////
// Returns a class factory to create an object of the requested type

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	TLogger& rLogger = Logger::get();
	BOOST_LOG_SEV(rLogger, debug) << L"DllGetClassObject()";

	HRESULT hResult = _AtlModule.DllGetClassObject(rclsid, riid, ppv);

	BOOST_LOG_SEV(rLogger, debug) << L"DllGetClassObject(clsid=" << GuidFormatter::FormatGuid(rclsid) << ", riid=" << GuidFormatter::FormatGuid(riid) << ", ppv=" << ppv << "): hResult=" << hResult;

	return hResult;
}

/////////////////////////////////////////////////////////////////////////////
// DllRegisterServer - Adds entries to the system registry

STDAPI DllRegisterServer()
{
	TLogger& rLogger = Logger::get();
	BOOST_LOG_SEV(rLogger, debug) << L"DllRegisterServer()";

	// registers object, typelib and all interfaces in typelib
	HRESULT hResult = _AtlModule.DllRegisterServer();

	BOOST_LOG_SEV(rLogger, debug) << L"DllRegisterServer(): hResult=" << hResult;

	return hResult;
}

/////////////////////////////////////////////////////////////////////////////
// DllUnregisterServer - Removes entries from the system registry

STDAPI DllUnregisterServer()
{
	TLogger& rLogger = Logger::get();
	BOOST_LOG_SEV(rLogger, debug) << L"DllUnregisterServer()";

	HRESULT hResult = _AtlModule.DllUnregisterServer();

	BOOST_LOG_SEV(rLogger, debug) << L"DllUnregisterServer(): hResult=" << hResult;

	return hResult;
}

// DllInstall - Adds/Removes entries to the system registry per user
//              per machine.	
STDAPI DllInstall(BOOL bInstall, LPCWSTR pszCmdLine)
{
	TLogger& rLogger = Logger::get();
	BOOST_LOG_SEV(rLogger, debug) << L"DllInstall()";

	HRESULT hr = E_FAIL;
	static const wchar_t szUserSwitch[] = _T("user");

	if (pszCmdLine != NULL)
	{
		if (_wcsnicmp(pszCmdLine, szUserSwitch, _countof(szUserSwitch)) == 0)
			AtlSetPerUserRegistration(true);
	}

	if (bInstall)
	{
		hr = DllRegisterServer();
		if (FAILED(hr))
			DllUnregisterServer();
	}
	else
		hr = DllUnregisterServer();

	BOOST_LOG_SEV(rLogger, debug) << L"DllInstall(bInstall=" << bInstall << ", pszCmdLine: " << pszCmdLine << L"): hResult=" << hr;

	return hr;
}
