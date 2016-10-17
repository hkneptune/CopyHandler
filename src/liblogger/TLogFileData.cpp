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
#include "TLogFileData.h"

namespace logger
{
	TLogFileData::TLogFileData() :
		m_spHasEntriesEvent(CreateEvent(nullptr, TRUE, FALSE, nullptr), CloseHandle),
		m_spLoggerConfig(std::make_shared<TMultiLoggerConfig>()),
		m_spLogFile()
	{
	}

	TLogFileData::TLogFileData(PCTSTR pszLogPath, const TMultiLoggerConfigPtr& spLoggerConfig, const TLoggerRotationInfoPtr& spRotationInfo) :
		m_spHasEntriesEvent(CreateEvent(nullptr, TRUE, FALSE, nullptr), CloseHandle),
		m_spLoggerConfig(spLoggerConfig),
		m_spLogFile(std::make_unique<internal::TLogFile>(pszLogPath, spRotationInfo))
	{
		if(m_spHasEntriesEvent.get() == INVALID_HANDLE_VALUE)
			throw std::runtime_error("Cannot create file data event");
		if(!spLoggerConfig)
			throw std::runtime_error("spLoggerConfig");
	}

	TMultiLoggerConfigPtr TLogFileData::GetMultiLoggerConfig() const
	{
		return m_spLoggerConfig;
	}

	std::shared_ptr<void> TLogFileData::GetEntriesEvent() const
	{
		return m_spHasEntriesEvent;
	}

	void TLogFileData::PushLogEntry(std::wstring strLine)
	{
		boost::unique_lock<boost::shared_mutex> lock(m_mutex);
		if(m_spLogFile)
		{
			m_listEntries.emplace_back(strLine);
			SetEvent(m_spHasEntriesEvent.get());
		}
	}

	void TLogFileData::StoreLogEntries()
	{
		if(m_spLogFile)
			m_spLogFile->Write(m_listEntries);
	}

	void TLogFileData::CloseUnusedFile()
	{
		if(m_spLogFile)
			m_spLogFile->CloseIfUnused();
	}
}
