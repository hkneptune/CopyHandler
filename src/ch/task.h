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

#include "TAutoHandles.h"
#include "TWorkerThreadController.h"
#include "FileInfo.h"
#include "DataBuffer.h"
#include "../libchcore/FeedbackHandlerBase.h"
#include "FileFilter.h"
#include "TTaskDefinition.h"
#include "TTaskConfigTracker.h"
#include "TBasePathData.h"
#include "TSubTaskBase.h"
#include "TTaskLocalStats.h"
#include "TTaskGlobalStats.h"
#include "TBasicProgressInfo.h"

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

// special value representing no task
#define NO_TASK_SESSION_UNIQUE_ID				0

// structure for getting status of a task
struct TASK_DISPLAY_DATA
{
	CString m_strFullFilePath;
	CString m_strFileName;

	int m_iCurrentBufferSize;
	int m_iCurrentBufferIndex;
	size_t m_stIndex;
	size_t m_stSize;

	chcore::TSmartPath m_pathDstPath;
	CFiltersArray* m_pafFilters;

	ETaskCurrentState m_eTaskState;
	EOperationType m_eOperationType;
	ESubOperationType m_eSubOperationType;

	int m_nPriority;

	ull_t m_ullProcessedSize;
	ull_t m_ullSizeAll;
	int m_nPercent;

	time_t m_timeElapsed;

	CString m_strUniqueName;	// doesn't change from first setting

	bool m_bIgnoreDirectories;
	bool m_bCreateEmptyFiles;
};

struct TASK_MINI_DISPLAY_DATA
{
	CString m_strPath;

	ETaskCurrentState m_eTaskState;

	int m_nPercent;
};

///////////////////////////////////////////////////////////////////////////
// CTask

class CTask
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
	CTask(chcore::IFeedbackHandler* piFeedbackHandler, size_t stSessionUniqueID);
	~CTask();

	void SetTaskDefinition(const TTaskDefinition& rTaskDefinition);
	const TTaskDefinition& GetTaskDefinition() const { return m_tTaskDefinition; }

	void SetTaskState(ETaskCurrentState eTaskState);
	ETaskCurrentState GetTaskState() const;

	// m_nBufferSize
	void SetBufferSizes(const BUFFERSIZES& bsSizes);
	void GetBufferSizes(BUFFERSIZES& bsSizes);
	int GetCurrentBufferIndex();

	// thread
	void SetPriority(int nPriority);

	void Load(const CString& strPath);
	void Store();

	void BeginProcessing();

	void PauseProcessing();		// pause
	void ResumeProcessing();	// resume
	bool RetryProcessing();		// retry
	void RestartProcessing();	// from beginning
	void CancelProcessing();	// cancel

	void GetSnapshot(TASK_DISPLAY_DATA *pData);
	void GetMiniSnapshot(TASK_MINI_DISPLAY_DATA *pData);

	void SetTaskDirectory(const CString& strDir);
	CString GetTaskDirectory() const;

	void SetTaskFilePath(const CString& strPath);
	CString GetTaskFilePath() const;

	void SetForceFlag(bool bFlag = true);
	bool GetForceFlag();

	size_t GetSessionUniqueID() const { return m_stSessionUniqueID; }

	CString GetRelatedPath(EPathType ePathType);

protected:
	// methods are called when task is being added or removed from the global task array
	/// Method is called when this task is being added to a CTaskArray object
	void OnRegisterTask(TTasksGlobalStats& rtGlobalStats);
	/// Method is called when task is being removed from the CTaskArray object
	void OnUnregisterTask();

	/// Method is called when processing is being started
	void OnBeginOperation();
	/// Method is called when processing is being ended
	void OnEndOperation();

	// Processing operations

	/// Thread function that delegates call to the CTask::ThrdProc
	static DWORD WINAPI DelegateThreadProc(LPVOID pParam);

	/// Main function for the task processing thread
	DWORD WINAPI ThrdProc();

	TSubTaskBase::ESubOperationResult CheckForWaitState();

	// Helper filesystem methods
	bool GetRequiredFreeSpace(ull_t *pi64Needed, ull_t *pi64Available);

	TSubTaskBase::ESubOperationResult CheckForFreeSpaceFB();

	// m_nStatus
	void SetStatusNL(UINT nStatus, UINT nMask);
	UINT GetStatusNL(UINT nMask = 0xffffffff);

	void CalculateProcessedSize();
	void CalculateProcessedSizeNL();

	void CalculateTotalSizeNL();

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

	CString GetRelatedPathNL(EPathType ePathType);

	static void OnCfgOptionChanged(const std::set<std::wstring>& rsetChanges, void* pParam);

private:
	// task initial information (needed to start a task); might be a bit processed.
	TTaskDefinition m_tTaskDefinition;

	TTaskConfigTracker m_cfgTracker;

	TBasePathDataContainer m_arrSourcePathsInfo;

	// current task state (derivatives of the task initial information)
	// changing slowly or only partially
	CFileInfoArray m_files;             // list of files/directories found during operating on the task input data (filled by search for files)

	// changing fast
	volatile ETaskCurrentState m_eCurrentState;     // current state of processing this task represents

	TTaskBasicProgressInfo m_tTaskBasicProgressInfo;	// task progress information

	// task control variables (per-session state)
	TTaskLocalStats m_localStats;       // local statistics

	// task settings
	CFiltersArray m_afFilters;          // filtering settings for files (will be filtered according to the rules inside when searching for files)

	bool m_bForce;						// if the continuation of tasks should be independent of max concurrently running task limit
	bool m_bContinue;					// allows task to continue

	CString m_strTaskDirectory;			// base path at which the files will be stored
	CString m_strFilePath;				// exact filename with path to the task definition file

	bool m_bRareStateModified;			// rarely changing state has been modified
	bool m_bOftenStateModified;			// rarely changing state has been modified

	size_t m_stSessionUniqueID;			///< Per-session unique ID for this task

	// other helpers
	icpf::log_file m_log;				///< Log file where task information will be stored

	/// Thread controlling object
	TWorkerThreadController m_workerThread;

	/// Mutex for locking concurrent access to members of this class
	mutable boost::shared_mutex m_lock;

	/// Pointer to the feedback handler, providing responses to feedback requests
	chcore::IFeedbackHandler* m_piFeedbackHandler;

	friend class CTaskArray;
};

typedef boost::shared_ptr<CTask> CTaskPtr;

///////////////////////////////////////////////////////////////////////////
// CTaskArray

class CTaskArray
{
public:
	CTaskArray();
	~CTaskArray();

	void Create(chcore::IFeedbackHandlerFactory* piFeedbackHandlerFactory);

	CTaskPtr CreateTask();

	size_t GetSize() const;

	CTaskPtr GetAt(size_t stIndex) const;
	CTaskPtr GetTaskBySessionUniqueID(size_t stSessionUniqueID) const;

	size_t Add(const CTaskPtr& spNewTask);

	void RemoveAt(size_t stIndex, size_t stCount = 1);
	void RemoveAll();
	void RemoveAllFinished();
	void RemoveFinished(const CTaskPtr& spSelTask);

	void ResumeWaitingTasks(size_t stMaxRunningTasks);
	void StopAllTasks();

	void SaveData();
	void LoadDataProgress();

	void TasksBeginProcessing();
	void TasksPauseProcessing();
	void TasksResumeProcessing();
	void TasksRestartProcessing();
	bool TasksRetryProcessing();
	void TasksCancelProcessing();

	ull_t GetPosition();
	ull_t GetRange();
	int GetPercent();

	bool AreAllFinished();

	void SetTasksDir(const tchar_t* pszPath);

protected:
	void StopAllTasksNL();
	
public:
	tstring_t m_strTasksDir;

	mutable boost::shared_mutex m_lock;

private:
	std::vector<CTaskPtr> m_vTasks;		// vector with tasks objects

	TTasksGlobalStats m_globalStats;	// global stats for all tasks

	size_t m_stNextSessionUniqueID;		// global counter for providing unique ids for tasks per session (launch of the program)

protected:
	chcore::IFeedbackHandlerFactory* m_piFeedbackFactory;
};

#endif
