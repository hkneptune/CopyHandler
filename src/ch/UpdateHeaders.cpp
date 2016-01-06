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
#include "UpdateHeaders.h"
#include "WindowsVersion.h"
#include <boost\lexical_cast.hpp>
#include <string>
#include "..\common\version.h"

UpdateHeaders::UpdateHeaders()
{
}

std::wstring UpdateHeaders::GetUserAgent()
{
	std::wstring wstrUserAgent(PRODUCT_FULL_VERSION_T);
	wstrUserAgent += L" (" +
		boost::lexical_cast<std::wstring>(PRODUCT_VERSION1) + L"." +
		boost::lexical_cast<std::wstring>(PRODUCT_VERSION2) + L"." +
		boost::lexical_cast<std::wstring>(PRODUCT_VERSION3) + L"." +
		boost::lexical_cast<std::wstring>(PRODUCT_VERSION4) + L")";

	wstrUserAgent += L" (" + m_tWindowsVersion.GetWindowsVersion() + L")";

	return wstrUserAgent;
}

std::wstring UpdateHeaders::GetHeaders(const std::wstring& wstrLanguagePath, UpdateVersionInfo::EVersionType eUpdateChannel)
{
	std::wstring wstrLanguage;
	size_t stPos = wstrLanguagePath.rfind(L'\\');
	if(stPos != std::wstring::npos)
		wstrLanguage = wstrLanguagePath.substr(stPos + 1);
	else
		wstrLanguage = L"unknown";

	std::wstring wstrHeaders;

	// copy handler version
	wstrHeaders = L"CopyHandler-Version: " +
		boost::lexical_cast<std::wstring>(PRODUCT_VERSION1) + L"." +
		boost::lexical_cast<std::wstring>(PRODUCT_VERSION2) + L"." +
		boost::lexical_cast<std::wstring>(PRODUCT_VERSION3) + L"." +
		boost::lexical_cast<std::wstring>(PRODUCT_VERSION4) + L"\r\n";

	// system version
	wstrHeaders += L"CopyHandler-OSVersionNumeric: " + m_tWindowsVersion.GetWindowsVersionNumeric() + L"\r\n";
	wstrHeaders += L"CopyHandler-OSReadableVersion: " + m_tWindowsVersion.GetWindowsVersionLongName() + L"\r\n";
	wstrHeaders += L"CopyHandler-OSInstallType: " + m_tWindowsVersion.GetWindowsInstallType() + L"\r\n";

	// cpu/os arch
	wstrHeaders += L"CopyHandler-OSArch: " + m_tWindowsVersion.GetCpuArch() + L"\r\n";

	// language
	wstrHeaders += L"CopyHandler-Language: " + wstrLanguage + L"\r\n";

	// #todo update channel
	wstrHeaders += L"CopyHandler-UpdateChannel: " + GetUpdateChannel(eUpdateChannel) + L"\r\n";

	// finalize
	wstrHeaders += L"\r\n\r\n";

	return wstrHeaders;
}

std::wstring UpdateHeaders::GetUpdateChannel(UpdateVersionInfo::EVersionType eUpdateChannel)
{
	switch(eUpdateChannel)
	{
	case UpdateVersionInfo::eAlpha:
		return L"alpha";

	case UpdateVersionInfo::eBeta:
		return L"beta";

	case UpdateVersionInfo::eReleaseCandidate:
		return L"rc";

	case UpdateVersionInfo::eStable:
		return L"stable";

	default:
		return L"unknown";
	}
}
