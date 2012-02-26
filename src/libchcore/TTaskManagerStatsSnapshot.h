// ============================================================================
//  Copyright (C) 2001-2012 by Jozef Starosczyk
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
/// @file  TTaskManagerStatsSnapshot.h
/// @date  2012/2/26
/// @brief Contains class responsible for holding task manager stats.
// ============================================================================
#ifndef __TTASKMANAGERSTATSSNAPSHOT_H__
#define __TTASKMANAGERSTATSSNAPSHOT_H__

#include "libchcore.h"

BEGIN_CHCORE_NAMESPACE

class LIBCHCORE_API TTaskManagerStatsSnapshot
{
public:
	TTaskManagerStatsSnapshot();
	TTaskManagerStatsSnapshot(const TTaskManagerStatsSnapshot& rSrc);

	TTaskManagerStatsSnapshot& operator=(const TTaskManagerStatsSnapshot& rSrc);

	void Clear();

	size_t GetProcessedCount() const { return m_stProcessedCount; }
	void SetProcessedCount(size_t stCount) { m_stProcessedCount = stCount; }

	size_t GetTotalCount() const { return m_stTotalCount; }
	void SetTotalCount(size_t stTotalCount) { m_stTotalCount = stTotalCount; }

	unsigned long long GetProcessedSize() const { return m_ullProcessedSize; }
	void SetProcessedSize(unsigned long long ullProcessedSize) { m_ullProcessedSize = ullProcessedSize; }

	unsigned long long GetTotalSize() const { return m_ullTotalSize; }
	void SetTotalSize(unsigned long long ullTotalSize) { m_ullTotalSize = ullTotalSize; }

	double GetGlobalProgressInPercent() const { return m_dGlobalProgressInPercent; }
	void SetGlobalProgressInPercent(double dPercent) { m_dGlobalProgressInPercent = dPercent; }

	size_t GetRunningTasks() const { return m_stRunningTasks; }
	void SetRunningTasks(size_t stRunningTasks) { m_stRunningTasks = stRunningTasks; }

private:
	size_t m_stProcessedCount;
	size_t m_stTotalCount;

	unsigned long long m_ullProcessedSize;
	unsigned long long m_ullTotalSize;

	double m_dGlobalProgressInPercent;

	size_t m_stRunningTasks;
};

END_CHCORE_NAMESPACE

#endif
