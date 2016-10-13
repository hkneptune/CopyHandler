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
#include "TLogger.h"
#include "TMultiLoggerConfig.h"

namespace logger
{
	TLogger::TLogger(const TLogFileDataPtr& spFileData, PCTSTR pszChannel) :
		m_spFileData(spFileData),
		m_spLoggerConfig(spFileData->GetMultiLoggerConfig()->GetLoggerConfig(pszChannel)),
		m_strChannel(pszChannel)
	{
		if (!spFileData)
			throw std::invalid_argument("spFileData");
	}

	TLogFileDataPtr TLogger::GetLogFileData() const
	{
		return m_spFileData;
	}

	ESeverityLevel TLogger::GetMinSeverity() const
	{
		return m_spLoggerConfig->GetMinSeverityLevel();
	}

	TLogRecord TLogger::OpenLogRecord(ESeverityLevel eLevel) const
	{
		return TLogRecord(m_spFileData, eLevel);
	}
}
