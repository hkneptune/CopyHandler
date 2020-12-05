/***************************************************************************
*   Copyright (C) 2001-2008 by Jozef Starosczyk                           *
*   ixen@copyhandler.com                                                  *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU Library General Public License          *
*   (version 2) as published by the Free Software Foundation;             *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU Library General Public     *
*   License along with this program; if not, write to the                 *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/
#ifndef __TASK_H__
#define __TASK_H__

#include "TTaskDefinition.h"
#include "TTaskConfigTracker.h"
#include "TBasePathData.h"
#include "TSubTaskBase.h"
#include "TTaskLocalStats.h"
#include "TSubTaskArray.h"
#include "TSubTaskContext.h"
#include "TTaskStatsSnapshot.h"
#include "TTaskBaseData.h"
#include <mutex>
#include "IFilesystem.h"
#include "../liblogger/TLogger.h"
#include "../libchcore/TWorkerThreadController.h"
#include "FeedbackManager.h"

namespace chengine
{
	class TBufferSizes;

	///////////////////////////////////////////////////////////////////////////
	// TTask
	class TTask;
	using TTaskPtr = std::shared_ptr<TTask>;

	class LIBCHENGINE_API TTask
	{
	private:
		TTask(const TTask&) = delete;
		TTask(const serializer::ISerializerPtr& spSerializer, const IFeedbackHandlerPtr& spFeedbackHandler, const logger::TLogFileDataPtr& spLogFileData);
		TTask(const serializer::ISerializerPtr& spSerializer, const IFeedbackHandlerPtr& spFeedbackHandler, const TTaskDefinition& rTaskDefinition, const logger::TLogFileDataPtr& spLogFileData);

		TTask& operator=(const TTask&) = delete;

	public:
		~TTask();

		void SetTaskState(ETaskCurrentState eTaskState);
		ETaskCurrentState GetTaskState() const;

		bool IsRunning() const;

		// m_nBufferSize
		void SetBufferSizes(const TBufferSizes& bsSizes);
		void GetBufferSizes(TBufferSizes& bsSizes);

		chcore::TSmartPath GetLogPath() const;
		void GetLogPaths(logger::TLoggerPaths& rLogPaths) const;
		string::TString GetTaskName() const;

		// thread
		void SetPriority(int nPriority);

		static TTaskPtr Load(const serializer::ISerializerPtr& spSerializer, const IFeedbackHandlerPtr& spFeedbackHandler, const logger::TLogFileDataPtr& spLogFileData);
		void Store(bool bForce);

		void BeginProcessing();

		void PauseProcessing();		// pause
		void ResumeProcessing();	// resume
		bool RetryProcessing();		// retry
		void RestartProcessing();	// from beginning
		void CancelProcessing();	// cancel

		void GetStatsSnapshot(TTaskStatsSnapshotPtr& spSnapshot);

		void SetForceFlag(bool bFlag = true);
		bool GetForceFlag();

		void RestoreFeedbackDefaults();

	private:
		void Load();

		void SetTaskDefinition(const TTaskDefinition& rTaskDefinition);

		// methods are called when task is being added or removed from the global task array
		/// Method is called when this task is being added to a TTaskManager object
		void OnRegisterTask();
		/// Method is called when task is being removed from the TTaskManager object
		void OnUnregisterTask();

		/// Method is called when processing is being started
		void OnBeginOperation();
		/// Method is called when processing is being ended
		void OnEndOperation();

		/// Thread function that delegates call to the TTask::ThrdProc
		static DWORD WINAPI DelegateThreadProc(LPVOID pParam);

		/// Main function for the task processing thread
		DWORD WINAPI ThrdProc();

		TSubTaskBase::ESubOperationResult CheckForWaitState();

		void SetForceFlagNL(bool bFlag = true);
		bool GetForceFlagNL();

		void SetContinueFlag(bool bFlag = true);
		bool GetContinueFlag();
		void SetContinueFlagNL(bool bFlag = true);
		bool GetContinueFlagNL();

		bool CanBegin();

		void KillThread();
		void RequestStopThread();

		static void OnCfgOptionChanged(const string::TStringSet& rsetChanges, void* pParam);

		serializer::ISerializerPtr GetSerializer() const;

	private:
		// serialization
#pragma warning(push)
#pragma warning(disable: 4251)
		serializer::ISerializerPtr m_spSerializer;
		std::mutex m_mutexSerializer;

		FeedbackManagerPtr m_spFeedbackManager;
#pragma warning(pop)

		// base data
		TTaskBaseData m_tBaseData;

#pragma warning(push)
#pragma warning(disable: 4251)
		logger::TLoggerPtr m_spLog;				///< Log file where task information will be stored
		TBasePathDataContainerPtr m_spSrcPaths;
#pragma warning(pop)

		// Global task settings
		TConfig m_tConfiguration;

		TSubTaskContext m_tSubTaskContext;
		TSubTasksArray m_tSubTasksArray;

		TTaskConfigTracker m_cfgTracker;

		// current task state (derivatives of the task initial information)

		// task settings
		TFileFiltersArray m_afFilters;          // filtering settings for files (will be filtered according to the rules inside when searching for files)

		bool m_bForce;						// if the continuation of tasks should be independent of max concurrently running task limit
		bool m_bContinue;					// allows task to continue

		/// Thread controlling object
		chcore::TWorkerThreadController m_workerThread;

		/// Mutex for locking concurrent access to members of this class
#pragma warning(push)
#pragma warning(disable: 4251)
		// Local filesystem access
		IFilesystemPtr m_fsLocal;

		// local statistics
		TTaskLocalStatsInfo m_tLocalStats;

		mutable boost::shared_mutex m_lock;
#pragma warning(pop)

		friend class TTaskManager;
	};
}

#endif
