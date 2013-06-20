// ============================================================================
//  Copyright (C) 2001-2011 by Jozef Starosczyk
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
/// @file  TTaskLocalStatsInfo.h
/// @date  2011/03/28
/// @brief Contains classes responsible for maintaining local task stats.
// ============================================================================
#ifndef __TTASKLOCALSTATS_H__
#define __TTASKLOCALSTATS_H__

#include "libchcore.h"
#include "ESubTaskTypes.h"
#include "TSubTaskStatsInfo.h"
#include "TTaskStatsSnapshot.h"

BEGIN_CHCORE_NAMESPACE

class TTaskLocalStatsInfo;
class TTaskStatsSnapshot;

class TTaskProcessingGuard
{
public:
	TTaskProcessingGuard(TTaskLocalStatsInfo& rLocalStats);
	~TTaskProcessingGuard();

	void PauseTimeTracking();
	void UnPauseTimeTracking();

	void PauseRunningState();
	void UnPauseRunningState();

private:
	TTaskProcessingGuard(const TTaskProcessingGuard& rLocalStats);
	TTaskProcessingGuard& operator=(const TTaskProcessingGuard& rLocalStats);

private:
	TTaskLocalStatsInfo& m_rLocalStats;
	bool m_bTimeTrackingPaused;
	bool m_bRunningStatePaused;
};

class TTaskLocalStatsInfo
{
public:
	TTaskLocalStatsInfo();
	~TTaskLocalStatsInfo();

	void Clear();
	void GetSnapshot(TTaskStatsSnapshotPtr& spSnapshot) const;

	void SetCurrentSubOperationType(ESubOperationType eSubOperationType);

	bool IsRunning() const;

protected:
	// running/not running state
	void MarkTaskAsRunning();
	void MarkTaskAsNotRunning();

	// time tracking
	void EnableTimeTracking();
	void DisableTimeTracking();

#pragma warning(push)
#pragma warning(disable: 4251)
	void UpdateTime(boost::upgrade_lock<boost::shared_mutex>& lock) const;
#pragma warning(pop)

	void SetTimeElapsed(unsigned long long timeElapsed);
	unsigned long long GetTimeElapsed();

private:
	TTaskLocalStatsInfo(const TTaskLocalStatsInfo&);
	TTaskLocalStatsInfo& operator=(const TTaskLocalStatsInfo&);

private:
	volatile bool m_bTaskIsRunning;

	mutable TSimpleTimer m_tTimer;

#pragma warning(push)
#pragma warning(disable: 4251)
	mutable boost::shared_mutex m_lock;
#pragma warning(pop)

	friend class TTaskProcessingGuard;
};

END_CHCORE_NAMESPACE

#endif
