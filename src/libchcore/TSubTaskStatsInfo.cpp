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
	m_tSizeSpeed(DefaultSpeedTrackTime, DefaultSpeedSampleTime),
	m_tCountSpeed(DefaultSpeedTrackTime, DefaultSpeedSampleTime),
	m_ullCurrentItemProcessedSize(0),
	m_ullCurrentItemTotalSize(0),
	m_eSubOperationType(eSubOperation_None)
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
	m_tTimer.Reset();
	m_tSizeSpeed.Clear();
	m_tCountSpeed.Clear();
	m_ullCurrentItemProcessedSize = 0;
	m_ullCurrentItemTotalSize = 0;
	m_eSubOperationType = eSubOperation_None;
}

void TSubTaskStatsInfo::GetSnapshot(TSubTaskStatsSnapshotPtr& spStatsSnapshot) const
{
	if(!spStatsSnapshot)
		THROW_CORE_EXCEPTION(eErr_InvalidArgument);

	spStatsSnapshot->Clear();

	boost::upgrade_lock<boost::shared_mutex> lock(m_lock);
	UpdateTime(lock);

	spStatsSnapshot->SetRunning(m_bSubTaskIsRunning);
	spStatsSnapshot->SetProcessedCount(m_stProcessedCount);
	spStatsSnapshot->SetTotalCount(m_stTotalCount);
	spStatsSnapshot->SetProcessedSize(m_ullProcessedSize);
	spStatsSnapshot->SetTotalSize(m_ullTotalSize);
	spStatsSnapshot->SetCurrentBufferIndex(m_iCurrentBufferIndex);
	spStatsSnapshot->SetCurrentPath(m_strCurrentPath);
	spStatsSnapshot->SetTimeElapsed(m_tTimer.GetTotalTime());
	spStatsSnapshot->SetSizeSpeed(m_tSizeSpeed.GetSpeed());
	spStatsSnapshot->SetCountSpeed(m_tCountSpeed.GetSpeed());
	spStatsSnapshot->SetCurrentItemProcessedSize(m_ullCurrentItemProcessedSize);
	spStatsSnapshot->SetCurrentItemTotalSize(m_ullCurrentItemTotalSize);
	spStatsSnapshot->SetSubOperationType(m_eSubOperationType);
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

	m_tCountSpeed.AddSample(stIncreaseBy, m_tTimer.Tick());

	_ASSERTE(m_stProcessedCount <= m_stTotalCount);
	if(m_stProcessedCount > m_stTotalCount)
		THROW_CORE_EXCEPTION(eErr_InternalProblem);
}

void TSubTaskStatsInfo::SetProcessedCount(size_t stProcessedCount)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);

	m_tCountSpeed.AddSample(stProcessedCount - m_stProcessedCount, m_tTimer.Tick());

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

	m_tSizeSpeed.AddSample(ullIncreaseBy, m_tTimer.Tick());

	_ASSERTE(m_ullProcessedSize <= m_ullTotalSize);
	if(m_ullProcessedSize > m_ullTotalSize)
		THROW_CORE_EXCEPTION(eErr_InternalProblem);
}

void TSubTaskStatsInfo::SetProcessedSize(unsigned long long ullProcessedSize)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);

	m_tSizeSpeed.AddSample(ullProcessedSize - m_ullProcessedSize, m_tTimer.Tick());

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

// current item
void TSubTaskStatsInfo::IncreaseCurrentItemProcessedSize(unsigned long long ullIncreaseBy)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_ullCurrentItemProcessedSize += ullIncreaseBy;

	_ASSERTE(m_ullCurrentItemProcessedSize <= m_ullCurrentItemTotalSize);
	if(m_ullCurrentItemProcessedSize > m_ullCurrentItemTotalSize)
		THROW_CORE_EXCEPTION(eErr_InternalProblem);
}

void TSubTaskStatsInfo::SetCurrentItemProcessedSize(unsigned long long ullProcessedSize)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);

	m_ullCurrentItemProcessedSize = ullProcessedSize;
	_ASSERTE(m_ullCurrentItemProcessedSize <= m_ullCurrentItemTotalSize);
	if(m_ullCurrentItemProcessedSize > m_ullCurrentItemTotalSize)
		THROW_CORE_EXCEPTION(eErr_InternalProblem);
}

void TSubTaskStatsInfo::SetCurrentItemTotalSize(unsigned long long ullTotalSize)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_ullCurrentItemTotalSize = ullTotalSize;
	_ASSERTE(m_ullCurrentItemProcessedSize <= m_ullCurrentItemTotalSize);
	if(m_ullCurrentItemProcessedSize > m_ullCurrentItemTotalSize)
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
void TSubTaskStatsInfo::EnableTimeTracking()
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_tTimer.Start();
}

void TSubTaskStatsInfo::DisableTimeTracking()
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_tTimer.Stop();
}

void TSubTaskStatsInfo::UpdateTime(boost::upgrade_lock<boost::shared_mutex>& lock) const
{
	boost::upgrade_to_unique_lock<boost::shared_mutex> lock_upgraded(lock);
	m_tTimer.Tick();
	m_tSizeSpeed.AddSample(0, m_tTimer.GetLastTimestamp());
	m_tCountSpeed.AddSample(0, m_tTimer.GetLastTimestamp());
}

END_CHCORE_NAMESPACE
