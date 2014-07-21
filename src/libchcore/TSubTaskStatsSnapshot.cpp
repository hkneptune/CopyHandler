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
/// @file  TSubTaskStatsSnapshot.cpp
/// @date  2012/2/26
/// @brief Contains class responsible for holding sub task stats.
// ============================================================================
#include "stdafx.h"
#include "TSubTaskStatsSnapshot.h"
#include "DataBuffer.h"
#include <boost/numeric/conversion/cast.hpp>
#include "MathFunctions.h"

BEGIN_CHCORE_NAMESPACE

///////////////////////////////////////////////////////////////////////////////////
// class TSubTaskStats
TSubTaskStatsSnapshot::TSubTaskStatsSnapshot() :
	m_bSubTaskIsRunning(false),
	m_ullTotalSize(0),
	m_ullProcessedSize(0),
	m_fcTotalCount(0),
	m_fcProcessedCount(0),
	m_iCurrentBufferIndex(TBufferSizes::eBuffer_Default),
	m_strCurrentPath(0),
	m_timeElapsed(0),
	m_dSizeSpeed(0),
	m_dCountSpeed(0),
	m_ullCurrentItemProcessedSize(0),
	m_ullCurrentItemTotalSize(0),
	m_eSubOperationType(eSubOperation_None),
	m_fcCurrentIndex(0)
{
}

void TSubTaskStatsSnapshot::Clear()
{
	m_bSubTaskIsRunning = false;
	m_ullTotalSize = 0;
	m_ullProcessedSize = 0;
	m_fcTotalCount = 0;
	m_fcProcessedCount = 0;
	m_iCurrentBufferIndex = TBufferSizes::eBuffer_Default;
	m_strCurrentPath = 0;
	m_timeElapsed = 0;
	m_dSizeSpeed = 0;
	m_dCountSpeed = 0;
	m_ullCurrentItemProcessedSize = 0;
	m_ullCurrentItemTotalSize = 0;
	m_eSubOperationType = eSubOperation_None;
	m_fcCurrentIndex = 0;
}

double TSubTaskStatsSnapshot::CalculateProgress() const
{
	// we're treating each of the items as 512B object to process
	// to have some balance between items' count and items' size in
	// progress information
	unsigned long long ullProcessed = 512ULL * m_fcProcessedCount + m_ullProcessedSize;
	unsigned long long ullTotal = 512ULL * m_fcTotalCount + m_ullTotalSize;

	if(ullTotal != 0)
		return Math::Div64(ullProcessed, ullTotal);
	else
		return 0.0;
}

unsigned long long TSubTaskStatsSnapshot::GetEstimatedTotalTime() const
{
	double dProgress = CalculateProgress();
	if(dProgress == 0.0)
		return std::numeric_limits<unsigned long long>::max();
	else
		return (unsigned long long)(m_timeElapsed * (1.0 / dProgress));
}

void TSubTaskStatsSnapshot::SetSizeSpeed(double dSizeSpeed)
{
	m_dSizeSpeed = dSizeSpeed;
}

void TSubTaskStatsSnapshot::SetCountSpeed(double dCountSpeed)
{
	m_dCountSpeed = dCountSpeed;
}

double TSubTaskStatsSnapshot::GetAvgSizeSpeed() const
{
	if(m_timeElapsed)
		return Math::Div64(m_ullProcessedSize, m_timeElapsed / 1000);
	else
		return 0.0;
}

double TSubTaskStatsSnapshot::GetAvgCountSpeed() const
{
	if(m_timeElapsed)
		return Math::Div64(m_fcProcessedCount, m_timeElapsed / 1000);
	else
		return 0.0;
}

double TSubTaskStatsSnapshot::GetCombinedProgress() const
{
	return CalculateProgress();
}

END_CHCORE_NAMESPACE
