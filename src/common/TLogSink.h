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
#ifndef __TLOGSINK_H__
#define __TLOGSINK_H__

#include "..\libchcore\TPath.h"
#include "..\libchcore\TAutoHandles.h"
#include <set>

namespace chcore
{
	class TLogSink
	{
	public:
		TLogSink(const chcore::TSmartPath& pathLog);

		HANDLE GetFileHandle();
		unsigned long long GetCurrentLogSize();

		void CloseLogFile();
		void CloseIfTimedOut(unsigned long long ullCurrentTimestamp, unsigned long long ullMaxHandleCacheTime);

		void AddRotatedFile(const TSmartPath& rPath);
		void RemoveObsoleteRotatedLogs(unsigned int uiMaxRotatedFiles);

	private:
		chcore::TSmartPath m_pathLog;
		chcore::TAutoFileHandle m_handleFile;
		unsigned long long m_ullLastUsageTimestamp = 0;
		std::set<chcore::TSmartPath> m_setRotatedFiles;
	};
}

#endif
