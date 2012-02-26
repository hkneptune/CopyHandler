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
/// @file  TSubTaskStatsSnapshot.h
/// @date  2012/2/26
/// @brief Contains class responsible for holding sub task stats.
// ============================================================================
#ifndef __TSUBTASKSTATSSNAPSHOT_H__
#define __TSUBTASKSTATSSNAPSHOT_H__

#include "libchcore.h"
#include "TString.h"

BEGIN_CHCORE_NAMESPACE

class LIBCHCORE_API TSubTaskStatsSnapshot
{
public:
	TSubTaskStatsSnapshot();

	TSubTaskStatsSnapshot(const TSubTaskStatsSnapshot& rSrc);
	TSubTaskStatsSnapshot& operator=(const TSubTaskStatsSnapshot& rSrc);

	void Clear();

	// is running?
	void SetRunning(bool bRunning) { m_bSubTaskIsRunning = bRunning; }
	bool IsRunning() const { return m_bSubTaskIsRunning; }

	// count stats
	void SetProcessedCount(size_t stIndex) { m_stProcessedCount = stIndex; }
	size_t GetProcessedCount() const { return m_stProcessedCount; }

	void SetTotalCount(size_t stCount) { m_stTotalCount = stCount; }
	size_t GetTotalCount() const { return m_stTotalCount; }

	// size stats
	void SetProcessedSize(unsigned long long ullProcessedSize) { m_ullProcessedSize = ullProcessedSize; }
	unsigned long long GetProcessedSize() const { return m_ullProcessedSize; }

	void SetTotalSize(unsigned long long ullTotalSize) { m_ullTotalSize = ullTotalSize; }
	unsigned long long GetTotalSize() const { return m_ullTotalSize; }

	// progress in percent
	void SetProgressInPercent(double dPercent) { m_dProgressInPercent = dPercent; }
	double GetProgressInPercent() const { return m_dProgressInPercent; }

	// buffer index
	void SetCurrentBufferIndex(int iCurrentIndex) { m_iCurrentBufferIndex = iCurrentIndex; }
	int GetCurrentBufferIndex() const { return m_iCurrentBufferIndex; }

	// current path
	void SetCurrentPath(const TString& strPath) { m_strCurrentPath = strPath; }
	const TString& GetCurrentPath() const { return m_strCurrentPath; }

	// time
	void SetTimeElapsed(time_t timeElapsed) { m_timeElapsed = timeElapsed; }
	time_t GetTimeElapsed() { return m_timeElapsed; }

private:
	bool m_bSubTaskIsRunning;

	unsigned long long m_ullTotalSize;
	unsigned long long m_ullProcessedSize;

	size_t m_stTotalCount;
	size_t m_stProcessedCount;

	double m_dProgressInPercent;

	int m_iCurrentBufferIndex;

	TString m_strCurrentPath;		// currently processed path

	time_t m_timeElapsed;
};

END_CHCORE_NAMESPACE

#endif
