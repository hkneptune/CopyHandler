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
#ifndef __TMULTILOGGERCONFIG_H__
#define __TMULTILOGGERCONFIG_H__

#include <map>
#include "TLoggerLevelConfig.h"
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/lock_types.hpp>

namespace logger
{
	class TMultiLoggerConfig
	{
	public:
		TLoggerLevelConfigPtr GetLoggerConfig(PCTSTR pszChannel, bool bForceAdd = false);
		void SetLogLevel(PCTSTR pszChannel, ESeverityLevel eLevel);

	private:
		TLoggerLevelConfigPtr GetLoggerConfig(boost::upgrade_lock<boost::shared_mutex>& lock, PCTSTR pszChannel, bool bForceAdd);

	private:
		std::map<std::wstring, TLoggerLevelConfigPtr> m_mapConfigs;	// channel, config
		boost::shared_mutex m_mutex;
	};

	using TMultiLoggerConfigPtr = std::shared_ptr<TMultiLoggerConfig>;
}

#endif
