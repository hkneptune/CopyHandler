#ifndef __TLOGGERINFO_H__
#define __TLOGGERINFO_H__

#include "libchcore.h"
#include "TPath.h"

namespace chcore
{
	class LIBCHCORE_API TLoggerInfo
	{
	public:
		TLoggerInfo(const TSmartPath& pathLog);

		TSmartPath GetLogPath() const;

	private:
		TSmartPath m_strLogPath;
	};

	using TLoggerInfoPtr = std::shared_ptr<TLoggerInfo>;
}

#endif
