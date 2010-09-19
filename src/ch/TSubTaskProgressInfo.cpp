// ============================================================================
//  Copyright (C) 2001-2010 by Jozef Starosczyk
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
/// @file  TSubTaskProgressInfo.cpp
/// @date  2010/09/19
/// @brief Contains implementation of class handling progress information for subtasks.
// ============================================================================
#include "stdafx.h"
#include "TSubTaskProgressInfo.h"

///////////////////////////////////////////////////////////////////////////
// TSubTaskProgressInfo
TSubTaskProgressInfo::TSubTaskProgressInfo() :
	m_stProcessedCount(0),
	m_stTotalCount(0),
	m_ullProcessedSize(0),
	m_ullTotalSize(0),
	m_timeElapsed(0),
	m_timeLast(-1)
{
}

TSubTaskProgressInfo::~TSubTaskProgressInfo()
{
}

void TSubTaskProgressInfo::GetSnapshot(TSubTaskProgressInfo& rDst) const
{
	boost::unique_lock<boost::shared_mutex> dst_lock(rDst.m_lock);
	boost::shared_lock<boost::shared_mutex> lock(m_lock);

	rDst.m_stProcessedCount = m_stProcessedCount;
	rDst.m_stTotalCount = m_stTotalCount;

	rDst.m_ullProcessedSize = m_ullProcessedSize;
	rDst.m_ullTotalSize = m_ullTotalSize;

	if(m_timeLast != -1)
		rDst.m_timeElapsed = m_timeElapsed + time(NULL) - m_timeLast;	// not storing current time to avoid writing to this object
	else
		rDst.m_timeElapsed = m_timeElapsed;

	rDst.m_timeLast = -1;
}

void TSubTaskProgressInfo::Clear()
{
	m_stProcessedCount = 0;
	m_stTotalCount = 0;
	m_ullProcessedSize = 0;
	m_ullTotalSize = 0;
	m_timeElapsed = 0;
	m_timeLast = -1;
}

// count-based progress
void TSubTaskProgressInfo::IncreaseProcessedCount(size_t stAdd)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_stProcessedCount += stAdd;
}

void TSubTaskProgressInfo::DecreaseProcessedCount(size_t stSub)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_stProcessedCount -= stSub;
}

void TSubTaskProgressInfo::SetProcessedCount(size_t stSet)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_stProcessedCount = stSet;
}

size_t TSubTaskProgressInfo::GetProcessedCount() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_stProcessedCount;
}

size_t TSubTaskProgressInfo::GetUnProcessedCount() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_stTotalCount - m_stProcessedCount;
}

void TSubTaskProgressInfo::IncreaseTotalCount(size_t stAdd)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_stTotalCount += stAdd;
}

void TSubTaskProgressInfo::DecreaseTotalCount(size_t stSub)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_stTotalCount -= stSub;
}

void TSubTaskProgressInfo::SetTotalCount(size_t stSet)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_stTotalCount = stSet;
}

size_t TSubTaskProgressInfo::GetTotalCount() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_stTotalCount;
}

double TSubTaskProgressInfo::GetCountProgressInPercent() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);

	long double dPercent = 0;

	if(m_stTotalCount != 0)
		dPercent = (long double)m_stProcessedCount / (long double)m_stTotalCount;

	return (double)dPercent;
}

// size-based progress
void TSubTaskProgressInfo::IncreaseProcessedSize(unsigned long long ullAdd)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_ullProcessedSize += ullAdd;
}

void TSubTaskProgressInfo::DecreaseProcessedSize(unsigned long long ullSub)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_ullProcessedSize -= ullSub;
}

void TSubTaskProgressInfo::SetProcessedSize(unsigned long long ullSet)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_ullProcessedSize = ullSet;
}

unsigned long long TSubTaskProgressInfo::GetProcessedSize() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_ullProcessedSize;
}

unsigned long long TSubTaskProgressInfo::GetUnProcessedSize() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_ullTotalSize - m_ullProcessedSize;
}

void TSubTaskProgressInfo::IncreaseTotalSize(unsigned long long ullAdd)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_ullTotalSize += ullAdd;
}

void TSubTaskProgressInfo::DecreaseTotalSize(unsigned long long ullSub)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_ullTotalSize -= ullSub;
}

void TSubTaskProgressInfo::SetTotalSize(unsigned long long ullSet)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_ullTotalSize = ullSet;
}

unsigned long long TSubTaskProgressInfo::GetTotalSize() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_ullTotalSize;
}

double TSubTaskProgressInfo::GetSizeProgressInPercent() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);

	long double dPercent = 0;

	if(m_ullTotalSize != 0)
		dPercent = (long double)m_ullProcessedSize / (long double)m_ullTotalSize;

	return (double)dPercent;
}

void TSubTaskProgressInfo::SetTimeElapsed(time_t timeElapsed)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_timeElapsed = timeElapsed;
}

time_t TSubTaskProgressInfo::GetTimeElapsed()
{
	UpdateTime();

	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_timeElapsed;
}

void TSubTaskProgressInfo::EnableTimeTracking()
{
	boost::upgrade_lock<boost::shared_mutex> lock(m_lock);
	if(m_timeLast == -1)
	{
		boost::upgrade_to_unique_lock<boost::shared_mutex> lock_upgraded(lock);
		m_timeLast = time(NULL);
	}
}

void TSubTaskProgressInfo::DisableTimeTracking()
{
	UpdateTime();

	boost::upgrade_lock<boost::shared_mutex> lock(m_lock);
	if(m_timeLast != -1)
	{
		boost::upgrade_to_unique_lock<boost::shared_mutex> lock_upgraded(lock);
		m_timeLast = -1;
	}
}

void TSubTaskProgressInfo::UpdateTime()
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
