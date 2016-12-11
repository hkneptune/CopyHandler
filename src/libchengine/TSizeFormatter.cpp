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
#include "TSizeFormatter.h"
#include "../libchengine/TConfig.h"
#include <tchar.h>

namespace
{
	PCTSTR Concat(std::wstring& wstrBuffer, PCTSTR pszFirst, PCTSTR pszSecond)
	{
		if(pszFirst && pszFirst[ 0 ] != _T('\0'))
		{
			wstrBuffer = pszFirst;
			wstrBuffer += _T(".");
			wstrBuffer += pszSecond;
		}
		else
			wstrBuffer = pszSecond;

		return wstrBuffer.c_str();
	}
}

namespace chengine
{
	TSizeFormatter::TSizeFormatter()
	{
	}

	TSizeFormatter::TSizeFormatter(const wchar_t* strBytes, const wchar_t* strKBytes, const wchar_t* strMBytes, const wchar_t* strGBytes, const wchar_t* strTBytes) :
		m_strBytes(strBytes),
		m_strKBytes(strKBytes),
		m_strMBytes(strMBytes),
		m_strGBytes(strGBytes),
		m_strTBytes(strTBytes)
	{
	}

	void TSizeFormatter::SetValues(const wchar_t* strBytes, const wchar_t* strKBytes, const wchar_t* strMBytes, const wchar_t* strGBytes, const wchar_t* strTBytes)
	{
		m_strBytes = strBytes;
		m_strKBytes = strKBytes;
		m_strMBytes = strMBytes;
		m_strGBytes = strGBytes;
		m_strTBytes = strTBytes;
	}

	std::wstring TSizeFormatter::GetSizeString(unsigned long long ullData, bool bStrict) const
	{
		const size_t stMaxSize = 512;
		wchar_t szData[ stMaxSize ] = { 0 };

		if(ullData >= 1288490188800 && (!bStrict || (ullData % 1099511627776) == 0))
			_sntprintf_s(szData, stMaxSize, L"%.2f %s", (double)(ullData / 1099511627776.0), m_strTBytes.c_str());
		else if(ullData >= 1258291200 && (!bStrict || (ullData % 1073741824) == 0))
			_sntprintf_s(szData, stMaxSize, L"%.2f %s", (double)(ullData / 1073741824.0), m_strGBytes.c_str());
		else if(ullData >= 1228800 && (!bStrict || (ullData % 1048576) == 0))
			_sntprintf_s(szData, stMaxSize, _T("%.2f %s"), (double)(ullData / 1048576.0), m_strMBytes.c_str());
		else if(ullData >= 1200 && (!bStrict || (ullData % 1024) == 0))
			_sntprintf_s(szData, stMaxSize, _T("%.2f %s"), (double)(ullData / 1024.0), m_strKBytes.c_str());
		else
			_sntprintf_s(szData, stMaxSize, _T("%I64u %s"), ullData, m_strBytes.c_str());

		return szData;
	}

	void TSizeFormatter::StoreInConfig(chengine::TConfig& rConfig, PCTSTR pszNodeName) const
	{
		std::wstring wstrBuffer;
		SetConfigValue(rConfig, Concat(wstrBuffer, pszNodeName, L"Bytes"), m_strBytes.c_str());
		SetConfigValue(rConfig, Concat(wstrBuffer, pszNodeName, L"KBytes"), m_strKBytes.c_str());
		SetConfigValue(rConfig, Concat(wstrBuffer, pszNodeName, L"MBytes"), m_strMBytes.c_str());
		SetConfigValue(rConfig, Concat(wstrBuffer, pszNodeName, L"GBytes"), m_strGBytes.c_str());
		SetConfigValue(rConfig, Concat(wstrBuffer, pszNodeName, L"TBytes"), m_strTBytes.c_str());
	}

	bool TSizeFormatter::ReadFromConfig(chengine::TConfig& rConfig, PCTSTR pszNodeName)
	{
		std::wstring strBuffer;
		string::TString strValue;

		if(!GetConfigValue(rConfig, Concat(strBuffer, pszNodeName, _T("Bytes")), strValue))
			return false;
		m_strBytes = strValue.c_str();

		if(!GetConfigValue(rConfig, Concat(strBuffer, pszNodeName, _T("KBytes")), strValue))
			return false;
		m_strKBytes = strValue.c_str();

		if(!GetConfigValue(rConfig, Concat(strBuffer, pszNodeName, _T("MBytes")), strValue))
			return false;
		m_strMBytes = strValue.c_str();

		if(!GetConfigValue(rConfig, Concat(strBuffer, pszNodeName, _T("GBytes")), strValue))
			return false;
		m_strGBytes = strValue.c_str();

		if(!GetConfigValue(rConfig, Concat(strBuffer, pszNodeName, _T("TBytes")), strValue))
			return false;
		m_strTBytes = strValue.c_str();

		return true;
	}
}
