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
#include "TMultiLoggerConfig.h"

namespace logger
{
	TLoggerLevelConfigPtr TMultiLoggerConfig::GetLoggerConfig(PCTSTR pszChannel, bool bForceAdd)
	{
		boost::upgrade_lock<boost::shared_mutex> lock(m_mutex);
		return GetLoggerConfig(lock, pszChannel, bForceAdd);
	}

	TLoggerLevelConfigPtr TMultiLoggerConfig::GetLoggerConfig(boost::upgrade_lock<boost::shared_mutex>& lock, PCTSTR pszChannel, bool bForceAdd)
	{
		auto iterConfig = m_mapConfigs.find(pszChannel);
		if (iterConfig == m_mapConfigs.end())
		{
			if (bForceAdd)
			{
				boost::upgrade_to_unique_lock<boost::shared_mutex> upgraded_lock(lock);
				iterConfig = m_mapConfigs.insert(std::make_pair(pszChannel, std::make_shared<TLoggerLevelConfig>())).first;
			}
			else
				return GetLoggerConfig(lock, L"default", true);
		}

		return iterConfig->second;
	}

	void TMultiLoggerConfig::SetLogLevel(PCTSTR pszChannel, ESeverityLevel eLevel)
	{
		TLoggerLevelConfigPtr spLoggerConfig = GetLoggerConfig(pszChannel, true);
		spLoggerConfig->SetMinSeverityLevel(eLevel);
	}
}
