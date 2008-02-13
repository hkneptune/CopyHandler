/************************************************************************
	Copy Handler 1.x - program for copying data	in Microsoft Windows
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

// Note: Proxy/Stub Information
//      To build a separate proxy/stub DLL, 
//      run nmake -f CopyHandlerShellExtps.mk in the project directory.

#include "stdafx.h"
#include "resource.h"
#include <initguid.h>
#include "chext.h"

#include "chext_i.c"
#include "MenuExt.h"
#include "DropMenuExt.h"

CComModule _Module;

// common memory - exactly 64kB
CSharedConfigStruct* g_pscsShared;
static HANDLE hMapObject=NULL;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_MenuExt, CMenuExt)
OBJECT_ENTRY(CLSID_DropMenuExt, CDropMenuExt)
END_OBJECT_MAP()

/////////////////////////////////////////////////////////////////////////////
// DLL Entry Point

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID /*lpReserved*/)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		_Module.Init(ObjectMap, hInstance, &LIBID_CHEXTLib);
        DisableThreadLibraryCalls(hInstance);
		
		// memory mapped file
		hMapObject = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(CSharedConfigStruct), _T("CHLMFile"));    // name of map object
		if (hMapObject == NULL) 
			return FALSE; 
		
		// The first process to attach initializes memory.
//		bool bInit = (GetLastError() != ERROR_ALREADY_EXISTS); 
		
		// Get a pointer to the file-mapped shared memory.
		g_pscsShared = (CSharedConfigStruct*)MapViewOfFile(hMapObject, FILE_MAP_WRITE, 0, 0, 0);
		if (g_pscsShared == NULL) 
			return FALSE; 
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		// Unmap shared memory from the process's address space.
		UnmapViewOfFile((LPVOID)g_pscsShared); 
		
		// Close the process's handle to the file-mapping object.
		CloseHandle(hMapObject); 

		_Module.Term();
	}
    return TRUE;    // ok
}

/////////////////////////////////////////////////////////////////////////////
// Used to determine whether the DLL can be unloaded by OLE

STDAPI DllCanUnloadNow(void)
{
    return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// Returns a class factory to create an object of the requested type

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _Module.GetClassObject(rclsid, riid, ppv);
}

/////////////////////////////////////////////////////////////////////////////
// DllRegisterServer - Adds entries to the system registry

STDAPI DllRegisterServer(void)
{
    // registers object, typelib and all interfaces in typelib
    return _Module.RegisterServer(TRUE);
}

/////////////////////////////////////////////////////////////////////////////
// DllUnregisterServer - Removes entries from the system registry

STDAPI DllUnregisterServer(void)
{
    return _Module.UnregisterServer(TRUE);
}


