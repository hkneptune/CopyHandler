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
/// @file  TTaskStatsSnapshot.h
/// @date  2012/02/26
/// @brief Contains class responsible for holding task stats.
// ============================================================================
#ifndef __TTASKSTATSSNAPSHOT_H__
#define __TTASKSTATSSNAPSHOT_H__

#include "libchcore.h"
#include "TSubTaskStatsInfo.h"
#include "ESubTaskTypes.h"
#include "TSubTaskStatsSnapshot.h"

BEGIN_CHCORE_NAMESPACE

class LIBCHCORE_API TTaskStatsSnapshot
{
public:
	TTaskStatsSnapshot();
	TTaskStatsSnapshot(const TTaskStatsSnapshot& rSrc);

	TTaskStatsSnapshot& operator=(const TTaskStatsSnapshot& rSrc);

	void Clear();

	const TSubTaskStatsSnapshot& GetCurrentSubTaskStats() const { return m_tCurrentSubTaskStats; }
	TSubTaskStatsSnapshot& GetCurrentSubTaskStats() { return m_tCurrentSubTaskStats; }

	bool IsTaskRunning() const { return m_bTaskIsRunning; }
	void SetIsTaskIsRunning(bool bRunning) { m_bTaskIsRunning = bRunning; }

	ESubOperationType GetCurrentSubOperationType() const { return m_eCurrentSubOperationType; }
	void SetCurrentSubOperationType(ESubOperationType eSubTaskType) { m_eCurrentSubOperationType = eSubTaskType; }

	time_t GetTimeElapsed() const { return m_timeElapsed; }
	void SetTimeElapsed(time_t timeElapsed) { m_timeElapsed = timeElapsed; }

	double GetTaskProgressInPercent() const { return m_dTaskProgress; }
	void SetTaskProgressInPercent(double dProgress) { m_dTaskProgress = dProgress; }

private:
	TSubTaskStatsSnapshot m_tCurrentSubTaskStats;

	double m_dTaskProgress;

	bool m_bTaskIsRunning;
	ESubOperationType m_eCurrentSubOperationType;
	time_t m_timeElapsed;
};

END_CHCORE_NAMESPACE

#endif
