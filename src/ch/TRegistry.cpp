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
#include "TRegistry.h"

TRegistry::TRegistry(HKEY key, const wchar_t* pszKey)
{
	LSTATUS lStatus = RegOpenKeyEx(key, pszKey, 0, KEY_QUERY_VALUE, &m_hKey);
	if (lStatus != ERROR_SUCCESS || m_hKey == nullptr)
		throw std::runtime_error("Cannot open registry key");
}

TRegistry::~TRegistry()
{
	if(m_hKey)
		RegCloseKey(m_hKey);
}

bool TRegistry::QueryString(const wchar_t* pszValueKey, std::wstring& wstrValue)
{
	DWORD dwType = REG_SZ;
	const DWORD stMaxBuffer = 1024;
	std::unique_ptr<wchar_t[]> buf(new wchar_t[stMaxBuffer]);

	DWORD dwCount = stMaxBuffer;
	LSTATUS lStatus = RegQueryValueEx(m_hKey, pszValueKey, nullptr, &dwType, (BYTE*)buf.get(), &dwCount);
	if (lStatus != ERROR_SUCCESS)
		return false;

	buf[dwCount / 2] = L'\0';
	wstrValue = buf.get();

	return true;
}

bool TRegistry::QueryDword(const wchar_t* pszValueKey, DWORD& dwOutValue)
{
	DWORD dwType = REG_DWORD;
	DWORD dwCount = sizeof(DWORD);
	DWORD dwValue = 0;
	LSTATUS lStatus = RegQueryValueEx(m_hKey, pszValueKey, nullptr, &dwType, (BYTE*)&dwValue, &dwCount);
	if (lStatus != ERROR_SUCCESS)
		return false;

	dwOutValue = dwValue;

	return true;
}
