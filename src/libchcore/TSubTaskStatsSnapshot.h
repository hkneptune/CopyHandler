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
#include "ESubTaskTypes.h"
#include "CommonDataTypes.h"

namespace chcore
{
	class LIBCHCORE_API TSubTaskStatsSnapshot
	{
	public:
		TSubTaskStatsSnapshot();

		void Clear();

		// is running?
		void SetRunning(bool bRunning) { m_bSubTaskIsRunning = bRunning; }
		bool IsRunning() const { return m_bSubTaskIsRunning; }

		// count stats
		void SetProcessedCount(file_count_t fcIndex) { m_fcProcessedCount = fcIndex; }
		file_count_t GetProcessedCount() const { return m_fcProcessedCount; }

		void SetTotalCount(file_count_t fcCount) { m_fcTotalCount = fcCount; }
		file_count_t GetTotalCount() const { return m_fcTotalCount; }

		// size stats
		void SetProcessedSize(unsigned long long ullProcessedSize) { m_ullProcessedSize = ullProcessedSize; }
		unsigned long long GetProcessedSize() const { return m_ullProcessedSize; }

		void SetTotalSize(unsigned long long ullTotalSize) { m_ullTotalSize = ullTotalSize; }
		unsigned long long GetTotalSize() const { return m_ullTotalSize; }

		// current file
		void SetCurrentItemProcessedSize(unsigned long long ullProcessedSize) { m_ullCurrentItemProcessedSize = ullProcessedSize; }
		unsigned long long GetCurrentItemProcessedSize() const { return m_ullCurrentItemProcessedSize; }

		void SetCurrentItemTotalSize(unsigned long long ullTotalSize) { m_ullCurrentItemTotalSize = ullTotalSize; }
		unsigned long long GetCurrentItemTotalSize() const { return m_ullCurrentItemTotalSize; }

		void SetCurrentIndex(file_count_t fcCurrentIndex) { m_fcCurrentIndex = fcCurrentIndex; }
		file_count_t GetCurrentIndex() const { return m_fcCurrentIndex; }

		// progress in percent
		double GetCombinedProgress() const;	// returns progress [0.0, 1.0]

		// buffer index
		void SetCurrentBufferIndex(int iCurrentIndex) { m_iCurrentBufferIndex = iCurrentIndex; }
		int GetCurrentBufferIndex() const { return m_iCurrentBufferIndex; }

		// current path
		void SetCurrentPath(const TString& strPath) { m_strCurrentPath = strPath; }
		const TString& GetCurrentPath() const { return m_strCurrentPath; }

		// time
		void SetTimeElapsed(unsigned long long timeElapsed) { m_timeElapsed = timeElapsed; }
		unsigned long long GetTimeElapsed() { return m_timeElapsed; }

		// time estimations
		unsigned long long GetEstimatedTotalTime() const;

		// speed
		void SetSizeSpeed(double dSizeSpeed);
		double GetSizeSpeed() const { return m_dSizeSpeed; }
		void SetCountSpeed(double dCountSpeed);
		double GetCountSpeed() const { return m_dCountSpeed; }

		double GetAvgSizeSpeed() const;
		double GetAvgCountSpeed() const;

		ESubOperationType GetSubOperationType() const { return m_eSubOperationType; }
		void SetSubOperationType(ESubOperationType val) { m_eSubOperationType = val; }

	private:
		double CalculateProgress() const;

	private:
		bool m_bSubTaskIsRunning;

		// subtask size and size speed per second
		unsigned long long m_ullTotalSize;
		unsigned long long m_ullProcessedSize;
		double m_dSizeSpeed;

		// subtask count of items and its speed per second
		file_count_t m_fcTotalCount;
		file_count_t m_fcProcessedCount;
		double m_dCountSpeed;

		// current item size
		unsigned long long m_ullCurrentItemTotalSize;
		unsigned long long m_ullCurrentItemProcessedSize;
		file_count_t m_fcCurrentIndex;

		ESubOperationType m_eSubOperationType;

		int m_iCurrentBufferIndex;

		TString m_strCurrentPath;		// currently processed path

		unsigned long long m_timeElapsed;			// time really elapsed for the subtask
	};

	typedef std::shared_ptr<TSubTaskStatsSnapshot> TSubTaskStatsSnapshotPtr;
}

#endif
