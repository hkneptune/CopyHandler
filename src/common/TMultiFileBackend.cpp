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
#include "TMultiFileBackend.h"
#include "..\libchcore\TTimestampProviderTickCount.h"
#include <boost\algorithm\string\case_conv.hpp>
#include <boost/log/utility/value_ref.hpp>
#include <boost/log/attributes/value_extraction.hpp>
#include <boost\date_time\posix_time\posix_time_io.hpp>
#include <sstream>
#include "..\libchcore\TStringArray.h"
#include "TLoggerLocationConfig.h"

namespace chcore
{
	TMultiFileBackend::TMultiFileBackend(ITimestampProviderPtr spTimestampProvider, unsigned int uiMaxRotatedFiles, unsigned long long ullMaxLogSize) :
		m_spTimestampProvider(spTimestampProvider),
		m_logRotator(uiMaxRotatedFiles, ullMaxLogSize)
	{
		if (!m_spTimestampProvider)
			m_spTimestampProvider.reset(new TTimestampProviderTickCount);
	}

	void TMultiFileBackend::Init(const TSmartPath& pathDirectory, unsigned int uiMaxRotatedFiles, unsigned long long ullMaxLogSize)
	{
		SetDirectory(pathDirectory);
		m_logRotator.SetLimits(uiMaxRotatedFiles, ullMaxLogSize);
	}

	void TMultiFileBackend::consume(const boost::log::record_view& rec, const string_type& formatted_message)
	{
		if (!m_bInitialized)
			return;

		TSmartPath pathLog = GetLogName(rec);
		if (pathLog.IsEmpty())
			return;

		TLogSink& sinkData = m_mapLogs.GetSinkData(pathLog);

		HANDLE hFile = GetLogFile(pathLog, sinkData, formatted_message.length());
		if (hFile == INVALID_HANDLE_VALUE)
			return;

		string_type strFullMessage = formatted_message + "\r\n";

		DWORD dwToWrite = boost::numeric_cast<DWORD>(strFullMessage.length());
		DWORD dwWritten = 0;
		WriteFile(hFile, strFullMessage.c_str(), dwToWrite, &dwWritten, nullptr);
	}

	TSmartPath TMultiFileBackend::GetLogName(const boost::log::record_view &rec)
	{
		auto attrLogPath = rec.attribute_values().find("LogPath");
		if (attrLogPath == rec.attribute_values().end())
			return TSmartPath();

		boost::log::value_ref<TLoggerLocationConfigPtr> val = boost::log::extract<TLoggerLocationConfigPtr>(attrLogPath->second);
		if (!val)
			return TSmartPath();

		return val.get()->GetLogPath();
	}

	HANDLE TMultiFileBackend::GetLogFile(const TSmartPath& pathLog, TLogSink& sinkData, size_t stRequiredSpace)
	{
		m_logRotator.RotateFile(pathLog, sinkData, stRequiredSpace);

		return sinkData.GetFileHandle();
	}

	void TMultiFileBackend::SetDirectory(const TSmartPath& pathDirectory)
	{
		m_mapLogs.Clear();
		m_logRotator.ScanForLogs(pathDirectory, m_mapLogs);
		m_bInitialized = true;
	}

}
