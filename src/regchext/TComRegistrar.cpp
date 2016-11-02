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
#include "TComRegistrar.h"
#include <shlwapi.h>

TComRegistrar::TComRegistrar()
{
	DetectRegsvrPaths();
}

void TComRegistrar::RegisterNative(const wchar_t* pszPath, const wchar_t* pszDir)
{
	if(!PathFileExists(pszPath))
		throw std::runtime_error("File does not exist");

	Register(pszPath, pszDir, m_strNativeRegsvrPath.c_str());
}

void TComRegistrar::UnregisterNative(const wchar_t* pszPath, const wchar_t* pszDir)
{
	if(!PathFileExists(pszPath))
		throw std::runtime_error("File does not exist");

	Unregister(pszPath, pszDir, m_strNativeRegsvrPath.c_str());
}

#ifdef _WIN64

void TComRegistrar::Register32bit(const wchar_t* pszPath, const wchar_t* pszDir)
{
	if(!PathFileExists(pszPath))
		throw std::runtime_error("File does not exist");

	Register(pszPath, pszDir, m_str32bitRegsvr.c_str());
}

void TComRegistrar::Unregister32bit(const wchar_t* pszPath, const wchar_t* pszDir)
{
	if(!PathFileExists(pszPath))
		throw std::runtime_error("File does not exist");

	Unregister(pszPath, pszDir, m_str32bitRegsvr.c_str());
}

#endif

bool TComRegistrar::Register(const wchar_t* pszPath, const wchar_t* pszDir, const wchar_t* pszRegsvrPath)
{
	// try with regsvr32
	SHELLEXECUTEINFO sei = { 0 };
	sei.cbSize = sizeof(sei);
	sei.fMask = SEE_MASK_UNICODE;
	sei.lpVerb = L"runas";
	sei.lpFile = pszRegsvrPath;
	sei.lpDirectory = pszDir;
	std::wstring strParams = std::wstring(L"/s \"") + pszPath + L"\"";
	sei.lpParameters = strParams.c_str();
	sei.nShow = SW_SHOW;

	return ShellExecuteEx(&sei) != FALSE;
}

bool TComRegistrar::Unregister(const wchar_t* pszPath, const wchar_t* pszDir, const wchar_t* pszRegsvrPath)
{
	// try with regsvr32
	SHELLEXECUTEINFO sei = { 0 };
	sei.cbSize = sizeof(sei);
	sei.fMask = SEE_MASK_UNICODE;
	sei.lpVerb = L"runas";
	sei.lpFile = pszRegsvrPath;
	sei.lpDirectory = pszDir;
	std::wstring strParams = std::wstring(L"/u /s \"") + pszPath + L"\"";
	sei.lpParameters = strParams.c_str();
	sei.nShow = SW_SHOW;

	return ShellExecuteEx(&sei) != FALSE;
}

void TComRegistrar::DetectRegsvrPaths()
{
	wchar_t szWindowsPath[ _MAX_PATH ] = { 0 };

	if(GetWindowsDirectory(szWindowsPath, _MAX_PATH) == 0)
		throw std::runtime_error("Cannot detect Windows directory");

	std::wstring wstrWindowsPath = szWindowsPath;
	if(*wstrWindowsPath.rbegin() != L'\\')
		wstrWindowsPath += L'\\';

	m_strNativeRegsvrPath = wstrWindowsPath + L"system32\\regsvr32.exe";
	if(!PathFileExists(m_strNativeRegsvrPath.c_str()))
		throw std::runtime_error("Native regsvr32.exe does not exist");

#ifdef _WIN64
	m_str32bitRegsvr = wstrWindowsPath + L"SysWOW64\\regsvr32.exe";
	if(!PathFileExists(m_str32bitRegsvr.c_str()))
		throw std::runtime_error("32bit regsvr32.exe does not exist");
#endif
}
