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
#ifndef __TLOGFILEDATA_H__
#define __TLOGFILEDATA_H__

#include <list>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/lock_types.hpp>
#include "TLogFile.h"
#include "TMultiLoggerConfig.h"
#include "liblogger.h"

namespace logger
{
	class TLoggerPaths;

	class LIBLOGGER_API TLogFileData
	{
	public:
		TLogFileData();
		TLogFileData(PCTSTR pszLogPath, const TMultiLoggerConfigPtr& spLoggerConfig, const TLoggerRotationInfoPtr& spRotationInfo);

		TMultiLoggerConfigPtr GetMultiLoggerConfig() const;

		void GetAllLogPaths(TLoggerPaths& rLoggerPaths) const;
		TLoggerPaths GetMainLogPath() const;

	private:
		void PushLogEntry(std::wstring strLine);
		void DisableLogging();

		std::shared_ptr<void> GetEntriesEvent() const;
		void StoreLogEntries();
		void CloseUnusedFile();

	private:
#pragma warning(push)
#pragma warning(disable: 4251)
		std::list<std::wstring> m_listEntries;
		boost::shared_mutex m_mutex;

		std::shared_ptr<void> m_spHasEntriesEvent;

		TMultiLoggerConfigPtr m_spLoggerConfig;

		std::unique_ptr<internal::TLogFile> m_spLogFile;
#pragma warning(pop)

		bool m_bLoggingEnabled = true;

		friend class TLogRecord;
		friend class TAsyncMultiLogger;
	};

	using TLogFileDataPtr = std::shared_ptr<TLogFileData>;
}

#endif
