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
	m_timeLast(0)
{
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

bool TSubTaskStatsInfo::IsRunning() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_bSubTaskIsRunning;
}

// count stats
void TSubTaskStatsInfo::GetCountStats(size_t& stProcessedCount, size_t& stTotalCount) const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	stProcessedCount = m_stProcessedCount;
	stTotalCount = m_stTotalCount;
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

// size stats
void TSubTaskStatsInfo::GetSizeStats(unsigned long long& ullProcessedSize, unsigned long long& ullTotalSize) const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	ullProcessedSize = m_ullProcessedSize;
	ullTotalSize = m_ullTotalSize;
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

int TSubTaskStatsInfo::GetCurrentBufferIndex() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_iCurrentBufferIndex;
}

// current path
void TSubTaskStatsInfo::SetCurrentPath(const TString& strPath)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_strCurrentPath = strPath;
}

const TString& TSubTaskStatsInfo::GetCurrentPath() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_strCurrentPath;
}

// time
void TSubTaskStatsInfo::SetTimeElapsed(time_t timeElapsed)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_timeElapsed = timeElapsed;
}

time_t TSubTaskStatsInfo::GetTimeElapsed()
{
	boost::upgrade_lock<boost::shared_mutex> lock(m_lock);

	UpdateTime(lock);

	return m_timeElapsed;
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

void TSubTaskStatsInfo::UpdateTime(boost::upgrade_lock<boost::shared_mutex>& lock)
{
	if(m_timeLast != -1)
	{
		time_t timeCurrent = time(NULL);

		boost::upgrade_to_unique_lock<boost::shared_mutex> lock_upgraded(lock);
		m_timeElapsed += timeCurrent - m_timeLast;
		m_timeLast = timeCurrent;
	}
}

END_CHCORE_NAMESPACE
