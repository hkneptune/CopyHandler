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
#include "DestPath.h"

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

// enum represents type of the operation handled by the task
enum EOperationType
{
	eOperation_None,
	eOperation_Copy,
	eOperation_Move,

	// add new operation types before this enum value
	eOperation_Max
};

enum ESubOperationType
{
	eSubOperation_None,
	eSubOperation_Scanning,
	eSubOperation_Copying,
	eSubOperation_Deleting,

	// add new operation types before this one
	eSubOperation_Max
};

// special value representing no task
#define NO_TASK_SESSION_UNIQUE_ID				0

// structure for getting status of a task
struct TASK_DISPLAY_DATA
{
	CString m_strFullFilePath;
	CString m_strFileName;

	int m_iCurrentBufferIndex;
	size_t m_stIndex;
	size_t m_stSize;

	CDestPath* m_pdpDestPath;
	CFiltersArray* m_pafFilters;

	ETaskCurrentState m_eTaskState;

	const BUFFERSIZES* m_pbsSizes;
	int m_nPriority;

	ull_t m_ullProcessedSize;
	ull_t m_ullSizeAll;
	int m_nPercent;

	time_t m_timeElapsed;

	const CString* m_pstrUniqueName;	// doesn't change from first setting

	TCHAR m_szStatusText[_MAX_PATH];
};

struct TASK_MINI_DISPLAY_DATA
{
	CString m_strPath;

	ETaskCurrentState m_eTaskState;

	int m_nPercent;
};

struct CUSTOM_COPY_PARAMS
{
	CFileInfoPtr spSrcFile;		// CFileInfo - src file
	CString strDstFile;			// dest path with filename
	const CDestPath* pDestPath;

	CDataBuffer dbBuffer;		// buffer handling
	bool bOnlyCreate;			// flag from configuration - skips real copying - only create
	bool bProcessed;			// has the element been processed ? (false if skipped)
};

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

///////////////////////////////////////////////////////////////////////////
// TTaskLocalStats
class TTaskLocalStats
{
public:
	TTaskLocalStats();
	~TTaskLocalStats();

	void ConnectGlobalStats(TTasksGlobalStats& rtGlobalStats);
	void DisconnectGlobalStats();

	void IncreaseProcessedSize(unsigned long long ullAdd);
	void DecreaseProcessedSize(unsigned long long ullSub);
	void SetProcessedSize(unsigned long long ullSet);
	unsigned long long GetProcessedSize() const;
	unsigned long long GetUnProcessedSize() const;

	void IncreaseTotalSize(unsigned long long ullAdd);
	void DecreaseTotalSize(unsigned long long ullSub);
	void SetTotalSize(unsigned long long ullSet);
	unsigned long long GetTotalSize() const;

	int GetProgressInPercent() const;

	void MarkTaskAsRunning();
	void MarkTaskAsNotRunning();
	bool IsRunning() const;

	void SetTimeElapsed(time_t timeElapsed);
	time_t GetTimeElapsed();

	void EnableTimeTracking();
	void DisableTimeTracking();
	void UpdateTime();

private:
	volatile unsigned long long m_ullProcessedSize;
	volatile unsigned long long m_ullTotalSize;

	volatile bool m_bTaskIsRunning;

	// time
	volatile time_t m_timeElapsed;
	volatile time_t m_timeLast;

	mutable boost::shared_mutex m_lock;
	TTasksGlobalStats* m_prtGlobalStats;
};

///////////////////////////////////////////////////////////////////////////
// TTaskBasicProgressInfo

class TTaskBasicProgressInfo
{
public:
	TTaskBasicProgressInfo();
	~TTaskBasicProgressInfo();

	void SetCurrentIndex(size_t stIndex);	// might be unneeded when serialization is implemented
	void IncreaseCurrentIndex();
	size_t GetCurrentIndex() const;

	void SetCurrentFileProcessedSize(unsigned long long ullSize);
	unsigned long long GetCurrentFileProcessedSize() const;
	void IncreaseCurrentFileProcessedSize(unsigned long long ullSizeToAdd);

	void SetSubOperationIndex(size_t stSubOperationIndex);
	size_t GetSubOperationIndex() const;
	void IncreaseSubOperationIndex();

	template<class Archive>
	void load(Archive& ar, unsigned int /*uiVersion*/)
	{
		size_t stCurrentIndex = 0;
		ar >> stCurrentIndex;

		unsigned long long ullCurrentFileProcessedSize = 0;
		ar >> ullCurrentFileProcessedSize;

		size_t stSubOperationIndex = 0;
		ar >> stSubOperationIndex;

		boost::unique_lock<boost::shared_mutex> lock(m_lock);

		m_stCurrentIndex = stCurrentIndex;
		m_ullCurrentFileProcessedSize = ullCurrentFileProcessedSize;
		m_stSubOperationIndex = stSubOperationIndex;
	}

	template<class Archive>
	void save(Archive& ar, unsigned int /*uiVersion*/) const
	{
		m_lock.lock_shared();

		size_t stCurrentIndex = m_stCurrentIndex;
		unsigned long long ullCurrentFileProcessedSize = m_ullCurrentFileProcessedSize;
		size_t stSubOperationIndex = m_stSubOperationIndex;
		
		m_lock.unlock_shared();

		ar << stCurrentIndex;
		ar << ullCurrentFileProcessedSize;
		ar << stSubOperationIndex;
	}

	BOOST_SERIALIZATION_SPLIT_MEMBER();

private:
	volatile size_t m_stSubOperationIndex;		 // index of sub-operation from TOperationDescription
	volatile size_t m_stCurrentIndex;   // index to the m_files array stating currently processed item
	volatile unsigned long long m_ullCurrentFileProcessedSize;	// count of bytes processed for current file

	mutable boost::shared_mutex m_lock;
};

///////////////////////////////////////////////////////////////////////////
// TOperationDescription

// class describes the sub-operations to be performed
class TOperationDescription
{
public:
	TOperationDescription();
	~TOperationDescription();

	void SetOperationType(EOperationType eOperation);
	EOperationType GetOperationType() const;

	size_t GetSubOperationsCount() const;
	ESubOperationType GetSubOperationAt(size_t stIndex) const;

	template<class Archive>
	void load(Archive& ar, unsigned int /*uiVersion*/)
	{
		EOperationType eOperation = eOperation_None;
		ar >> eOperation;
		SetOperationType(eOperation);
	}

	template<class Archive>
	void save(Archive& ar, unsigned int /*uiVersion*/) const
	{
		ar << m_eOperation;
	}

	BOOST_SERIALIZATION_SPLIT_MEMBER();

private:
	EOperationType m_eOperation;
	std::vector<ESubOperationType> m_vSubOperations;

	mutable boost::shared_mutex m_lock;
};

class TTaskBasicConfiguration
{
public:
	enum EFlags
	{
		eFlag_None = 0,
		eFlag_IgnoreDirectories = 0x0001,
		eFlag_CreateEmptyFiles = 0x0002,
		eFlag_CreateOnlyDirectories = 0x0004
	};

public:
	TTaskBasicConfiguration();
	~TTaskBasicConfiguration();

	bool GetIgnoreDirectories() const;
	void SetIgnoreDirectories(bool bIgnoreDirectories);

	bool GetCreateEmptyFiles() const;
	void SetCreateEmptyFiles(bool bCreateEmptyFiles);

	bool GetCreateOnlyDirectories() const;
	void SetCreateOnlyDirectories(bool bCreateOnlyDirectories);

	template<class Archive>
	void serialize(Archive& ar, unsigned int /*uiVersion*/)
	{
		ar & m_iConfigFlags;
	}

private:
	int m_iConfigFlags;
};

///////////////////////////////////////////////////////////////////////////
// CTask

class CTask
{
protected:
	// enum using internally by the CTask class to pass the operation results between methods
	enum ESubOperationResult
	{
		eSubResult_Continue,
		eSubResult_KillRequest,
		eSubResult_Error,
		eSubResult_CancelRequest,
		eSubResult_PauseRequest
	};

public:
	CTask(chcore::IFeedbackHandler* piFeedbackHandler, size_t stSessionUniqueID);
	~CTask();

	// m_clipboard
	void AddClipboardData(const CClipboardEntryPtr& spEntry);
	CClipboardEntryPtr GetClipboardData(size_t stIndex);
	size_t GetClipboardDataSize();
	int ReplaceClipboardStrings(CString strOld, CString strNew);

	// m_strDestPath
	void SetDestPath(LPCTSTR lpszPath);
	const CDestPath& GetDestPath();

	void SetFilters(const CFiltersArray* pFilters);

	void SetTaskState(ETaskCurrentState eTaskState);
	ETaskCurrentState GetTaskState() const;

	void SetOperationType(EOperationType eOperationType);
	EOperationType GetOperationType() const;

	void SetTaskBasicConfiguration(const TTaskBasicConfiguration& TTaskBasicConfiguration);
	const TTaskBasicConfiguration& GetTaskBasicConfiguration() const;

	// m_nBufferSize
	void SetBufferSizes(const BUFFERSIZES* bsSizes);
	const BUFFERSIZES* GetBufferSizes();
	int GetCurrentBufferIndex();

	// m_pThread
	// m_nPriority
	int  GetPriority();
	void SetPriority(int nPriority);

	// m_strUniqueName
	CString GetUniqueName();

	void Load(const CString& strPath, bool bData);
	void Store(bool bData);

	void BeginProcessing();

	void PauseProcessing();		// pause
	void ResumeProcessing();	// resume
	bool RetryProcessing();		// retry
	void RestartProcessing();	// from beginning
	void CancelProcessing();	// cancel

	void GetSnapshot(TASK_DISPLAY_DATA *pData);
	void GetMiniSnapshot(TASK_MINI_DISPLAY_DATA *pData);

	CClipboardArray* GetClipboard() { return &m_clipboard; };

	void SetTaskPath(const tchar_t* pszDir);
	const tchar_t* GetTaskPath() const;

	void SetForceFlag(bool bFlag = true);
	bool GetForceFlag();

	size_t GetSessionUniqueID() const { return m_stSessionUniqueID; }

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

	ESubOperationResult RecurseDirectories();
	int ScanDirectory(CString strDirName, size_t stSrcIndex, bool bRecurse, bool bIncludeDirs);

	ESubOperationResult ProcessFiles();
	ESubOperationResult CustomCopyFile(CUSTOM_COPY_PARAMS* pData);

	ESubOperationResult DeleteFiles();

	ESubOperationResult CheckForWaitState();

	// Helper filesystem methods
	static bool SetFileDirectoryTime(LPCTSTR lpszName, const CFileInfoPtr& spFileInfo);

	bool GetRequiredFreeSpace(ull_t *pi64Needed, ull_t *pi64Available);

	ESubOperationResult OpenSourceFileFB(TAutoFileHandle& hFile, const CFileInfoPtr& spSrcFileInfo, bool bNoBuffering);
	ESubOperationResult OpenDestinationFileFB(TAutoFileHandle& hFile, const CString& strDstFilePath, bool bNoBuffering, const CFileInfoPtr& spSrcFileInfo, unsigned long long& ullSeekTo, bool& bFreshlyCreated);
	ESubOperationResult OpenExistingDestinationFileFB(TAutoFileHandle& hFile, const CString& strDstFilePath, bool bNoBuffering);

	ESubOperationResult SetFilePointerFB(HANDLE hFile, long long llDistance, const CString& strFilePath, bool& bSkip);
	ESubOperationResult SetEndOfFileFB(HANDLE hFile, const CString& strFilePath, bool& bSkip);

	ESubOperationResult ReadFileFB(HANDLE hFile, CDataBuffer& rBuffer, DWORD dwToRead, DWORD& rdwBytesRead, const CString& strFilePath, bool& bSkip);
	ESubOperationResult WriteFileFB(HANDLE hFile, CDataBuffer& rBuffer, DWORD dwToWrite, DWORD& rdwBytesWritten, const CString& strFilePath, bool& bSkip);

	ESubOperationResult CheckForFreeSpaceFB();

	int GetDestDriveNumber();

	// m_nStatus
	void SetStatusNL(UINT nStatus, UINT nMask);
	UINT GetStatusNL(UINT nMask = 0xffffffff);

	// m_nBufferSize
	void SetBufferSizesNL(const BUFFERSIZES* bsSizes);
	const BUFFERSIZES* GetBufferSizesNL();

	// m_nPriority
	int  GetPriorityNL();
	void SetPriorityNL(int nPriority);

	void CalculateProcessedSize();
	void CalculateProcessedSizeNL();

	void CalculateTotalSize();
	void CalculateTotalSizeNL();

	void DeleteProgress(LPCTSTR lpszDirectory);

	void SetForceFlagNL(bool bFlag = true);
	bool GetForceFlagNL();

	void SetContinueFlag(bool bFlag = true);
	bool GetContinueFlag();
	void SetContinueFlagNL(bool bFlag = true);
	bool GetContinueFlagNL();

	bool CanBegin();

	void KillThread();
	void RequestStopThread();

private:
	// task initial information (needed to start a task); might be a bit processed.
	CClipboardArray m_clipboard;        // original paths with which we started operation
	CDestPath m_dpDestPath;             // destination path

	// task settings
	int m_nPriority;                    // task priority (really processing thread priority)

	CString m_strUniqueName;            // name for the task (should be something like uuid)
	CFiltersArray m_afFilters;          // filtering settings for files (will be filtered according to the rules inside when searching for files)

	BUFFERSIZES m_bsSizes;              // sizes of buffers used to copy (derived from the global

	// current task state (derivatives of the task initial information)
	// changing slowly or only partially
	CFileInfoArray m_files;             // list of files/directories found during operating on the task input data (filled by search for files)

	// changing fast
	volatile ETaskCurrentState m_eCurrentState;     // current state of processing this task represents

	TOperationDescription m_tOperation;		// manages the operation and its suboperations

	TTaskBasicConfiguration m_tTaskConfig;		// task configuration options

	TTaskBasicProgressInfo m_TTaskBasicProgressInfo;	// task progress information

	// task control variables (per-session state)
	TTaskLocalStats m_localStats;       // local statistics

	bool m_bForce;						// if the continuation of tasks should be independent of max concurrently running task limit
	bool m_bContinue;					// allows task to continue

	tstring_t m_strTaskBasePath;		// base path at which the files will be stored
	bool m_bSaved;						// has the state been saved ('til next modification)

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
	void SaveProgress();
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
