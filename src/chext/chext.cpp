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
#include "Logger.h"
#include "guids.h"
#include "MenuExtClassFactory.h"
#include "DropMenuExtClassFactory.h"
#include "ShellExtControlClassFactory.h"
#include "../common/TRegistry.h"
#include "DllRegistration.h"

LONG g_DllRefCount = 0; // Reference count of this DLL.
extern HINSTANCE g_hInstance;

namespace
{
	template<class T>
	HRESULT CreateFactory(REFIID riid, LPVOID* ppv)
	{
		auto classFactory = new (std::nothrow) T;
		if (!classFactory)
			return E_OUTOFMEMORY;

		HRESULT hResult = classFactory->QueryInterface(riid, ppv);
		if (hResult != S_OK)
			delete classFactory;

		return hResult;
	}
}

/////////////////////////////////////////////////////////////////////////////
// Used to determine whether the DLL can be unloaded by OLE

STDAPI DllCanUnloadNow()
{
	return (g_DllRefCount == 0 ? S_OK : S_FALSE);
}

/////////////////////////////////////////////////////////////////////////////
// Returns a class factory to create an object of the requested type

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	if (!ppv)
		return E_POINTER;

	*ppv = nullptr;

	try
	{
		if (IsEqualIID(rclsid, CLSID_MenuExt))
			return CreateFactory<MenuExtClassFactory>(riid, ppv);
		else if (IsEqualIID(rclsid, CLSID_DropMenuExt))
			return CreateFactory<DropMenuExtClassFactory>(riid, ppv);
		else if (IsEqualIID(rclsid, CLSID_CShellExtControl))
			return CreateFactory<ShellExtControlClassFactory>(riid, ppv);
	}
	catch (const std::exception& e)
	{
		OutputDebugStringA(e.what());
		return E_FAIL;
	}

	return CLASS_E_CLASSNOTAVAILABLE;
}

/////////////////////////////////////////////////////////////////////////////
// DllRegisterServer - Adds entries to the system registry

STDAPI DllRegisterServer()
{
	try
	{
		DllRegistration regDll(g_hInstance);

		regDll.RegisterAll();
	}
	catch (const std::exception& e)
	{
		OutputDebugStringA(e.what());
		return E_FAIL;
	}

	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// DllUnregisterServer - Removes entries from the system registry

STDAPI DllUnregisterServer()
{
	try
	{
		DllRegistration regDll(g_hInstance);
		regDll.UnregisterAll();
	}
	catch (const std::exception& e)
	{
		OutputDebugStringA(e.what());
		return E_FAIL;
	}

	return S_OK;
}
