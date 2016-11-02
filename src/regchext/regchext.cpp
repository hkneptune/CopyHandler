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
#include <string>
#include "TExtensionDetector.h"
#include "TComRegistrar.h"

int APIENTRY wWinMain(_In_ HINSTANCE /*hInstance*/,
	_In_opt_ HINSTANCE /*hPrevInstance*/,
	_In_ LPWSTR    /*lpCmdLine*/,
	_In_ int       /*nCmdShow*/)
{
	bool bRegister = true;

	int argc = __argc;
	if(argc > 1)
	{
		const wchar_t* pszParam = __wargv[ 1 ];
		std::wstring wstrParam = pszParam;
		if(wstrParam == L"/u" || wstrParam == L"/U")
			bRegister = false;
	}

	try
	{
		TExtensionDetector extensions;
		TComRegistrar registrar;

		if(bRegister)
		{
#ifdef _WIN64
			registrar.Register32bit(extensions.Get32bitExtension().c_str(), extensions.Get32bitBasePath().c_str());
#endif
			registrar.RegisterNative(extensions.GetNativeExtension().c_str(), extensions.GetNativeBasePath().c_str());
		}
		else
		{
#ifdef _WIN64
			registrar.Unregister32bit(extensions.Get32bitExtension().c_str(), extensions.Get32bitBasePath().c_str());
#endif
			registrar.UnregisterNative(extensions.GetNativeExtension().c_str(), extensions.GetNativeBasePath().c_str());
		}
	}
	catch(const std::exception&)
	{
		return 1;
	}

	return 0;
}
