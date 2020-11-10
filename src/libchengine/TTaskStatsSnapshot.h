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
/// @file  TTaskStatsSnapshot.h
/// @date  2012/02/26
/// @brief Contains class responsible for holding task stats.
// ============================================================================
#ifndef __TTASKSTATSSNAPSHOT_H__
#define __TTASKSTATSSNAPSHOT_H__

#include "TSubTaskStatsInfo.h"
#include "TSubTaskArrayStatsSnapshot.h"
#include "TFileFiltersArray.h"
#include "EOperationTypes.h"
#include "ETaskCurrentState.h"
#include "TaskID.h"
#include "FeedbackAlreadyExistsRuleList.h"
#include "FeedbackRules.h"

namespace chengine
{
	class LIBCHENGINE_API TTaskStatsSnapshot
	{
	public:
		TTaskStatsSnapshot();

		void Clear();

		// task ID
		taskid_t GetTaskID() const { return m_tTaskID; }
		void SetTaskID(taskid_t val) { m_tTaskID = val; }

		// subtasks' stats
		const TSubTaskArrayStatsSnapshot& GetSubTasksStats() const { return m_tSubTasksStats; }
		TSubTaskArrayStatsSnapshot& GetSubTasksStats() { return m_tSubTasksStats; }

		// task running
		bool IsTaskRunning() const { return m_bTaskIsRunning; }
		void SetTaskRunning(bool bRunning) { m_bTaskIsRunning = bRunning; }

		// time elapsed
		unsigned long long GetTimeElapsed() const { return m_ullTimeElapsed; }
		void SetTimeElapsed(unsigned long long ullTimeElapsed) { m_ullTimeElapsed = ullTimeElapsed; }
		unsigned long long GetEstimatedTotalTime() const;

		// speed and progress
		unsigned long long GetProcessedCount() const;
		unsigned long long GetTotalCount() const;
		unsigned long long GetProcessedSize() const;
		unsigned long long GetTotalSize() const;

		double GetCountSpeed() const;
		double GetSizeSpeed() const;
		double GetAvgCountSpeed() const;
		double GetAvgSizeSpeed() const;

		double GetCombinedProgress() const;

		// other properties
		int GetThreadPriority() const { return m_iThreadPriority; }
		void SetThreadPriority(int val) { m_iThreadPriority = val; }

		string::TString GetDestinationPath() const { return m_strDestinationPath; }
		void SetDestinationPath(const string::TString& val) { m_strDestinationPath = val; }

		const TFileFiltersArray& GetFilters() const { return m_filters; }
		void SetFilters(const TFileFiltersArray& val) { m_filters = val; }

		const FeedbackRules& GetFeedbackRules() const { return m_feedbackRules; }
		void SetFeedbackRules(const FeedbackRules& val) { m_feedbackRules = val; }

		ETaskCurrentState GetTaskState() const { return m_eTaskState; }
		void SetTaskState(ETaskCurrentState val) { m_eTaskState = val; }

		string::TString GetTaskName() const { return m_strTaskID; }
		void SetTaskName(const string::TString& val) { m_strTaskID = val; }

		EOperationType GetOperationType() const { return m_eOperationType; }
		void SetOperationType(EOperationType val) { m_eOperationType = val; }

		bool GetIgnoreDirectories() const { return m_bIgnoreDirectories; }
		void SetIgnoreDirectories(bool val) { m_bIgnoreDirectories = val; }

		bool GetCreateEmptyFiles() const { return m_bCreateEmptyFiles; }
		void SetCreateEmptyFiles(bool val) { m_bCreateEmptyFiles = val; }

		void SetCurrentBufferSize(unsigned long long ullSize) { m_ullCurrentBufferSize = ullSize; }
		unsigned long long GetCurrentBufferSize() const { return m_ullCurrentBufferSize; }

		unsigned int GetBufferCount() const { return m_uiBufferCount; }
		void SetBufferCount(unsigned int uiBufferCount) { m_uiBufferCount = uiBufferCount; }

		string::TString GetSourcePath() const;
		void SetSourcePath(const string::TString& pathSrc);

	private:
		void CalculateProgressAndSpeeds() const;

	private:
		TSubTaskArrayStatsSnapshot m_tSubTasksStats;

		taskid_t m_tTaskID;

		bool m_bTaskIsRunning;

		string::TString m_strSrcPath;
		unsigned long long m_ullTimeElapsed;

		int m_iThreadPriority;
		string::TString m_strDestinationPath;

		TFileFiltersArray m_filters;

		FeedbackRules m_feedbackRules;

		ETaskCurrentState m_eTaskState;
		string::TString m_strTaskID;
		EOperationType m_eOperationType;
		bool m_bIgnoreDirectories;
		bool m_bCreateEmptyFiles;

		unsigned long long m_ullCurrentBufferSize;
		unsigned int m_uiBufferCount;

		// cache for items calculated on-demand
		mutable bool m_bCacheFilled;
		mutable unsigned long long m_ullProcessedCount;
		mutable unsigned long long m_ullTotalCount;
		mutable unsigned long long m_ullProcessedSize;
		mutable unsigned long long m_ullTotalSize;

		mutable double m_dTaskCountSpeed;
		mutable double m_dTaskSizeSpeed;

		mutable double m_dCombinedProgress;
	};

	typedef std::shared_ptr<TTaskStatsSnapshot> TTaskStatsSnapshotPtr;
}

#endif
