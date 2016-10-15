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
#ifndef __TLOGGERCONFIG_H__
#define __TLOGGERCONFIG_H__

#include "SeverityLevels.h"
#include <boost/thread/shared_mutex.hpp>
#include "liblogger.h"

namespace logger
{
	class LIBLOGGER_API TLoggerLevelConfig
	{
	public:
		explicit TLoggerLevelConfig(ESeverityLevel eMinSeverity = trace);

		void SetMinSeverityLevel(ESeverityLevel eLevel);
		ESeverityLevel GetMinSeverityLevel() const;

	private:
		volatile mutable unsigned int m_uiMinSeverity = trace;
	};

	using TLoggerLevelConfigPtr = std::shared_ptr<TLoggerLevelConfig>;
}

#endif
