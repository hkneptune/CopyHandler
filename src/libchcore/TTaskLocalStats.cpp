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
/// @file  TTaskLocalStatsInfo.cpp
/// @date  2011/03/28
/// @brief Contains implementation of classes responsible for maintaining local task stats.
// ============================================================================
#include "stdafx.h"
#include "TTaskLocalStats.h"
#include "TSubTaskStatsInfo.h"
#include <boost\numeric\conversion\cast.hpp>
#include "DataBuffer.h"
#include "TTaskStatsSnapshot.h"
#include "ISerializerContainer.h"
#include "ISerializerRowData.h"
#include "TRowData.h"

BEGIN_CHCORE_NAMESPACE

////////////////////////////////////////////////////////////////////////////////
// class TTaskProcessingGuard

TTaskProcessingGuard::TTaskProcessingGuard(TTaskLocalStatsInfo& rLocalStats) :
	m_rLocalStats(rLocalStats)
{
	rLocalStats.EnableTimeTracking();
	rLocalStats.MarkTaskAsRunning();
}

TTaskProcessingGuard::~TTaskProcessingGuard()
{
	m_rLocalStats.MarkTaskAsNotRunning();
	m_rLocalStats.DisableTimeTracking();
}

void TTaskProcessingGuard::PauseTimeTracking()
{
	if(!m_bTimeTrackingPaused)
	{
		m_rLocalStats.DisableTimeTracking();
		m_bTimeTrackingPaused = true;
	}
}

void TTaskProcessingGuard::UnPauseTimeTracking()
{
	if(m_bTimeTrackingPaused)
	{
		m_rLocalStats.EnableTimeTracking();
		m_bTimeTrackingPaused = false;
	}
}

void TTaskProcessingGuard::PauseRunningState()
{
	if(!m_bRunningStatePaused)
	{
		m_rLocalStats.MarkTaskAsNotRunning();
		m_bRunningStatePaused = true;
	}
}

void TTaskProcessingGuard::UnPauseRunningState()
{
	if(m_bRunningStatePaused)
	{
		m_rLocalStats.MarkTaskAsRunning();
		m_bRunningStatePaused = false;
	}
}

////////////////////////////////////////////////////////////////////////////////
// TTasksGlobalStats members
TTaskLocalStatsInfo::TTaskLocalStatsInfo() :
	m_tTimer(m_setModifications),
	m_bTaskIsRunning(false)
{
	m_setModifications[eMod_Added] = true;
}

TTaskLocalStatsInfo::~TTaskLocalStatsInfo()
{
}

void TTaskLocalStatsInfo::Clear()
{
	m_bTaskIsRunning = false;
	m_tTimer.Modify().Reset();
}

void TTaskLocalStatsInfo::GetSnapshot(TTaskStatsSnapshotPtr& spSnapshot) const
{
	boost::upgrade_lock<boost::shared_mutex> lock(m_lock);
	UpdateTime(lock);
	spSnapshot->SetTaskRunning(m_bTaskIsRunning);
	spSnapshot->SetTimeElapsed(m_tTimer.Get().GetTotalTime());
}

void TTaskLocalStatsInfo::MarkTaskAsRunning()
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_bTaskIsRunning = true;
}

void TTaskLocalStatsInfo::MarkTaskAsNotRunning()
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_bTaskIsRunning = false;
}

bool TTaskLocalStatsInfo::IsRunning() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_bTaskIsRunning;
}

void TTaskLocalStatsInfo::EnableTimeTracking()
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_tTimer.Modify().Start();
}

void TTaskLocalStatsInfo::DisableTimeTracking()
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_tTimer.Modify().Stop();
}

void TTaskLocalStatsInfo::UpdateTime(boost::upgrade_lock<boost::shared_mutex>& lock) const
{
	// if the timer is not running then there is no point modifying timer object
	if(m_tTimer.Get().IsRunning())
	{
		boost::upgrade_to_unique_lock<boost::shared_mutex> lock_upgraded(lock);
		m_tTimer.Modify().Tick();
	}
}

void TTaskLocalStatsInfo::Store(const ISerializerContainerPtr& spContainer) const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	InitColumns(spContainer);

	ISerializerRowDataPtr spRow;
	bool bAdded = m_setModifications[eMod_Added];
	if(m_setModifications.any())
		spRow = spContainer->GetRow(0, bAdded);
	else
		return;

	if(bAdded || m_setModifications[eMod_Timer])
	{
		*spRow % TRowData(_T("elapsed_time"), m_tTimer.Get().GetTotalTime());
		m_setModifications.reset();
	}
}

void TTaskLocalStatsInfo::Load(const ISerializerContainerPtr& spContainer)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);

	InitColumns(spContainer);
	ISerializerRowReaderPtr spRowReader = spContainer->GetRowReader();
	if(spRowReader->Next())
	{
		unsigned long long ullTime = 0;
		spRowReader->GetValue(_T("elapsed_time"), ullTime);
		m_tTimer.Modify().Init(ullTime);

		m_setModifications.reset();
	}
}

void TTaskLocalStatsInfo::InitColumns(const ISerializerContainerPtr& spContainer) const
{
	IColumnsDefinition& rColumns = spContainer->GetColumnsDefinition();
	if(rColumns.IsEmpty())
	{
		rColumns.AddColumn(_T("id"), IColumnsDefinition::eType_long);
		rColumns.AddColumn(_T("elapsed_time"), IColumnsDefinition::eType_ulonglong);
	}
}

END_CHCORE_NAMESPACE
