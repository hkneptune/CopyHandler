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
#ifndef __TLOGGER_H__
#define __TLOGGER_H__

#include "TLoggerLevelConfig.h"
#include <memory>
#include "TLogFileData.h"
#include "SeverityLevels.h"
#include "TMultiLoggerConfig.h"
#include "TLogRecord.h"

namespace logger
{
	class TLogger
	{
	public:
		TLogger(const TLogFileDataPtr& spFileData, PCTSTR pszChannel);

		TLogFileDataPtr GetLogFileData() const;
		ESeverityLevel GetMinSeverity() const;
		TLogRecord OpenLogRecord(ESeverityLevel eLevel) const;

	private:
		TLoggerLevelConfigPtr m_spLoggerConfig;
		TLogFileDataPtr m_spFileData;
		std::wstring m_strChannel;

		friend class TLoggerFactory;
	};

	inline TLogger::TLogger(const TLogFileDataPtr& spFileData, PCTSTR pszChannel) :
		m_spFileData(spFileData),
		m_spLoggerConfig(spFileData->GetMultiLoggerConfig()->GetLoggerConfig(pszChannel)),
		m_strChannel(pszChannel)
	{
		if(!spFileData)
			throw std::invalid_argument("spFileData");
	}

	inline TLogFileDataPtr TLogger::GetLogFileData() const
	{
		return m_spFileData;
	}

	inline ESeverityLevel TLogger::GetMinSeverity() const
	{
		return m_spLoggerConfig->GetMinSeverityLevel();
	}

	inline TLogRecord TLogger::OpenLogRecord(ESeverityLevel eLevel) const
	{
		return TLogRecord(m_spFileData, eLevel, m_strChannel);
	}

	using TLoggerPtr = std::unique_ptr<TLogger>;

	inline TLoggerPtr MakeLogger(const TLogFileDataPtr& spFileData, PCTSTR pszChannel)
	{
		return std::make_unique<TLogger>(spFileData, pszChannel);
	}
}

#define LOG(log, level) for(logger::TLogRecord rec = (log)->OpenLogRecord(level); rec.IsEnabled(); rec.Disable()) rec

#define LOG_TRACE(log) if(logger::trace >= (log)->GetMinSeverity()) LOG(log, logger::trace)
#define LOG_DEBUG(log) if(logger::debug >= (log)->GetMinSeverity()) LOG(log, logger::debug)
#define LOG_INFO(log) if(logger::info >= (log)->GetMinSeverity()) LOG(log, logger::info)
#define LOG_WARNING(log) if(logger::warning >= (log)->GetMinSeverity()) LOG(log, logger::warning)
#define LOG_ERROR(log) if(logger::error >= (log)->GetMinSeverity()) LOG(log, logger::error)
#define LOG_CRITICAL(log) if(logger::critical >= (log)->GetMinSeverity()) LOG(log, logger::critical)

#endif
