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
#include "TTaskStatsSnapshot.h"

namespace chcore
{
	class LIBCHCORE_API TTaskManagerStatsSnapshot
	{
	public:
		static const unsigned long long AssumedFileEquivalentSize = 4096;

	public:
		TTaskManagerStatsSnapshot();
		TTaskManagerStatsSnapshot(const TTaskManagerStatsSnapshot& rSrc) = delete;

		TTaskManagerStatsSnapshot& operator=(const TTaskManagerStatsSnapshot& rSrc) = delete;

		void Clear();

		void AddTaskStats(const TTaskStatsSnapshotPtr& spStats);
		size_t GetTaskStatsCount() const;
		TTaskStatsSnapshotPtr GetTaskStatsAt(size_t stIndex) const;
		TTaskStatsSnapshotPtr GetTaskStatsForTaskID(taskid_t tTaskID) const;

		size_t GetRunningTasks() const { return m_stRunningTasks; }
		void SetRunningTasks(size_t stRunningTasks) { m_stRunningTasks = stRunningTasks; }

		unsigned long long GetProcessedCount() const;
		unsigned long long GetTotalCount() const;
		unsigned long long GetProcessedSize() const;
		unsigned long long GetTotalSize() const;

		double GetCountSpeed() const;
		double GetAvgCountSpeed() const;
		double GetSizeSpeed() const;
		double GetAvgSizeSpeed() const;

		double GetCombinedProgress() const;

	private:
		void CalculateProgressAndSpeeds() const;

	private:
		size_t m_stRunningTasks;

#pragma warning(push)
#pragma warning(disable: 4251)
		std::vector<TTaskStatsSnapshotPtr> m_vTasksSnapshots;
#pragma warning(pop)

		// cache for items calculated on-demand
		mutable bool m_bCacheFilled;
		mutable unsigned long long m_ullProcessedCount;
		mutable unsigned long long m_ullTotalCount;
		mutable unsigned long long m_ullProcessedSize;
		mutable unsigned long long m_ullTotalSize;

		mutable double m_dCountSpeed;
		mutable double m_dSizeSpeed;

		mutable double m_dAvgCountSpeed;
		mutable double m_dAvgSizeSpeed;

		mutable double m_dCombinedProgress;
	};

	typedef std::shared_ptr<TTaskManagerStatsSnapshot> TTaskManagerStatsSnapshotPtr;
}

#endif
