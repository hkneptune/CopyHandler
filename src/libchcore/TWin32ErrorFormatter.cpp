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
#include "TWin32ErrorFormatter.h"
#include <algorithm>
#include <atltrace.h>

namespace chcore
{
	TString TWin32ErrorFormatter::FormatWin32ErrorCode(DWORD dwErrorCode, bool bUseNumberFallback)
	{
		return FormatWin32ErrorCodeWithModule(dwErrorCode, nullptr, bUseNumberFallback);
	}

	TString TWin32ErrorFormatter::FormatWin32ErrorCodeWithFallback(DWORD dwErrorCode, const wchar_t* pszModuleName, bool bUseNumberFallback)
	{
		TString strError = FormatWin32ErrorCode(dwErrorCode, false);
		if (strError.IsEmpty())
			return FormatWin32ErrorCodeWithModule(dwErrorCode, GetModuleHandle(pszModuleName), bUseNumberFallback);

		return strError;
	}

	TString TWin32ErrorFormatter::FormatWin32ErrorCodeWithModule(DWORD dwErrorCode, HMODULE hModule, bool bUseNumberFallback)
	{
		const DWORD dwMaxError = 1024;

		TString strData;
		wchar_t* pszBuffer = strData.GetBuffer(dwMaxError);

		DWORD dwPos = 0;
		if(hModule)
			dwPos = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_FROM_HMODULE, hModule, dwErrorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), pszBuffer, dwMaxError - 1, nullptr);
		else
			dwPos = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, dwErrorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), pszBuffer, dwMaxError - 1, nullptr);

		if (dwPos == 0xffffffff)
		{
			int iPos = 0;
			if (bUseNumberFallback)
				iPos = _sntprintf_s(pszBuffer, dwMaxError, _TRUNCATE, _T("ErrorCode: 0x%lx"), dwErrorCode);
			strData.ReleaseBufferSetLength(iPos < 0 ? 0 : iPos);
		}
		else
		{
			if(dwPos == 0)
				ATLTRACE(L"Cannot format error message. Error code=%lu\n", GetLastError());

			strData.ReleaseBufferSetLength(std::min(dwPos, dwMaxError - 1));
		}

		strData.TrimRightSelf(_T("\r\n"));

		return strData;
	}
}
