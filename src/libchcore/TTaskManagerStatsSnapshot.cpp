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
/// @file  TTaskManagerStatsSnapshot.cpp
/// @date  2012/2/26
/// @brief Contains class responsible for holding task manager stats.
// ============================================================================
#include "stdafx.h"
#include "TTaskManagerStatsSnapshot.h"
#include "MathFunctions.h"

namespace chcore
{
	////////////////////////////////////////////////////////////////////////////////
	// class TTaskManagerStatsSnapshot

	TTaskManagerStatsSnapshot::TTaskManagerStatsSnapshot() :
		m_stRunningTasks(0),
		m_bCacheFilled(false),
		m_ullProcessedCount(0),
		m_ullTotalCount(0),
		m_ullProcessedSize(0),
		m_ullTotalSize(0),
		m_dCountSpeed(0.0),
		m_dSizeSpeed(0.0),
		m_dCombinedProgress(0.0),
		m_dAvgCountSpeed(0.0),
		m_dAvgSizeSpeed(0.0)
	{
	}

	void TTaskManagerStatsSnapshot::Clear()
	{
		m_stRunningTasks = 0;
		m_bCacheFilled = false;
		m_ullProcessedCount = 0;
		m_ullTotalCount = 0;
		m_ullProcessedSize = 0;
		m_ullTotalSize = 0;
		m_dCountSpeed = 0.0;
		m_dSizeSpeed = 0.0;
		m_dCombinedProgress = 0.0;
		m_dAvgCountSpeed = 0.0;
		m_dAvgSizeSpeed = 0.0;

		m_vTasksSnapshots.clear();
	}

	void TTaskManagerStatsSnapshot::AddTaskStats(const TTaskStatsSnapshotPtr& spStats)
	{
		m_vTasksSnapshots.push_back(spStats);
	}

	size_t TTaskManagerStatsSnapshot::GetTaskStatsCount() const
	{
		return m_vTasksSnapshots.size();
	}

	TTaskStatsSnapshotPtr TTaskManagerStatsSnapshot::GetTaskStatsAt(size_t stIndex) const
	{
		if (stIndex >= m_vTasksSnapshots.size())
			return TTaskStatsSnapshotPtr();

		return m_vTasksSnapshots[stIndex];
	}

	TTaskStatsSnapshotPtr TTaskManagerStatsSnapshot::GetTaskStatsForTaskID(taskid_t tTaskID) const
	{
		BOOST_FOREACH(TTaskStatsSnapshotPtr spStats, m_vTasksSnapshots)
		{
			if (spStats->GetTaskID() == tTaskID)
				return spStats;
		}

		return TTaskStatsSnapshotPtr();
	}


	void TTaskManagerStatsSnapshot::CalculateProgressAndSpeeds() const
	{
		m_bCacheFilled = false;
		m_ullProcessedCount = 0;
		m_ullTotalCount = 0;
		m_ullProcessedSize = 0;
		m_ullTotalSize = 0;
		m_dCountSpeed = 0.0;
		m_dSizeSpeed = 0.0;
		m_dCombinedProgress = 0.0;
		m_dAvgCountSpeed = 0.0;
		m_dAvgSizeSpeed = 0.0;

		BOOST_FOREACH(TTaskStatsSnapshotPtr spTaskStats, m_vTasksSnapshots)
		{
			m_ullProcessedCount += spTaskStats->GetProcessedCount();
			m_ullTotalCount += spTaskStats->GetTotalCount();

			m_ullProcessedSize += spTaskStats->GetProcessedSize();
			m_ullTotalSize += spTaskStats->GetTotalSize();

			m_dCountSpeed += spTaskStats->GetCountSpeed();
			m_dSizeSpeed += spTaskStats->GetSizeSpeed();

			m_dAvgCountSpeed += spTaskStats->GetAvgCountSpeed();
			m_dAvgSizeSpeed += spTaskStats->GetAvgSizeSpeed();
		}

		// we're treating each of the items as 512B object to process
		// to have some balance between items' count and items' size in
		// progress information
		unsigned long long ullProcessed = 512ULL * m_ullProcessedCount + m_ullProcessedSize;
		unsigned long long ullTotal = 512ULL * m_ullTotalCount + m_ullTotalSize;

		if (ullTotal != 0)
			m_dCombinedProgress = Math::Div64(ullProcessed, ullTotal);

		m_bCacheFilled = true;
	}

	unsigned long long TTaskManagerStatsSnapshot::GetProcessedCount() const
	{
		if (!m_bCacheFilled)
			CalculateProgressAndSpeeds();

		return m_ullProcessedCount;
	}

	unsigned long long TTaskManagerStatsSnapshot::GetTotalCount() const
	{
		if (!m_bCacheFilled)
			CalculateProgressAndSpeeds();

		return m_ullTotalCount;
	}

	unsigned long long TTaskManagerStatsSnapshot::GetProcessedSize() const
	{
		if (!m_bCacheFilled)
			CalculateProgressAndSpeeds();

		return m_ullProcessedSize;
	}

	unsigned long long TTaskManagerStatsSnapshot::GetTotalSize() const
	{
		if (!m_bCacheFilled)
			CalculateProgressAndSpeeds();

		return m_ullTotalSize;
	}

	double TTaskManagerStatsSnapshot::GetCountSpeed() const
	{
		if (!m_bCacheFilled)
			CalculateProgressAndSpeeds();

		return m_dCountSpeed;
	}

	double TTaskManagerStatsSnapshot::GetSizeSpeed() const
	{
		if (!m_bCacheFilled)
			CalculateProgressAndSpeeds();

		return m_dSizeSpeed;
	}

	double TTaskManagerStatsSnapshot::GetCombinedProgress() const
	{
		if (!m_bCacheFilled)
			CalculateProgressAndSpeeds();

		return m_dCombinedProgress;
	}

	double TTaskManagerStatsSnapshot::GetAvgCountSpeed() const
	{
		if (!m_bCacheFilled)
			CalculateProgressAndSpeeds();

		return m_dAvgCountSpeed;
	}

	double TTaskManagerStatsSnapshot::GetAvgSizeSpeed() const
	{
		if (!m_bCacheFilled)
			CalculateProgressAndSpeeds();

		return m_dAvgSizeSpeed;
	}
}
