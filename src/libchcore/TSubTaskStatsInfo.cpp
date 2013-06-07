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
/// @file  TSubTaskStatsInfo.cpp
/// @date  2012/02/22
/// @brief Contains definition of class responsible for tracking stats for subtasks.
// ============================================================================
#include "stdafx.h"
#include "TSubTaskStatsInfo.h"
#include <boost\numeric\conversion\cast.hpp>
#include "DataBuffer.h"
#include "TSubTaskStatsSnapshot.h"
#include "TCoreException.h"
#include "ErrorCodes.h"

BEGIN_CHCORE_NAMESPACE

///////////////////////////////////////////////////////////////////////////////////
// class TSubTaskProcessingGuard
TSubTaskProcessingGuard::TSubTaskProcessingGuard(TSubTaskStatsInfo& rStats) :
	m_rStats(rStats)
{
	rStats.MarkAsRunning();
	rStats.EnableTimeTracking();
}

TSubTaskProcessingGuard::~TSubTaskProcessingGuard()
{
	m_rStats.DisableTimeTracking();
	m_rStats.MarkAsNotRunning();
}

///////////////////////////////////////////////////////////////////////////////////
// class TSubTaskStatsInfo
TSubTaskStatsInfo::TSubTaskStatsInfo() :
	m_bSubTaskIsRunning(false),
	m_ullTotalSize(0),
	m_ullProcessedSize(0),
	m_stTotalCount(0),
	m_stProcessedCount(0),
	m_iCurrentBufferIndex(0),
	m_strCurrentPath(),
	m_timeElapsed(0),
	m_timeLast(-1)
{
}

void TSubTaskStatsInfo::Clear()
{
	m_bSubTaskIsRunning = false;
	m_ullTotalSize = 0;
	m_ullProcessedSize = 0;
	m_stTotalCount = 0;
	m_stProcessedCount = 0;
	m_iCurrentBufferIndex = 0;
	m_strCurrentPath.Clear();
	m_timeElapsed = 0;
	m_timeLast = -1;
}

void TSubTaskStatsInfo::GetSnapshot(TSubTaskStatsSnapshot& rStatsSnapshot) const
{
	rStatsSnapshot.Clear();

	boost::upgrade_lock<boost::shared_mutex> lock(m_lock);
	UpdateTime(lock);

	rStatsSnapshot.SetRunning(m_bSubTaskIsRunning);
	rStatsSnapshot.SetProcessedCount(m_stProcessedCount);
	rStatsSnapshot.SetTotalCount(m_stTotalCount);
	rStatsSnapshot.SetProcessedSize(m_ullProcessedSize);
	rStatsSnapshot.SetTotalSize(m_ullTotalSize);
	rStatsSnapshot.SetProgressInPercent(CalculateProgressInPercent(lock));
	rStatsSnapshot.SetCurrentBufferIndex(m_iCurrentBufferIndex);
	rStatsSnapshot.SetCurrentPath(m_strCurrentPath);
	rStatsSnapshot.SetTimeElapsed(m_timeElapsed);
}

// is running?
void TSubTaskStatsInfo::MarkAsRunning()
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_bSubTaskIsRunning = true;
}

void TSubTaskStatsInfo::MarkAsNotRunning()
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_bSubTaskIsRunning = false;
}

void TSubTaskStatsInfo::IncreaseProcessedCount(size_t stIncreaseBy)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_stProcessedCount += stIncreaseBy;
	_ASSERTE(m_stProcessedCount <= m_stTotalCount);
	if(m_stProcessedCount > m_stTotalCount)
		THROW_CORE_EXCEPTION(eErr_InternalProblem);
}

void TSubTaskStatsInfo::SetProcessedCount(size_t stProcessedCount)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_stProcessedCount = stProcessedCount;
	_ASSERTE(m_stProcessedCount <= m_stTotalCount);
	if(m_stProcessedCount > m_stTotalCount)
		THROW_CORE_EXCEPTION(eErr_InternalProblem);
}

void TSubTaskStatsInfo::SetTotalCount(size_t stCount)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_stTotalCount = stCount;
	_ASSERTE(m_stProcessedCount <= m_stTotalCount);
	if(m_stProcessedCount > m_stTotalCount)
		THROW_CORE_EXCEPTION(eErr_InternalProblem);
}

void TSubTaskStatsInfo::IncreaseProcessedSize(unsigned long long ullIncreaseBy)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_ullProcessedSize += ullIncreaseBy;
	_ASSERTE(m_ullProcessedSize <= m_ullTotalSize);
	if(m_ullProcessedSize > m_ullTotalSize)
		THROW_CORE_EXCEPTION(eErr_InternalProblem);
}

void TSubTaskStatsInfo::SetProcessedSize(unsigned long long ullProcessedSize)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_ullProcessedSize = ullProcessedSize;
	_ASSERTE(m_ullProcessedSize <= m_ullTotalSize);
	if(m_ullProcessedSize > m_ullTotalSize)
		THROW_CORE_EXCEPTION(eErr_InternalProblem);
}

void TSubTaskStatsInfo::SetTotalSize(unsigned long long ullTotalSize)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_ullTotalSize = ullTotalSize;
	_ASSERTE(m_ullProcessedSize <= m_ullTotalSize);
	if(m_ullProcessedSize > m_ullTotalSize)
		THROW_CORE_EXCEPTION(eErr_InternalProblem);
}

// buffer index
void TSubTaskStatsInfo::SetCurrentBufferIndex(int iCurrentIndex)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_iCurrentBufferIndex = iCurrentIndex;
}
// current path
void TSubTaskStatsInfo::SetCurrentPath(const TString& strPath)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_strCurrentPath = strPath;
}

// time
void TSubTaskStatsInfo::SetTimeElapsed(time_t timeElapsed)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_timeElapsed = timeElapsed;
}

void TSubTaskStatsInfo::EnableTimeTracking()
{
	boost::upgrade_lock<boost::shared_mutex> lock(m_lock);
	if(m_timeLast == -1)
	{
		boost::upgrade_to_unique_lock<boost::shared_mutex> lock_upgraded(lock);
		m_timeLast = time(NULL);
	}
}

void TSubTaskStatsInfo::DisableTimeTracking()
{
	boost::upgrade_lock<boost::shared_mutex> lock(m_lock);

	UpdateTime(lock);

	if(m_timeLast != -1)
	{
		boost::upgrade_to_unique_lock<boost::shared_mutex> lock_upgraded(lock);
		m_timeLast = -1;
	}
}

void TSubTaskStatsInfo::UpdateTime(boost::upgrade_lock<boost::shared_mutex>& lock) const
{
	if(m_timeLast != -1)
	{
		time_t timeCurrent = time(NULL);

		boost::upgrade_to_unique_lock<boost::shared_mutex> lock_upgraded(lock);
		m_timeElapsed += timeCurrent - m_timeLast;
		m_timeLast = timeCurrent;
	}
}

double TSubTaskStatsInfo::CalculateProgressInPercent(boost::upgrade_lock<boost::shared_mutex>& lock) const
{
	lock;	// lock unused; enforced passing as parameter to ensure the code is executed in critical section
	double dSizePercent = 0;
	double dCountPercent = 0;

	if(m_ullTotalSize)
		dSizePercent = 100.0 * boost::numeric_cast<double>(m_ullProcessedSize) / boost::numeric_cast<double>(m_ullTotalSize);
	if(m_stTotalCount)
		dCountPercent = 100.0 * boost::numeric_cast<double>(m_stProcessedCount) / boost::numeric_cast<double>(m_stTotalCount);

	if(m_ullTotalSize && m_stTotalCount)
		return (dSizePercent + dCountPercent) / 2;
	else
		return dSizePercent + dCountPercent;
}

END_CHCORE_NAMESPACE
