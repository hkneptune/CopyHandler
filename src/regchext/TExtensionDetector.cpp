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
#include "TExtensionDetector.h"
#include <shlwapi.h>

#ifdef _WIN64
	#define DLL_NATIVE L"chext64.dll"
	#define DLL_32BIT L"chext.dll"
#else
	#define DLL_NATIVE L"chext.dll"
#endif

TExtensionDetector::TExtensionDetector()
{
	DetectPaths();
}

void TExtensionDetector::DetectPaths()
{
	// get path of this file
	const DWORD dwMaxPath = 32768;
	wchar_t szThisPath[ dwMaxPath ];
	DWORD dwLen = GetModuleFileName(nullptr, szThisPath, dwMaxPath);
	if(dwLen == 0)
		throw std::runtime_error("Cannot retrieve program path");

	szThisPath[ dwLen ] = L'\0';
	std::wstring wstrThisPath = szThisPath;
	size_t stPos = wstrThisPath.find_last_of(L'\\');
	if(stPos != std::wstring::npos)
		wstrThisPath.erase(wstrThisPath.begin() + stPos + 1, wstrThisPath.end());
	else if(*wstrThisPath.rbegin() != L'\\')
		wstrThisPath += L'\\';

	// find chext.dll/chext64.dll
	m_strNativeExtension = wstrThisPath + DLL_NATIVE;
	m_strNativeBasePath = wstrThisPath;
	if(!PathFileExists(m_strNativeExtension.c_str()))
		throw std::runtime_error("Native extension does not exist");

#ifdef _WIN64
	m_str32bitExtension = wstrThisPath + DLL_32BIT;
	m_str32bitBasePath = wstrThisPath;

	if(!PathFileExists(m_strNativeExtension.c_str()))
	{
		m_str32bitExtension = wstrThisPath + L"ShellExt32\\" + DLL_32BIT;
		m_str32bitBasePath = wstrThisPath + L"ShellExt32\\";
		if(!PathFileExists(m_strNativeExtension.c_str()))
			throw std::runtime_error("32bit extension does not exist");
	}
#endif
}
