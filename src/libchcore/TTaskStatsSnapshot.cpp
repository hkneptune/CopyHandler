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
/// @file  TTaskStatsSnapshot.cpp
/// @date  2012/02/26
/// @brief Contains class responsible for holding task stats.
// ============================================================================
#include "stdafx.h"
#include "TTaskStatsSnapshot.h"

BEGIN_CHCORE_NAMESPACE

////////////////////////////////////////////////////////////////////////////////
// TTaskStatsSnapshot members

TTaskStatsSnapshot::TTaskStatsSnapshot() :
	m_tCurrentSubTaskStats(),
	m_bTaskIsRunning(false),
	m_eCurrentSubOperationType(eSubOperation_None),
	m_timeElapsed(0),
	m_dTaskProgress(0.0)
{
}

TTaskStatsSnapshot::TTaskStatsSnapshot(const TTaskStatsSnapshot& rSrc) :
	m_tCurrentSubTaskStats(rSrc.m_tCurrentSubTaskStats),
	m_bTaskIsRunning(rSrc.m_bTaskIsRunning),
	m_eCurrentSubOperationType(rSrc.m_eCurrentSubOperationType),
	m_timeElapsed(rSrc.m_timeElapsed),
	m_dTaskProgress(rSrc.m_dTaskProgress)
{
}

TTaskStatsSnapshot& TTaskStatsSnapshot::operator=(const TTaskStatsSnapshot& rSrc)
{
	if(this != &rSrc)
	{
		m_tCurrentSubTaskStats = rSrc.m_tCurrentSubTaskStats;
		m_bTaskIsRunning = rSrc.m_bTaskIsRunning;
		m_eCurrentSubOperationType = rSrc.m_eCurrentSubOperationType;
		m_timeElapsed = rSrc.m_timeElapsed;
		m_dTaskProgress = rSrc.m_dTaskProgress;
	}

	return *this;
}

void TTaskStatsSnapshot::Clear()
{
	m_tCurrentSubTaskStats.Clear();
	m_bTaskIsRunning = false;
	m_eCurrentSubOperationType = eSubOperation_None;
	m_timeElapsed = 0;
	m_dTaskProgress = 0.0;
}

END_CHCORE_NAMESPACE
