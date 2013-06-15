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

BEGIN_CHCORE_NAMESPACE

class TBufferSizes;

// enum representing current processing state of the task
enum ETaskCurrentState
{
	eTaskState_None,
	eTaskState_Waiting,
	eTaskState_Processing,
	eTaskState_Paused,
	eTaskState_Cancelled,
	eTaskState_Error,
	eTaskState_Finished,

	// insert new values before this one
	eTaskState_Max
};

// structure for getting status of a task
struct TASK_DISPLAY_DATA
{
	chcore::TTaskStatsSnapshot m_tTaskSnapshot;

	TString m_strFullFilePath;
	TString m_strFileName;

	int m_iCurrentBufferSize;
	int m_iCurrentBufferIndex;
	size_t m_stIndex;
	size_t m_stSize;

	TSmartPath m_pathDstPath;
	TFileFiltersArray* m_pafFilters;

	ETaskCurrentState m_eTaskState;
	EOperationType m_eOperationType;
	ESubOperationType m_eSubOperationType;

	int m_nPriority;

	ull_t m_ullProcessedSize;
	ull_t m_ullSizeAll;
	double m_dPercent;

	time_t m_timeElapsed;

	TString m_strUniqueName;	// doesn't change from first setting

	bool m_bIgnoreDirectories;
	bool m_bCreateEmptyFiles;
};

struct TASK_MINI_DISPLAY_DATA
{
	TString m_strPath;

	ETaskCurrentState m_eTaskState;

	double m_dPercent;
};

///////////////////////////////////////////////////////////////////////////
// TTask

class LIBCHCORE_API TTask
{
public:
	enum EPathType
	{
		ePathType_TaskDefinition,
		ePathType_TaskRarelyChangingState,
		ePathType_TaskOftenChangingState,
		ePathType_TaskLogFile,
	};

public:
	~TTask();

	const TTaskDefinition& GetTaskDefinition() const { return m_tTaskDefinition; }

	void SetTaskState(ETaskCurrentState eTaskState);
	ETaskCurrentState GetTaskState() const;

	// m_nBufferSize
	void SetBufferSizes(const TBufferSizes& bsSizes);
	void GetBufferSizes(TBufferSizes& bsSizes);

	// thread
	void SetPriority(int nPriority);

	void Load(const TSmartPath& strPath);
	void Store();

	void BeginProcessing();

	void PauseProcessing();		// pause
	void ResumeProcessing();	// resume
	bool RetryProcessing();		// retry
	void RestartProcessing();	// from beginning
	void CancelProcessing();	// cancel

	void GetSnapshot(TASK_DISPLAY_DATA *pData);
	void GetMiniSnapshot(TASK_MINI_DISPLAY_DATA *pData);

	void SetTaskDirectory(const TSmartPath& strDir);
	TSmartPath GetTaskDirectory() const;

	void SetForceFlag(bool bFlag = true);
	bool GetForceFlag();

	size_t GetSessionUniqueID() const { return m_stSessionUniqueID; }

	TSmartPath GetRelatedPath(EPathType ePathType);

	// Stats handling
	void GetTaskStats(TTaskStatsSnapshot& rSnapshot) const;

private:
	TTask(IFeedbackHandler* piFeedbackHandler, size_t stSessionUniqueID);

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

	// m_nStatus
	void SetStatusNL(UINT nStatus, UINT nMask);
	UINT GetStatusNL(UINT nMask = 0xffffffff);

	void DeleteProgress();

	void SetForceFlagNL(bool bFlag = true);
	bool GetForceFlagNL();

	void SetContinueFlag(bool bFlag = true);
	bool GetContinueFlag();
	void SetContinueFlagNL(bool bFlag = true);
	bool GetContinueFlagNL();

	bool CanBegin();

	void KillThread();
	void RequestStopThread();

	TSmartPath GetRelatedPathNL(EPathType ePathType);

	static void OnCfgOptionChanged(const TStringSet& rsetChanges, void* pParam);

private:
	// task initial information (needed to start a task); might be a bit processed.
	TTaskDefinition m_tTaskDefinition;

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

	TSmartPath m_strTaskDirectory;			// base path at which the files will be stored
	TSmartPath m_strFilePath;				// exact filename with path to the task definition file

	bool m_bRareStateModified;			// rarely changing state has been modified
	bool m_bOftenStateModified;			// rarely changing state has been modified

	size_t m_stSessionUniqueID;			///< Per-session unique ID for this task

	// other helpers
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
