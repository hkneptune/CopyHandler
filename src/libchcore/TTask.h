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

#include "libchcore.h"
#include "TWorkerThreadController.h"
#include "FeedbackHandlerBase.h"
#include "TTaskDefinition.h"
#include "TTaskConfigTracker.h"
#include "TBasePathData.h"
#include "TSubTaskBase.h"
#include "TTaskLocalStats.h"
#include "..\libicpf\log.h"
#include "TLocalFilesystem.h"
#include "TFileInfoArray.h"
#include "TSubTaskArray.h"
#include "TSubTaskContext.h"
#include "TTaskStatsSnapshot.h"
#include "ISerializer.h"

BEGIN_CHCORE_NAMESPACE

class TBufferSizes;

///////////////////////////////////////////////////////////////////////////
// TTask

class LIBCHCORE_API TTask
{
private:
	TTask(const ISerializerPtr& spSerializer, IFeedbackHandler* piFeedbackHandler);

public:
	~TTask();

	void SetTaskState(ETaskCurrentState eTaskState);
	ETaskCurrentState GetTaskState() const;

	bool IsRunning() const;

	// m_nBufferSize
	void SetBufferSizes(const TBufferSizes& bsSizes);
	void GetBufferSizes(TBufferSizes& bsSizes);

	TSmartPath GetLogPath() const;

	// thread
	void SetPriority(int nPriority);

	void Load();
	void Store();

	void BeginProcessing();

	void PauseProcessing();		// pause
	void ResumeProcessing();	// resume
	bool RetryProcessing();		// retry
	void RestartProcessing();	// from beginning
	void CancelProcessing();	// cancel

	void GetStatsSnapshot(TTaskStatsSnapshotPtr& spSnapshot);

	void SetForceFlag(bool bFlag = true);
	bool GetForceFlag();

private:
	void SetTaskDefinition(const TTaskDefinition& rTaskDefinition);

	void SetLogPath(const TSmartPath& pathLog);

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

	// m_nStatus
	void SetStatusNL(UINT nStatus, UINT nMask);
	UINT GetStatusNL(UINT nMask = 0xffffffff);

	void SetForceFlagNL(bool bFlag = true);
	bool GetForceFlagNL();

	void SetContinueFlag(bool bFlag = true);
	bool GetContinueFlag();
	void SetContinueFlagNL(bool bFlag = true);
	bool GetContinueFlagNL();

	bool CanBegin();

	void KillThread();
	void RequestStopThread();

	static void OnCfgOptionChanged(const TStringSet& rsetChanges, void* pParam);

	ISerializerPtr GetSerializer() const;

private:
#pragma warning(push)
#pragma warning(disable: 4251)
	ISerializerPtr m_spSerializer;
#pragma warning(pop)

	TString m_strTaskName;

	// basic information
	TPathContainer m_vSourcePaths;
	TSmartPath m_pathDestinationPath;

	// Global task settings
	TConfig m_tConfiguration;

	TSubTasksArray m_tSubTasksArray;

	TSubTaskContext m_tSubTaskContext;

	TTaskConfigTracker m_cfgTracker;

	TBasePathDataContainer m_arrSourcePathsInfo;

	// current task state (derivatives of the task initial information)
	// changing slowly or only partially
	TFileInfoArray m_files;             // list of files/directories found during operating on the task input data (filled by search for files)

	// changing fast
	volatile ETaskCurrentState m_eCurrentState;     // current state of processing this task represents

	// task settings
	TFileFiltersArray m_afFilters;          // filtering settings for files (will be filtered according to the rules inside when searching for files)

	bool m_bForce;						// if the continuation of tasks should be independent of max concurrently running task limit
	bool m_bContinue;					// allows task to continue

	// other helpers
	TSmartPath m_pathLog;
	icpf::log_file m_log;				///< Log file where task information will be stored

	// Local filesystem access
	TLocalFilesystem m_fsLocal;

	/// Thread controlling object
	TWorkerThreadController m_workerThread;

	/// Mutex for locking concurrent access to members of this class
#pragma warning(push)
#pragma warning(disable: 4251)
	TTaskLocalStatsInfo m_tLocalStats;       // local statistics

	mutable boost::shared_mutex m_lock;
#pragma warning(pop)

	/// Pointer to the feedback handler, providing responses to feedback requests
	IFeedbackHandler* m_piFeedbackHandler;

	friend class TTaskManager;
};

typedef boost::shared_ptr<TTask> TTaskPtr;

END_CHCORE_NAMESPACE

#endif
