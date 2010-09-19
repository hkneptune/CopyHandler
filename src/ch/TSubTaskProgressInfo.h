// ============================================================================
//  Copyright (C) 2001-2010 by Jozef Starosczyk
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
/// @file  TSubTaskProgressInfo.h
/// @date  2010/09/19
/// @brief Contains declaration of class handling progress information for subtasks.
// ============================================================================
#ifndef __TSUBTASKPROGRESSINFO_H__
#define __TSUBTASKPROGRESSINFO_H__


///////////////////////////////////////////////////////////////////////////
// TSubTaskProgressInfo

class TSubTaskProgressInfo
{
public:
	TSubTaskProgressInfo();
	~TSubTaskProgressInfo();

	void GetSnapshot(TSubTaskProgressInfo& rDst) const;

	void Clear();

	// count of data
	void IncreaseProcessedCount(size_t stAdd);
	void DecreaseProcessedCount(size_t stSub);
	void SetProcessedCount(size_t stSet);
	size_t GetProcessedCount() const;

	size_t GetUnProcessedCount() const;

	void IncreaseTotalCount(size_t stAdd);
	void DecreaseTotalCount(size_t stSub);
	void SetTotalCount(size_t stSet);
	size_t GetTotalCount() const;

	double GetCountProgressInPercent() const;

	// size of data
	void IncreaseProcessedSize(unsigned long long ullAdd);
	void DecreaseProcessedSize(unsigned long long ullSub);
	void SetProcessedSize(unsigned long long ullSet);
	unsigned long long GetProcessedSize() const;

	unsigned long long GetUnProcessedSize() const;

	void IncreaseTotalSize(unsigned long long ullAdd);
	void DecreaseTotalSize(unsigned long long ullSub);
	void SetTotalSize(unsigned long long ullSet);
	unsigned long long GetTotalSize() const;

	// calculated values
	double GetSizeProgressInPercent() const;

	// time tracking
	void SetTimeElapsed(time_t timeElapsed);
	time_t GetTimeElapsed();

	void EnableTimeTracking();
	void DisableTimeTracking();
	void UpdateTime();

private:
	// count of data
	volatile size_t m_stProcessedCount;
	volatile size_t m_stTotalCount;

	// size of data
	volatile unsigned long long m_ullProcessedSize;
	volatile unsigned long long m_ullTotalSize;

	// time
	volatile time_t m_timeElapsed;		///< How much time has elapsed from the start
	volatile time_t m_timeLast;			///< Last time the time elapsed has been updated

	mutable boost::shared_mutex m_lock;
};

typedef boost::shared_ptr<TSubTaskProgressInfo> TSubTaskProgressInfoPtr;

#endif // __TSUBTASKPROGRESSINFO_H__