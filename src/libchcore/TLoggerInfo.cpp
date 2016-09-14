#include "stdafx.h"
#include "TLoggerInfo.h"

namespace chcore
{
	TLoggerInfo::TLoggerInfo(const TSmartPath& pathLog) :
		m_strLogPath(pathLog)
	{
	}

	TSmartPath TLoggerInfo::GetLogPath() const
	{
		return m_strLogPath;
	}
}
