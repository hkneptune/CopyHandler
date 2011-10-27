// ============================================================================
//  Copyright (C) 2001-2009 by Jozef Starosczyk
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
/// @file  TTaskGlobalStats.cpp
/// @date  2011/03/28
/// @brief Contains declarations of classes responsible for maintaining task global stats.
// ============================================================================
#include "stdafx.h"
#include "TTaskGlobalStats.h"
#include <boost\numeric\conversion\cast.hpp>

BEGIN_CHCORE_NAMESPACE

////////////////////////////////////////////////////////////////////////////////
// TTasksGlobalStats members

TTasksGlobalStats::TTasksGlobalStats() :
m_ullGlobalTotalSize(0),
m_ullGlobalProcessedSize(0),
m_stRunningTasks(0)
{
}

TTasksGlobalStats::~TTasksGlobalStats()
{
}

void TTasksGlobalStats::IncreaseGlobalTotalSize(unsigned long long ullModify)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_ullGlobalTotalSize += ullModify;
}

void TTasksGlobalStats::DecreaseGlobalTotalSize(unsigned long long ullModify)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_ullGlobalTotalSize -= ullModify;
}

unsigned long long TTasksGlobalStats::GetGlobalTotalSize() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_ullGlobalTotalSize;
}

void TTasksGlobalStats::IncreaseGlobalProcessedSize(unsigned long long ullModify)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_ullGlobalProcessedSize += ullModify;
}

void TTasksGlobalStats::DecreaseGlobalProcessedSize(unsigned long long ullModify)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_ullGlobalProcessedSize -= ullModify;
}

unsigned long long TTasksGlobalStats::GetGlobalProcessedSize() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_ullGlobalProcessedSize;
}

void TTasksGlobalStats::IncreaseGlobalProgressData(unsigned long long ullTasksPosition, unsigned long long ullTasksSize)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_ullGlobalTotalSize += ullTasksSize;
	m_ullGlobalProcessedSize += ullTasksPosition;
}

void TTasksGlobalStats::DecreaseGlobalProgressData(unsigned long long ullTasksPosition, unsigned long long ullTasksSize)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_ullGlobalTotalSize -= ullTasksSize;
	m_ullGlobalProcessedSize -= ullTasksPosition;
}

int TTasksGlobalStats::GetProgressPercents() const
{
	unsigned long long llPercent = 0;

	boost::shared_lock<boost::shared_mutex> lock(m_lock);

	if(m_ullGlobalTotalSize != 0)
		llPercent = m_ullGlobalProcessedSize * 100 / m_ullGlobalTotalSize;

	return boost::numeric_cast<int>(llPercent);
}

void TTasksGlobalStats::IncreaseRunningTasks()
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	++m_stRunningTasks;
}

void TTasksGlobalStats::DecreaseRunningTasks()
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	--m_stRunningTasks;
}

size_t TTasksGlobalStats::GetRunningTasksCount() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_stRunningTasks;
}

END_CHCORE_NAMESPACE
