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

#include "TWorkerThreadController.h"
#include "FileInfo.h"
#include "DataBuffer.h"
#include "../libchcore/FeedbackHandlerBase.h"
#include "FileFilter.h"
#include "DestPath.h"

class CDestPath;

#define ST_NULL_STATUS		0x00000000

#define ST_WRITE_MASK		0x000fffff

//------------------------------------
#define ST_STEP_MASK		0x000000ff
#define ST_SEARCHING		0x00000001
#define ST_COPYING			0x00000002
#define ST_DELETING			0x00000003

//------------------------------------
#define ST_SPECIAL_MASK		0x0000f000
// simultaneous flags
#define ST_IGNORE_DIRS		0x00001000
#define ST_IGNORE_CONTENT	0x00002000
#define ST_FORCE_DIRS		0x00004000

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

enum EOperationType
{
	eOperation_Copy,
	eOperation_Move
};

///////////////////////////////////////////////////////////////////////////
// Exceptions

#define E_KILL_REQUEST		0x00
#define E_ERROR				0x01
#define E_CANCEL			0x02
#define E_PAUSE				0x03

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

/// class encapsulates windows HANDLE, allowing automatic closing it in destructor.
class TAutoFileHandle
{
public:
	// ============================================================================
	/// TAutoFileHandle::TAutoFileHandle
	/// @date 2010/08/26
	///
	/// @brief     Constructs the TAutoFileHandle object.
	// ============================================================================
	TAutoFileHandle() :
		m_hHandle(INVALID_HANDLE_VALUE)
	{
	}

	// ============================================================================
	/// TAutoFileHandle::TAutoFileHandle
	/// @date 2010/08/26
	///
	/// @brief     Constructs the TAutoFileHandle object with specified handle.
	/// @param[in] hHandle - System handle to be managed by this class.
	// ============================================================================
	TAutoFileHandle(HANDLE hHandle) :
		m_hHandle(hHandle)
	{
	}

	// ============================================================================
	/// TAutoFileHandle::~TAutoFileHandle
	/// @date 2010/08/26
	///
	/// @brief     Destructs the TAutoFileHandle object and closes handle if not closed already.
	// ============================================================================
	~TAutoFileHandle()
	{
		VERIFY(Close());
	}

	// ============================================================================
	/// TAutoFileHandle::operator=
	/// @date 2010/08/26
	///
	/// @brief     Assignment operator.
	/// @param[in] hHandle - Handle to be assigned.
	/// @return    Reference to this object,
	// ============================================================================
	TAutoFileHandle& operator=(HANDLE hHandle)
	{
		if(m_hHandle != hHandle)
		{
			VERIFY(Close());
			m_hHandle = hHandle;
		}
		return *this;
	}

	// ============================================================================
	/// TAutoFileHandle::operator HANDLE
	/// @date 2010/08/26
	///
	/// @brief     Retrieves the system handle.
	/// @return    HANDLE value.
	// ============================================================================
	operator HANDLE()
	{
		return m_hHandle;
	}

	// ============================================================================
	/// TAutoFileHandle::Close
	/// @date 2010/08/26
	///
	/// @brief     Closes the internal handle if needed.
	/// @return    Result of the CloseHandle() function.
	// ============================================================================
	BOOL Close()
	{
		BOOL bResult = TRUE;
		if(m_hHandle != INVALID_HANDLE_VALUE)
		{
			bResult = CloseHandle(m_hHandle);
			m_hHandle = INVALID_HANDLE_VALUE;
		}

		return bResult;
	}

	// ============================================================================
	/// TAutoFileHandle::Detach
	/// @date 2010/09/12
	///
	/// @brief     Detaches the handle, so it won't be closed in destructor.
	/// @return	   Returns current handle.
	// ============================================================================
	HANDLE Detach()
	{
		HANDLE hHandle = m_hHandle;
		m_hHandle = INVALID_HANDLE_VALUE;
		return hHandle;
	}

private:
	HANDLE m_hHandle;		///< System handle
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
// CTask

class TTaskProgressInfo
{
public:
	TTaskProgressInfo();
	~TTaskProgressInfo();

	void SetCurrentIndex(size_t stIndex);	// might be unneeded when serialization is implemented
	void IncreaseCurrentIndex();
	size_t GetCurrentIndex() const;

	void SetCurrentFileProcessedSize(unsigned long long ullSize);
	unsigned long long GetCurrentFileProcessedSize() const;
	void IncreaseCurrentFileProcessedSize(unsigned long long ullSizeToAdd);

	template<class Archive>
	void load(Archive& ar, unsigned int /*uiVersion*/)
	{
		size_t stCurrentIndex = 0;
		ar >> stCurrentIndex;

		unsigned long long ullCurrentFileProcessedSize = 0;
		ar >> ullCurrentFileProcessedSize;

		boost::unique_lock<boost::shared_mutex> lock(m_lock);

		m_stCurrentIndex = stCurrentIndex;
		m_ullCurrentFileProcessedSize = ullCurrentFileProcessedSize;
	}

	template<class Archive>
	void save(Archive& ar, unsigned int /*uiVersion*/) const
	{
		m_lock.lock_shared();

		size_t stCurrentIndex = m_stCurrentIndex;
		unsigned long long ullCurrentFileProcessedSize = m_ullCurrentFileProcessedSize;
		
		m_lock.unlock_shared();

		ar << stCurrentIndex;
		ar << ullCurrentFileProcessedSize;
	}

	BOOST_SERIALIZATION_SPLIT_MEMBER();

private:
	volatile size_t m_stCurrentIndex;   // index to the m_files array stating currently processed item
	volatile unsigned long long m_ullCurrentFileProcessedSize;	// count of bytes processed for current file

	mutable boost::shared_mutex m_lock;
};

///////////////////////////////////////////////////////////////////////////
// CTask
class CTask
{
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

	// m_nStatus
	void SetStatus(UINT nStatus, UINT nMask);
	UINT GetStatus(UINT nMask = 0xffffffff);

	void SetTaskState(ETaskCurrentState eTaskState);
	ETaskCurrentState GetTaskState() const;

	void SetOperationType(EOperationType eOperationType);
	EOperationType GetOperationType() const;

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

	void RecurseDirectories();
	int ScanDirectory(CString strDirName, size_t stSrcIndex, bool bRecurse, bool bIncludeDirs);

	void ProcessFiles();
	void CustomCopyFile(CUSTOM_COPY_PARAMS* pData);

	void DeleteFiles();

	void CheckForWaitState();

	// Helper filesystem methods
	static bool SetFileDirectoryTime(LPCTSTR lpszName, const CFileInfoPtr& spFileInfo);

	bool GetRequiredFreeSpace(ull_t *pi64Needed, ull_t *pi64Available);

	HANDLE OpenSourceFileFB(const CFileInfoPtr& spSrcFileInfo, bool bNoBuffering);
	HANDLE OpenDestinationFileFB(const CString& strDstFilePath, bool bNoBuffering, const CFileInfoPtr& spSrcFileInfo, unsigned long long& ullSeekTo, bool& bFreshlyCreated);
	HANDLE OpenExistingDestinationFileFB(const CString& strDstFilePath, bool bNoBuffering);

	bool SetFilePointerFB(HANDLE hFile, long long llDistance, const CString& strFilePath);
	bool SetEndOfFileFB(HANDLE hFile, const CString& strFilePath);

	bool ReadFileFB(HANDLE hFile, CDataBuffer& rBuffer, DWORD dwToRead, DWORD& rdwBytesRead, const CString& strFilePath);
	bool WriteFileFB(HANDLE hFile, CDataBuffer& rBuffer, DWORD dwToWrite, DWORD& rdwBytesWritten, const CString& strFilePath);

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

	EOperationType m_eOperation;					// operation which is to be performed by this task object

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

	volatile UINT m_nStatus;            // what phase of the operation is this task in

	TTaskProgressInfo m_tTaskProgressInfo;	// task progress information

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
// CProcessingException

class CProcessingException
{
public:
	CProcessingException(int iType) { m_iType=iType; m_dwError=0; };
	CProcessingException(int iType, UINT uiFmtID, DWORD dwError, ...);
	CProcessingException(int iType, DWORD dwError, const tchar_t* pszDesc);

	// Implementation
public:
	int m_iType;	// kill request, error, ...

	CString m_strErrorDesc;
	DWORD m_dwError;
};

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
