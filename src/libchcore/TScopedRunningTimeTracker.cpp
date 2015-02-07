// ============================================================================
//  Copyright (C) 2001-2015 by Jozef Starosczyk
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
#include "TScopedRunningTimeTracker.h"

BEGIN_CHCORE_NAMESPACE

TScopedRunningTimeTracker::TScopedRunningTimeTracker(IRunningTimeControl& rLocalStats) :
	m_rLocalStats(rLocalStats),
	m_bTimeTrackingPaused(false),
	m_bRunningStatePaused(false)
{
	rLocalStats.EnableTimeTracking();
	rLocalStats.MarkAsRunning();
}

TScopedRunningTimeTracker::~TScopedRunningTimeTracker()
{
	m_rLocalStats.MarkAsNotRunning();
	m_rLocalStats.DisableTimeTracking();
}

void TScopedRunningTimeTracker::PauseTimeTracking()
{
	if (!m_bTimeTrackingPaused)
	{
		m_rLocalStats.DisableTimeTracking();
		m_bTimeTrackingPaused = true;
	}
}

void TScopedRunningTimeTracker::UnPauseTimeTracking()
{
	if (m_bTimeTrackingPaused)
	{
		m_rLocalStats.EnableTimeTracking();
		m_bTimeTrackingPaused = false;
	}
}

void TScopedRunningTimeTracker::PauseRunningState()
{
	if (!m_bRunningStatePaused)
	{
		m_rLocalStats.MarkAsNotRunning();
		m_bRunningStatePaused = true;
	}
}

void TScopedRunningTimeTracker::UnPauseRunningState()
{
	if (m_bRunningStatePaused)
	{
		m_rLocalStats.MarkAsRunning();
		m_bRunningStatePaused = false;
	}
}

END_CHCORE_NAMESPACE
