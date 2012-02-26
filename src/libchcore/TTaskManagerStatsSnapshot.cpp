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
/// @file  TTaskManagerStatsSnapshot.cpp
/// @date  2012/2/26
/// @brief Contains class responsible for holding task manager stats.
// ============================================================================
#include "stdafx.h"
#include "TTaskManagerStatsSnapshot.h"

BEGIN_CHCORE_NAMESPACE


////////////////////////////////////////////////////////////////////////////////
// class TTaskManagerStatsSnapshot

TTaskManagerStatsSnapshot::TTaskManagerStatsSnapshot() :
	m_stProcessedCount(0),
	m_stTotalCount(0),
	m_ullProcessedSize(0),
	m_ullTotalSize(0),
	m_dGlobalProgressInPercent(0.0),
	m_stRunningTasks(0)
{
}

TTaskManagerStatsSnapshot::TTaskManagerStatsSnapshot(const TTaskManagerStatsSnapshot& rSrc) :
	m_stProcessedCount(rSrc.m_stProcessedCount),
	m_stTotalCount(rSrc.m_stTotalCount),
	m_ullProcessedSize(rSrc.m_ullProcessedSize),
	m_ullTotalSize(rSrc.m_ullTotalSize),
	m_dGlobalProgressInPercent(rSrc.m_dGlobalProgressInPercent),
	m_stRunningTasks(rSrc.m_stRunningTasks)
{
}

TTaskManagerStatsSnapshot& TTaskManagerStatsSnapshot::operator=(const TTaskManagerStatsSnapshot& rSrc)
{
	m_stProcessedCount = rSrc.m_stProcessedCount;
	m_stTotalCount = rSrc.m_stTotalCount;
	m_ullProcessedSize = rSrc.m_ullProcessedSize;
	m_ullTotalSize = rSrc.m_ullTotalSize;
	m_dGlobalProgressInPercent = rSrc.m_dGlobalProgressInPercent;
	m_stRunningTasks = rSrc.m_stRunningTasks;

	return *this;
}

void TTaskManagerStatsSnapshot::Clear()
{
	m_stProcessedCount = 0;
	m_stTotalCount = 0;
	m_ullProcessedSize = 0;
	m_ullTotalSize = 0;
	m_dGlobalProgressInPercent = 0.0;
	m_stRunningTasks = 0;
}

END_CHCORE_NAMESPACE
