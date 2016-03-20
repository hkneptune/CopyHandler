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
#ifndef __WINDOWSVERSION_H__
#define __WINDOWSVERSION_H__

#include <string>

class WindowsVersion
{
public:
	std::wstring GetWindowsVersion();
	std::wstring GetWindowsVersionNumeric();
	std::wstring GetWindowsVersionLongName();
	std::wstring GetWindowsInstallType();
	std::wstring GetCpuArch();

	static bool IsWindowsXP();

private:
	void UpdateCachedData();

private:
	bool m_bCacheFilled = false;
	std::wstring m_wstrVersion;
	std::wstring m_wstrProductName;
	std::wstring m_wstrInstallType;
	std::wstring m_wstrBuildNumber;
	std::wstring m_wstrServicePack;
	std::wstring m_wstrCpuArch;
};

#endif
