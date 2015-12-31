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
#include "WindowsVersion.h"
#include "TRegistry.h"
#include <boost\lexical_cast.hpp>
#include <boost\algorithm\string\replace.hpp>

std::wstring WindowsVersion::GetWindowsVersion()
{
	TRegistry regCurrentVer(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion");

	std::wstring wstrVersion;
	std::wstring wstrProductName;
	std::wstring wstrInstallType;
	std::wstring wstrBuildNumber;
	std::wstring wstrServicePack;

	DWORD dwMajor = 0;
	DWORD dwMinor = 0;
	if (regCurrentVer.QueryDword(L"CurrentMajorVersionNumber", dwMajor) && regCurrentVer.QueryDword(L"CurrentMinorVersionNumber", dwMinor))
		wstrVersion = boost::lexical_cast<std::wstring>(dwMajor) + L"." + boost::lexical_cast<std::wstring>(dwMinor);
	else
	{
		if (!regCurrentVer.QueryString(L"CurrentVersion", wstrVersion))
			wstrVersion = L"Unknown version";
	}

	if (regCurrentVer.QueryString(L"CurrentBuildNumber", wstrBuildNumber))
		wstrVersion += L"." + wstrBuildNumber;

	regCurrentVer.QueryString(L"ProductName", wstrProductName);
	if (regCurrentVer.QueryString(L"InstallationType", wstrInstallType) && !wstrInstallType.empty())
	{
		if (wstrInstallType == L"Client")
			wstrInstallType.clear();
		else
			wstrInstallType = wstrInstallType[0];
	}

	regCurrentVer.QueryString(L"CSDVersion", wstrServicePack);
	boost::replace_all(wstrServicePack, L"Service Pack ", L"SP");

	std::wstring wstrFullVer = wstrProductName;
	if (!wstrServicePack.empty())
		wstrFullVer += L" " + wstrServicePack;

	wstrFullVer += L" (" + wstrVersion + wstrInstallType + L";" + GetCpuArch() + L")";
	return wstrFullVer;
}

std::wstring WindowsVersion::GetCpuArch()
{
	SYSTEM_INFO si = { 0 };
	GetNativeSystemInfo(&si);

	switch (si.wProcessorArchitecture)
	{
	case PROCESSOR_ARCHITECTURE_AMD64:
	{
#ifndef _M_AMD64
		return L"x64-";
#else
		return L"x64";
#endif
	}

	case PROCESSOR_ARCHITECTURE_INTEL:
	{
#ifndef _M_IX86
		return L"x86-";
#else
		return L"x86";
#endif
	}

	default:
		return L"A" + boost::lexical_cast<std::wstring>(si.wProcessorArchitecture);
	}
}
