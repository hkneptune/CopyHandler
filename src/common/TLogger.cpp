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
#include <boost/log/attributes/constant.hpp>
#include "../libchcore/TCoreException.h"

namespace keywords = boost::log::keywords;

namespace chcore
{
	TLogger::TLogger(const TLoggerLevelConfigPtr& spLoggerConfig, const TLoggerLocationConfigPtr& spLogLocation, PCTSTR pszChannel) :
		Logger(keywords::channel = pszChannel)
	{
		if (!spLoggerConfig)
			throw TCoreException(eErr_InvalidArgument, L"spLoggerConfig", LOCATION);
		if (!spLogLocation)
			throw TCoreException(eErr_InvalidArgument, L"spLogLocation", LOCATION);

		m_iterLogPath = add_attribute("LogPath", boost::log::attributes::constant<TLoggerLocationConfigPtr>(spLogLocation)).first;
	}

	severity_level TLogger::GetMinSeverity() const
	{
		return m_spLoggerConfig->GetMinSeverityLevel();
	}
}
