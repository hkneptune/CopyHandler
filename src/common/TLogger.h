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

#include <boost/log/trivial.hpp>
#include <boost/log/sources/severity_channel_logger.hpp>
#include "TLoggerLevelConfig.h"
#include "TLoggerLocationConfig.h"
#include <memory>

using boost::log::trivial::severity_level;
using Logger = boost::log::sources::wseverity_channel_logger_mt<severity_level, std::wstring>;

namespace chcore
{
	class TLogger : public Logger
	{
	private:
		TLogger(const TLoggerLevelConfigPtr& spLoggerConfig, const TLoggerLocationConfigPtr& spLogLocation, PCTSTR pszChannel);

	public:
		severity_level GetMinSeverity() const;

	private:
		boost::log::attribute_set::iterator m_iterLogPath;
		TLoggerLevelConfigPtr m_spLoggerConfig;

		friend class TLoggerFactory;
	};

	using TLoggerPtr = std::unique_ptr<TLogger>;
}

#define LOG_TRACE(logger) if(boost::log::trivial::trace >= (logger)->GetMinSeverity()) BOOST_LOG_SEV((*logger), boost::log::trivial::trace)
#define LOG_DEBUG(logger) if(boost::log::trivial::debug >= (logger)->GetMinSeverity()) BOOST_LOG_SEV((*logger), boost::log::trivial::debug)
#define LOG_INFO(logger) if(boost::log::trivial::info >= (logger)->GetMinSeverity()) BOOST_LOG_SEV((*logger), boost::log::trivial::info)
#define LOG_WARNING(logger) if(boost::log::trivial::warning >= (logger)->GetMinSeverity()) BOOST_LOG_SEV((*logger), boost::log::trivial::warning)
#define LOG_ERROR(logger) if(boost::log::trivial::error >= (logger)->GetMinSeverity()) BOOST_LOG_SEV((*logger), boost::log::trivial::error)
#define LOG_FATAL(logger) if(boost::log::trivial::fatal >= (logger)->GetMinSeverity()) BOOST_LOG_SEV((*logger), boost::log::trivial::fatal)

#endif
