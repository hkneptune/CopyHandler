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
#ifndef __TASYNCMULTILOGGER_H__
#define __TASYNCMULTILOGGER_H__

#include "TLogFileData.h"
#include <unordered_set>
#include <thread>
#include "TLoggerRotationInfo.h"

namespace logger
{
	class TAsyncMultiLogger;

	using TAsyncMultiLoggerPtr = std::shared_ptr<TAsyncMultiLogger>;

	class TAsyncMultiLogger
	{
	public:
		static TAsyncMultiLoggerPtr GetInstance();

	public:
		TAsyncMultiLogger();

		void FinishLogging();
		TLogFileDataPtr CreateLoggerData(PCTSTR pszLogPath, const TMultiLoggerConfigPtr& spLoggerConfig);

		TLoggerRotationInfoPtr GetRotationInfo() const;
		void SetRotationInfo(unsigned int uiMaxLogSize, unsigned int uiMaxRotatedCount);

	private:
		void LoggingThread();

	private:
		std::unordered_set<TLogFileDataPtr> m_setLoggerData;
		boost::shared_mutex m_mutex;

		std::shared_ptr<void> m_spStopEvent;
		std::unique_ptr<std::thread> m_spThread;

		TLoggerRotationInfoPtr m_spGlobalRotationInfo;
	};
}

#endif
