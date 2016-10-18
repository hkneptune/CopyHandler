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
#ifndef __TLOGFILE_H__
#define __TLOGFILE_H__

#include <set>
#include "TLoggerRotationInfo.h"
#include <vector>
#include <list>

namespace logger
{
	namespace internal
	{
		class TLogFile
		{
		public:
			static const time_t MaxHandleCacheTime = 60;

		public:
			TLogFile(PCTSTR pszPath, const TLoggerRotationInfoPtr& spRotationInfo);

			void Write(std::list<std::wstring>& pszData);
			void CloseIfUnused();
			void CloseLogFile();

			const std::vector<std::wstring>& GetRotatedLogs() const;
			std::wstring GetLogPath() const;

		private:
			HANDLE GetFileHandle();
			unsigned long long GetCurrentLogSize();
			void RotateFile();
			void RemoveObsoleteRotatedLogs();
			void ScanForRotatedLogs();
			bool NeedRotation(size_t stDataSize);

		private:
			std::wstring m_strLogPath;

			time_t m_timeLastWriteTime = 0;
			std::shared_ptr<void> m_spFileHandle;

			// rotation
			TLoggerRotationInfoPtr m_spRotationInfo;
			std::vector<std::wstring> m_vRotatedFiles;
		};
	}
}

#endif
