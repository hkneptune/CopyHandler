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
#include "TLogger.h"

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/support/exception.hpp>
#include <boost/locale.hpp>

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;
namespace expr = boost::log::expressions;
namespace attrs = boost::log::attributes;

namespace
{
	struct LoggerInfo
	{
		std::wstring strLogPath;
		DWORD dwMinLogLevel = 0;
	};

	bool ReadLoggerConfig(LoggerInfo& rInfo)
	{
		HKEY hKeyShellExt = NULL;
		LSTATUS lStatus = RegOpenKeyEx(HKEY_CURRENT_USER, _T("SOFTWARE\\CopyHandler\\ShellExtension"), 0, KEY_QUERY_VALUE, &hKeyShellExt);
		if(lStatus != ERROR_SUCCESS)
			return false;

		// log path
		DWORD dwType = REG_SZ;
		const DWORD stMaxBuffer = 1024;
		std::unique_ptr<wchar_t[]> buf(new wchar_t[stMaxBuffer]);

		DWORD dwCount = stMaxBuffer;
		lStatus = RegQueryValueEx(hKeyShellExt, L"LogPath", NULL, &dwType, (BYTE*)buf.get(), &dwCount);
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
		lStatus = RegQueryValueEx(hKeyShellExt, L"MinLogLevel", NULL, &dwType, (BYTE*)&dwValue, &dwCount);
		if(lStatus != ERROR_SUCCESS)
		{
			RegCloseKey(hKeyShellExt);
			return false;
		}

		rInfo.dwMinLogLevel = dwValue;

		RegCloseKey(hKeyShellExt);

		return true;
	}
}

BOOST_LOG_GLOBAL_LOGGER_INIT(Logger, TLogger)
{
	LoggerInfo li;
	if(ReadLoggerConfig(li))
	{
		DWORD dwProcessId = GetProcessId(GetCurrentProcess());
		boost::replace_all(li.strLogPath, L"%pid%", boost::lexical_cast<std::wstring>(dwProcessId));

		logging::add_common_attributes();
		logging::core::get()->add_global_attribute("Scope", attrs::named_scope());

		auto sink = logging::add_file_log(
			keywords::file_name = li.strLogPath,
			keywords::rotation_size = 10 * 1024 * 1024,
			keywords::open_mode = (std::ios::out | std::ios::app),
			keywords::format =
			(
				expr::stream 
					<< expr::format_date_time< boost::posix_time::ptime >("TimeStamp", "[%Y-%m-%d %H:%M:%S]")
					<< "[" << severity << "]: "
					<< expr::wmessage
					<< expr::if_(expr::has_attr("Scope"))
					[
						expr::stream
						<< " ("
						<< expr::format_named_scope("Scope",
							keywords::format = "%n",
							keywords::depth = 1,
							keywords::iteration = expr::forward)
						<< ")"
					]
			)
		);

		std::locale loc = boost::locale::generator()("en_EN.UTF-8");
		sink->imbue(loc);

		severity_level eSeverity = (severity_level)li.dwMinLogLevel;
		logging::core::get()->set_filter(
			severity >= eSeverity
		);
	}

	boost::log::sources::wseverity_logger_mt<severity_level> lg;
	return lg;
}
