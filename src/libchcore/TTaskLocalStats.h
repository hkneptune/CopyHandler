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
/// @file  TTaskLocalStats.h
/// @date  2011/03/28
/// @brief Contains classes responsible for maintaining local task stats.
// ============================================================================
#ifndef __TTASKLOCALSTATS_H__
#define __TTASKLOCALSTATS_H__

#include "libchcore.h"
#include "ESubTaskTypes.h"
#include "TString.h"

BEGIN_CHCORE_NAMESPACE

class TTasksGlobalStats;

class LIBCHCORE_API TTaskLocalStats
{
public:
	TTaskLocalStats();
	~TTaskLocalStats();

	void ConnectGlobalStats(TTasksGlobalStats& rtGlobalStats);
	void DisconnectGlobalStats();

	void IncreaseProcessedSize(unsigned long long ullAdd);
	void DecreaseProcessedSize(unsigned long long ullSub);
	void SetProcessedSize(unsigned long long ullSet);
	unsigned long long GetProcessedSize() const;
	unsigned long long GetUnProcessedSize() const;

	void IncreaseTotalSize(unsigned long long ullAdd);
	void DecreaseTotalSize(unsigned long long ullSub);
	void SetTotalSize(unsigned long long ullSet);
	unsigned long long GetTotalSize() const;

	size_t GetCurrentIndex() const;
	void SetCurrentIndex(size_t stIndex);

	size_t GetTotalItems();
	void SetTotalItems(size_t stCount);

	int GetProgressInPercent() const;

	void SetCurrentPath(const TString& strPath);
	const TString& GetCurrentPath() const;

	void MarkTaskAsRunning();
	void MarkTaskAsNotRunning();
	bool IsRunning() const;

	void SetTimeElapsed(time_t timeElapsed);
	time_t GetTimeElapsed();

	void EnableTimeTracking();
	void DisableTimeTracking();
	void UpdateTime();

	void SetCurrentBufferIndex(int iCurrentIndex);
	int GetCurrentBufferIndex() const;

	ESubOperationType GetCurrentSubOperationType() const;
	void SetCurrentSubOperationType(ESubOperationType eSubOperationType);

private:
	volatile unsigned long long m_ullProcessedSize;
	volatile unsigned long long m_ullTotalSize;

	volatile size_t m_stCurrentIndex;
	volatile size_t m_stTotalItems;

	volatile bool m_bTaskIsRunning;

	// time
	volatile time_t m_timeElapsed;
	volatile time_t m_timeLast;

	volatile int m_iCurrentBufferIndex;

	volatile ESubOperationType m_eCurrentSubOperationType;

	TString m_strCurrentPath;

#pragma warning(push)
#pragma warning(disable: 4251)
	mutable boost::shared_mutex m_lock;
#pragma warning(pop)
	TTasksGlobalStats* m_prtGlobalStats;
};

END_CHCORE_NAMESPACE

#endif
