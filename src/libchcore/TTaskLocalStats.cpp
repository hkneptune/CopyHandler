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
/// @file  TTaskLocalStats.cpp
/// @date  2011/03/28
/// @brief Contains implementation of classes responsible for maintaining local task stats.
// ============================================================================
#include "stdafx.h"
#include "TTaskLocalStats.h"
#include "TTaskGlobalStats.h"
#include <boost\numeric\conversion\cast.hpp>

BEGIN_CHCORE_NAMESPACE

////////////////////////////////////////////////////////////////////////////////
// TTasksGlobalStats members
TTaskLocalStats::TTaskLocalStats() :
m_prtGlobalStats(NULL),
m_ullProcessedSize(0),
m_ullTotalSize(0),
m_bTaskIsRunning(false),
m_timeElapsed(0),
m_timeLast(-1),
m_iCurrentBufferIndex(0)
{
}

TTaskLocalStats::~TTaskLocalStats()
{
	DisconnectGlobalStats();
}

void TTaskLocalStats::ConnectGlobalStats(TTasksGlobalStats& rtGlobalStats)
{
	DisconnectGlobalStats();

	boost::unique_lock<boost::shared_mutex> lock(m_lock);

	m_prtGlobalStats = &rtGlobalStats;
	m_prtGlobalStats->IncreaseGlobalProgressData(m_ullProcessedSize, m_ullTotalSize);
	if(m_bTaskIsRunning)
		m_prtGlobalStats->IncreaseRunningTasks();
}

void TTaskLocalStats::DisconnectGlobalStats()
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);

	if(m_prtGlobalStats)
	{
		m_prtGlobalStats->DecreaseGlobalProgressData(m_ullProcessedSize, m_ullTotalSize);
		if(m_bTaskIsRunning)
			m_prtGlobalStats->DecreaseRunningTasks();
		m_prtGlobalStats = NULL;
	}
}

void TTaskLocalStats::IncreaseProcessedSize(unsigned long long ullAdd)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);

	if(m_prtGlobalStats)
		m_prtGlobalStats->IncreaseGlobalProcessedSize(ullAdd);

	m_ullProcessedSize += ullAdd;
}

void TTaskLocalStats::DecreaseProcessedSize(unsigned long long ullSub)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	if(m_prtGlobalStats)
		m_prtGlobalStats->DecreaseGlobalProcessedSize(ullSub);

	m_ullProcessedSize -= ullSub;
}

void TTaskLocalStats::SetProcessedSize(unsigned long long ullSet)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);

	if(m_prtGlobalStats)
	{
		if(ullSet < m_ullProcessedSize)
			m_prtGlobalStats->DecreaseGlobalProcessedSize(m_ullProcessedSize - ullSet);
		else
			m_prtGlobalStats->IncreaseGlobalProcessedSize(ullSet - m_ullProcessedSize);
	}

	m_ullProcessedSize = ullSet;
}

unsigned long long TTaskLocalStats::GetProcessedSize() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_ullProcessedSize;
}

unsigned long long TTaskLocalStats::GetUnProcessedSize() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_ullTotalSize - m_ullProcessedSize;
}

void TTaskLocalStats::IncreaseTotalSize(unsigned long long ullAdd)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);

	if(m_prtGlobalStats)
		m_prtGlobalStats->IncreaseGlobalTotalSize(ullAdd);
	m_ullTotalSize += ullAdd;
}

void TTaskLocalStats::DecreaseTotalSize(unsigned long long ullSub)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);

	if(m_prtGlobalStats)
		m_prtGlobalStats->DecreaseGlobalTotalSize(ullSub);

	m_ullTotalSize -= ullSub;
}

void TTaskLocalStats::SetTotalSize(unsigned long long ullSet)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);

	if(m_prtGlobalStats)
	{
		if(ullSet < m_ullTotalSize)
			m_prtGlobalStats->DecreaseGlobalTotalSize(m_ullTotalSize - ullSet);
		else
			m_prtGlobalStats->IncreaseGlobalTotalSize(ullSet - m_ullTotalSize);
	}

	m_ullTotalSize = ullSet;
}

unsigned long long TTaskLocalStats::GetTotalSize() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_ullTotalSize;
}

int TTaskLocalStats::GetProgressInPercent() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);

	unsigned long long ullPercent = 0;

	if(m_ullTotalSize != 0)
		ullPercent = m_ullProcessedSize * 100 / m_ullTotalSize;

	return boost::numeric_cast<int>(ullPercent);
}

void TTaskLocalStats::MarkTaskAsRunning()
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	if(!m_bTaskIsRunning)
	{
		if(m_prtGlobalStats)
			m_prtGlobalStats->IncreaseRunningTasks();
		m_bTaskIsRunning = true;
	}
}

void TTaskLocalStats::MarkTaskAsNotRunning()
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	if(m_bTaskIsRunning)
	{
		if(m_prtGlobalStats)
			m_prtGlobalStats->DecreaseRunningTasks();
		m_bTaskIsRunning = false;
	}
}

bool TTaskLocalStats::IsRunning() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_bTaskIsRunning;
}

void TTaskLocalStats::SetTimeElapsed(time_t timeElapsed)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_timeElapsed = timeElapsed;
}

time_t TTaskLocalStats::GetTimeElapsed()
{
	UpdateTime();

	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_timeElapsed;
}

void TTaskLocalStats::EnableTimeTracking()
{
	boost::upgrade_lock<boost::shared_mutex> lock(m_lock);
	if(m_timeLast == -1)
	{
		boost::upgrade_to_unique_lock<boost::shared_mutex> lock_upgraded(lock);
		m_timeLast = time(NULL);
	}
}

void TTaskLocalStats::DisableTimeTracking()
{
	UpdateTime();

	boost::upgrade_lock<boost::shared_mutex> lock(m_lock);
	if(m_timeLast != -1)
	{
		boost::upgrade_to_unique_lock<boost::shared_mutex> lock_upgraded(lock);
		m_timeLast = -1;
	}
}

void TTaskLocalStats::UpdateTime()
{
	boost::upgrade_lock<boost::shared_mutex> lock(m_lock);
	if(m_timeLast != -1)
	{
		time_t timeCurrent = time(NULL);

		boost::upgrade_to_unique_lock<boost::shared_mutex> lock_upgraded(lock);
		m_timeElapsed += timeCurrent - m_timeLast;
		m_timeLast = timeCurrent;
	}
}

void TTaskLocalStats::SetCurrentBufferIndex(int iCurrentIndex)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_iCurrentBufferIndex = iCurrentIndex;
}

int TTaskLocalStats::GetCurrentBufferIndex() const
{
	// locking possibly not needed, not entirely sure now
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	int iResult = m_iCurrentBufferIndex;
	return iResult;
}

ESubOperationType TTaskLocalStats::GetCurrentSubOperationType() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_eCurrentSubOperationType;
}

void TTaskLocalStats::SetCurrentSubOperationType(ESubOperationType eSubOperationType)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_eCurrentSubOperationType = eSubOperationType;
}

END_CHCORE_NAMESPACE
