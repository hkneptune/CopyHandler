// ============================================================================
//  Copyright (C) 2001-2015 by Jozef Starosczyk
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
#include "ShellExtensionVerifier.h"
#include "Logger.h"
#include "../libchcore/TSharedMemory.h"
#include "../libchcore/TConfig.h"
#include "../liblogger/TLogger.h"
#include "../common/TShellExtMenuConfig.h"
#include <stdlib.h>

HWND ShellExtensionVerifier::VerifyShellExt(IShellExtControl* piShellExtControl)
{
	logger::TLoggerPtr spLogger = GetLogger(L"ShellExtVerifier");

	HRESULT hResult = IsShellExtEnabled(piShellExtControl);
	if(FAILED(hResult) || hResult == S_FALSE)
	{
		LOG_DEBUG(spLogger) << L"Shell extension is disabled.";
		return nullptr;
	}

	// find CH's window
	HWND hWnd = ::FindWindow(_T("Copy Handler Wnd Class"), _T("Copy handler"));
	if(!hWnd)
	{
		LOG_DEBUG(spLogger) << L"Cannot find Copy Handler's window.";
		return nullptr;
	}

	return hWnd;
}

HRESULT ShellExtensionVerifier::IsShellExtEnabled(IShellExtControl* piShellExtControl)
{
	if(!piShellExtControl)
		return E_FAIL;

	LONG lFlags = eShellExt_None;
	HRESULT hResult = piShellExtControl->GetFlags(&lFlags);
	if(FAILED(hResult))
		return hResult;

	if(lFlags & eShellExt_Enabled)
		return S_OK;
	
	return S_FALSE;
}

HRESULT ShellExtensionVerifier::ReadShellConfig(IShellExtControl* piShellExtControl, TShellExtMenuConfig& tShellExtConfig)
{
	logger::TLoggerPtr spLogger = GetLogger(L"ShellExtVerifier");
	try
	{
		HWND hWnd = ShellExtensionVerifier::VerifyShellExt(piShellExtControl);
		if(hWnd == nullptr)
			return E_FAIL;

		// generate a random number for naming shared memory
		unsigned int uiSHMID = 0;
		if(rand_s(&uiSHMID) != 0 || uiSHMID == 0)
		{
			LOG_WARNING(spLogger) << L"Failed to generate random number for shared memory naming. Falling back to tick count.";
			uiSHMID = GetTickCount();
		}

		LOG_DEBUG(spLogger) << L"Requesting CH configuration. Shared memory identifier " << uiSHMID;

		if(::SendMessage(hWnd, WM_GETCONFIG, 0, uiSHMID) != TRUE)
		{
			LOG_ERROR(spLogger) << L"Failed to retrieve configuration from Copy Handler";
			return E_FAIL;
		}

		std::wstring strSHMName = IPCSupport::GenerateSHMName(uiSHMID);

		chcore::TSharedMemory tSharedMemory;
		chcore::TString wstrData;
		chcore::TConfig cfgShellExtData;

		tSharedMemory.Open(strSHMName.c_str());
		tSharedMemory.Read(wstrData);

		LOG_TRACE(spLogger) << L"Retrieved shell ext config: " << wstrData;

		cfgShellExtData.ReadFromString(wstrData);

		tShellExtConfig.ReadFromConfig(cfgShellExtData, _T("ShellExtCfg"));

		return S_OK;
	}
	catch(...)
	{
		return E_FAIL;
	}
}
