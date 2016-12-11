// ============================================================================
//  Copyright (C) 2001-2013 by Jozef Starosczyk
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
#include "stdafx.h"
#include "TSubTaskArrayStatsSnapshot.h"
#include <boost/numeric/conversion/cast.hpp>

namespace chengine
{
	TSubTaskArrayStatsSnapshot::TSubTaskArrayStatsSnapshot() :
		m_oidCurrentSubtaskIndex(0)
	{
	}

	void TSubTaskArrayStatsSnapshot::Clear()
	{
		m_vSubTaskSnapshots.clear();
	}

	void TSubTaskArrayStatsSnapshot::AddSubTaskSnapshot(const TSubTaskStatsSnapshotPtr& spSnapshot)
	{
		m_vSubTaskSnapshots.push_back(spSnapshot);
	}

	TSubTaskStatsSnapshotPtr TSubTaskArrayStatsSnapshot::GetSubTaskSnapshotAt(size_t stIndex) const
	{
		if (stIndex >= m_vSubTaskSnapshots.size())
			return TSubTaskStatsSnapshotPtr();

		return m_vSubTaskSnapshots[stIndex];
	}

	TSubTaskStatsSnapshotPtr TSubTaskArrayStatsSnapshot::GetCurrentSubTaskSnapshot() const
	{
		if (m_oidCurrentSubtaskIndex >= m_vSubTaskSnapshots.size())
			return TSubTaskStatsSnapshotPtr();

		return m_vSubTaskSnapshots[boost::numeric_cast<size_t>(m_oidCurrentSubtaskIndex)];
	}

	size_t TSubTaskArrayStatsSnapshot::GetSubTaskSnapshotCount() const
	{
		return m_vSubTaskSnapshots.size();
	}
}
