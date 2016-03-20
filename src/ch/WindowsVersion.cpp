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
	UpdateCachedData();

	std::wstring wstrShortInstallType;
	if(m_wstrInstallType != L"Client" && !m_wstrInstallType.empty())
		wstrShortInstallType = m_wstrInstallType[ 0 ];

	std::wstring wstrFullVer = m_wstrProductName;
	if (!m_wstrServicePack.empty())
		wstrFullVer += L" " + m_wstrServicePack;

	wstrFullVer += L" (" + m_wstrVersion + wstrShortInstallType + L")";
	return wstrFullVer;
}

std::wstring WindowsVersion::GetWindowsVersionNumeric()
{
	UpdateCachedData();
	return m_wstrVersion;
}

std::wstring WindowsVersion::GetWindowsVersionLongName()
{
	UpdateCachedData();
	return m_wstrProductName;
}

std::wstring WindowsVersion::GetWindowsInstallType()
{
	UpdateCachedData();
	return m_wstrInstallType;
}

std::wstring WindowsVersion::GetCpuArch()
{
	UpdateCachedData();
	return m_wstrCpuArch;
}

bool WindowsVersion::IsWindowsXP()
{
	OSVERSIONINFOEX ovi = { 0 };
	ovi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	if(!GetVersionEx((OSVERSIONINFO*)&ovi))
		return false;

	if(ovi.dwMajorVersion != 5)
		return false;

	// 32-bit WinXP
	if(ovi.dwMinorVersion == 1)
		return true;

	// 64bit WinXP
	SYSTEM_INFO si = { 0 };
	GetNativeSystemInfo(&si);

	return ovi.dwMinorVersion == 2 && ovi.wProductType == VER_NT_WORKSTATION && si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64;
}

bool WindowsVersion::IsWindows7Or2008R2OrGreater()
{
	OSVERSIONINFOEX ovi = { 0 };
	ovi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	if(!GetVersionEx((OSVERSIONINFO*)&ovi))
		return false;

	if(ovi.dwMajorVersion != 6)
		return ovi.dwMajorVersion > 6;

	if(ovi.dwMinorVersion >= 1)
		return true;

	return false;
}

void WindowsVersion::UpdateCachedData()
{
	if(m_bCacheFilled)
		return;

	TRegistry regCurrentVer(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion");

	DWORD dwMajor = 0;
	DWORD dwMinor = 0;
	if(regCurrentVer.QueryDword(L"CurrentMajorVersionNumber", dwMajor) && regCurrentVer.QueryDword(L"CurrentMinorVersionNumber", dwMinor))
		m_wstrVersion = boost::lexical_cast<std::wstring>(dwMajor) + L"." + boost::lexical_cast<std::wstring>(dwMinor);
	else
	{
		if(!regCurrentVer.QueryString(L"CurrentVersion", m_wstrVersion))
			m_wstrVersion = L"Unknown version";
	}

	if(regCurrentVer.QueryString(L"CurrentBuildNumber", m_wstrBuildNumber))
		m_wstrVersion += L"." + m_wstrBuildNumber;

	regCurrentVer.QueryString(L"ProductName", m_wstrProductName);
	regCurrentVer.QueryString(L"InstallationType", m_wstrInstallType);
	regCurrentVer.QueryString(L"CSDVersion", m_wstrServicePack);
	boost::replace_all(m_wstrServicePack, L"Service Pack ", L"SP");

	// cpu/os arch
	SYSTEM_INFO si = { 0 };
	GetNativeSystemInfo(&si);

	switch(si.wProcessorArchitecture)
	{
	case PROCESSOR_ARCHITECTURE_AMD64:
	{
#ifndef _M_AMD64
		m_wstrCpuArch = L"x86_64 WOW64";
#else
		m_wstrCpuArch = L"x86_64";
#endif
		break;
	}

	case PROCESSOR_ARCHITECTURE_INTEL:
	{
#ifndef _M_IX86
		m_wstrCpuArch = L"x86_32-";		// should not happen
#else
		m_wstrCpuArch = L"x86_32";
#endif
		break;
	}

	default:
		m_wstrCpuArch = L"A" + boost::lexical_cast<std::wstring>(si.wProcessorArchitecture);
	}
}
