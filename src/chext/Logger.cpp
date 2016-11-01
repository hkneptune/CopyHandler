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
#include "Logger.h"

#include <boost/algorithm/string/replace.hpp>
#include "../liblogger/TMultiLoggerConfig.h"
#include "../liblogger/TAsyncMultiLogger.h"

namespace
{
	struct LoggerInfo
	{
		std::wstring strLogPath;
		DWORD dwMinLogLevel = 0;
	};

	bool ReadLoggerConfig(LoggerInfo& rInfo)
	{
		HKEY hKeyShellExt = nullptr;
		LSTATUS lStatus = RegOpenKeyEx(HKEY_CURRENT_USER, _T("SOFTWARE\\CopyHandler\\ShellExtension"), 0, KEY_QUERY_VALUE, &hKeyShellExt);
		if(lStatus != ERROR_SUCCESS)
			return false;

		// log path
		DWORD dwType = REG_SZ;
		const DWORD stMaxBuffer = 1024;
		std::unique_ptr<wchar_t[]> buf(new wchar_t[stMaxBuffer]);

		DWORD dwCount = stMaxBuffer;
		lStatus = RegQueryValueEx(hKeyShellExt, L"LogPath", nullptr, &dwType, (BYTE*)buf.get(), &dwCount);
		if(lStatus != ERROR_SUCCESS)
		{
			RegCloseKey(hKeyShellExt);
			return false;
		}

		buf[ dwCount / 2 ] = L'\0';
		rInfo.strLogPath = buf.get();

		// log level
		dwType = REG_DWORD;
		dwCount = sizeof(DWORD);
		DWORD dwValue = 0;
		lStatus = RegQueryValueEx(hKeyShellExt, L"MinLogLevel", nullptr, &dwType, (BYTE*)&dwValue, &dwCount);
		if(lStatus != ERROR_SUCCESS)
		{
			RegCloseKey(hKeyShellExt);
			return false;
		}

		rInfo.dwMinLogLevel = dwValue;

		RegCloseKey(hKeyShellExt);

		return true;
	}

	logger::TLogFileDataPtr CreateLoggerData()
	{
		LoggerInfo li;
		if(ReadLoggerConfig(li))
		{
			logger::TMultiLoggerConfigPtr spConfig(std::make_shared<logger::TMultiLoggerConfig>());
			spConfig->SetLogLevel(L"default", (logger::ESeverityLevel)li.dwMinLogLevel);

			DWORD dwProcessId = GetProcessId(GetCurrentProcess());
			boost::replace_all(li.strLogPath, L"%pid%", boost::lexical_cast<std::wstring>(dwProcessId));

			return logger::TAsyncMultiLogger::GetInstance()->CreateLoggerData(li.strLogPath.c_str(), spConfig);
		}

		logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());
		return spLogData;
	}
}

logger::TLoggerPtr GetLogger(PCTSTR pszChannel)
{
	return logger::MakeLogger(GetLogFileData(), pszChannel);
}

logger::TLogFileDataPtr GetLogFileData()
{
	static logger::TLogFileDataPtr spLogFileData = CreateLoggerData();
	return spLogFileData;
}
