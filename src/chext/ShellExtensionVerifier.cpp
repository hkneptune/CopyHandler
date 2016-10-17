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

HWND ShellExtensionVerifier::VerifyShellExt(IShellExtControl* piShellExtControl)
{
	logger::TLoggerPtr rLogger = GetLogger(L"ShellExtVerifier");

	HRESULT hResult = IsShellExtEnabled(piShellExtControl);
	if(FAILED(hResult) || hResult == S_FALSE)
	{
		LOG_DEBUG(rLogger) << L"Shell extension is disabled.";
		return nullptr;
	}

	// find CH's window
	HWND hWnd = ::FindWindow(_T("Copy Handler Wnd Class"), _T("Copy handler"));
	if(!hWnd)
	{
		LOG_DEBUG(rLogger) << L"Cannot find Copy Handler's window.";
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
