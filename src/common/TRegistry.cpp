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
#include <Shlwapi.h>
#include <boost/numeric/conversion/cast.hpp>

TRegistry::TRegistry(HKEY key, const wchar_t* pszKey, bool bReadOnly, bool bFailIfNotFound)
{
	ReOpen(key, pszKey, bReadOnly, bFailIfNotFound);
}

TRegistry::~TRegistry()
{
	if(IsOpen())
		RegCloseKey(m_hKey);
}

void TRegistry::ReOpen(HKEY key, const wchar_t* pszKey, bool bReadOnly, bool bFailIfNotFound)
{
	if (!pszKey)
		throw std::invalid_argument("pszKey");

	if (IsOpen())
		RegCloseKey(m_hKey);

	LSTATUS lStatus = RegOpenKeyEx(key, pszKey, 0, bReadOnly ? KEY_QUERY_VALUE : KEY_ALL_ACCESS, &m_hKey);
	if (lStatus != ERROR_SUCCESS || m_hKey == nullptr)
	{
		if(bFailIfNotFound || lStatus != ERROR_FILE_NOT_FOUND)
			throw std::runtime_error("Cannot open registry key");
	}
}

void TRegistry::CreateSubKey(const wchar_t* pszKey)
{
	if (!pszKey)
		throw std::invalid_argument("pszKey");
	if(!IsOpen())
		throw std::runtime_error("Registry key not open");

	HKEY newKey = nullptr;
	LSTATUS status = RegCreateKeyEx(m_hKey, pszKey, 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_CREATE_SUB_KEY, nullptr, &newKey, nullptr);
	if(status != ERROR_SUCCESS)
		throw std::runtime_error("Cannot create registry key");
}

void TRegistry::DeleteSubKey(const wchar_t* pszKey)
{
	if (!pszKey)
		throw std::invalid_argument("pszKey");
	if(!IsOpen())
		throw std::runtime_error("Registry key not open");

	LSTATUS status = SHDeleteKey(m_hKey, pszKey);
	if (status != ERROR_SUCCESS && status != ERROR_FILE_NOT_FOUND)
		throw std::runtime_error("Cannot delete registry key");
}

void TRegistry::DeleteValue(const wchar_t* pszValueKey)
{
	if (!pszValueKey)
		throw std::invalid_argument("pszValueKey");
	if(!IsOpen())
		throw std::runtime_error("Registry key not open");

	LSTATUS status = RegDeleteValue(m_hKey, pszValueKey);
	if (status != ERROR_SUCCESS && status != ERROR_FILE_NOT_FOUND)
		throw std::runtime_error("Cannot delete value");
}

bool TRegistry::QueryString(const wchar_t* pszValueKey, std::wstring& wstrValue)
{
	if (!pszValueKey)
		throw std::invalid_argument("pszValueKey");
	if(!IsOpen())
		throw std::runtime_error("Registry key not open");

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
	if (!pszValueKey)
		throw std::invalid_argument("pszValueKey");
	if(!IsOpen())
		throw std::runtime_error("Registry key not open");

	DWORD dwType = REG_DWORD;
	DWORD dwCount = sizeof(DWORD);
	DWORD dwValue = 0;
	LSTATUS lStatus = RegQueryValueEx(m_hKey, pszValueKey, nullptr, &dwType, (BYTE*)&dwValue, &dwCount);
	if (lStatus != ERROR_SUCCESS)
		return false;

	dwOutValue = dwValue;

	return true;
}

void TRegistry::SetString(const wchar_t* pszValueKey, const wchar_t* pszValue)
{
	if (!pszValueKey)
		throw std::invalid_argument("pszValueKey");
	if (!pszValue)
		throw std::invalid_argument("pszValue");
	if(!IsOpen())
		throw std::runtime_error("Registry key not open");

	size_t stValueLen = wcslen(pszValue) + 1;
	LSTATUS status = RegSetValueEx(m_hKey, pszValueKey, 0, REG_SZ, (const BYTE*)pszValue, boost::numeric_cast<DWORD>(stValueLen * sizeof(wchar_t)));
	if (status != ERROR_SUCCESS)
		throw std::runtime_error("Cannot set string value in registry");
}
