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
#include "TTaskStatsSnapshot.h"
#include "ISerializerContainer.h"
#include "ISerializerRowData.h"
#include <boost/thread/locks.hpp>

namespace chcore
{
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

	void TTaskLocalStatsInfo::MarkAsRunning()
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);
		m_bTaskIsRunning = true;
	}

	void TTaskLocalStatsInfo::MarkAsNotRunning()
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
		if (m_tTimer.Get().IsRunning())
		{
			boost::upgrade_to_unique_lock<boost::shared_mutex> lock_upgraded(lock);
			m_tTimer.Modify().Tick();
		}
	}

	void TTaskLocalStatsInfo::Store(const ISerializerContainerPtr& spContainer) const
	{
		boost::shared_lock<boost::shared_mutex> lock(m_lock);
		InitColumns(spContainer);

		bool bAdded = m_setModifications[eMod_Added];
		if (m_setModifications.any())
		{
			ISerializerRowData& rRow = spContainer->GetRow(0, bAdded);
			if (bAdded || m_setModifications[eMod_Timer])
			{
				rRow.SetValue(_T("elapsed_time"), m_tTimer.Get().GetTotalTime());
				m_setModifications.reset();
			}
		}
	}

	void TTaskLocalStatsInfo::Load(const ISerializerContainerPtr& spContainer)
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);

		InitColumns(spContainer);
		ISerializerRowReaderPtr spRowReader = spContainer->GetRowReader();
		if (spRowReader->Next())
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
		if (rColumns.IsEmpty())
		{
			rColumns.AddColumn(_T("id"), ColumnType<object_id_t>::value);
			rColumns.AddColumn(_T("elapsed_time"), IColumnsDefinition::eType_ulonglong);
		}
	}
}
