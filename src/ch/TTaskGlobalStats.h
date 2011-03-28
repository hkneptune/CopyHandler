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
/// @file  TTaskGlobalStats.h
/// @date  2011/03/28
/// @brief Contains declarations of classes responsible for maintaining task global stats.
// ============================================================================
#ifndef __TTASKGLOBALSTATS_H__
#define __TTASKGLOBALSTATS_H__

///////////////////////////////////////////////////////////////////////////
// TTasksGlobalStats
class TTasksGlobalStats
{
public:
	TTasksGlobalStats();
	~TTasksGlobalStats();

	void IncreaseGlobalTotalSize(unsigned long long ullModify);
	void DecreaseGlobalTotalSize(unsigned long long ullModify);
	unsigned long long GetGlobalTotalSize() const;

	void IncreaseGlobalProcessedSize(unsigned long long ullModify);
	void DecreaseGlobalProcessedSize(unsigned long long ullModify);
	unsigned long long GetGlobalProcessedSize() const;

	void IncreaseGlobalProgressData(unsigned long long ullTasksPosition, unsigned long long ullTasksSize);
	void DecreaseGlobalProgressData(unsigned long long ullTasksPosition, unsigned long long ullTasksSize);

	int GetProgressPercents() const;

	void IncreaseRunningTasks();
	void DecreaseRunningTasks();
	size_t GetRunningTasksCount() const;

private:
	volatile unsigned long long m_ullGlobalTotalSize;
	volatile unsigned long long m_ullGlobalProcessedSize;

	volatile size_t m_stRunningTasks;		// count of current operations
	mutable boost::shared_mutex m_lock;
};

#endif
