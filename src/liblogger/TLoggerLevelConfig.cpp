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
#include "TLoggerLevelConfig.h"
#include <boost\thread\lock_types.hpp>

namespace logger
{
	TLoggerLevelConfig::TLoggerLevelConfig(ESeverityLevel eMinSeverity) :
		m_eMinSeverity(eMinSeverity)
	{
	}

	void TLoggerLevelConfig::SetMinSeverityLevel(ESeverityLevel eLevel)
	{
		boost::unique_lock<boost::shared_mutex> lock;
		m_eMinSeverity = eLevel;
	}

	ESeverityLevel TLoggerLevelConfig::GetMinSeverityLevel() const
	{
		boost::shared_lock<boost::shared_mutex> lock;
		return m_eMinSeverity;
	}
}
