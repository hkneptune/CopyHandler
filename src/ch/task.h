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
#define ST_FINISHED			0x00000004
#define ST_CANCELLED		0x00000005

//------------------------------------
#define ST_OPERATION_MASK	0x00000f00
#define ST_COPY				0x00000100
// moving - delete after copying all files
#define ST_MOVE				0x00000200

//------------------------------------
#define ST_SPECIAL_MASK		0x0000f000
// simultaneous flags
#define ST_IGNORE_DIRS		0x00001000
#define ST_IGNORE_CONTENT	0x00002000
#define ST_FORCE_DIRS		0x00004000

//------------------------------------
#define ST_WORKING_MASK		0x000f0000
#define ST_ERROR			0x000C0000
#define ST_PAUSED			0x00080000

//------------------------------------
#define ST_WAITING_MASK		0x00f00000
#define ST_WAITING			0x00100000

///////////////////////////////////////////////////////////////////////////
// Exceptions

#define E_KILL_REQUEST		0x00
#define E_ERROR				0x01
#define E_CANCEL			0x02
#define E_PAUSE				0x03

// structure for getting status of a task
struct TASK_DISPLAY_DATA
{
   CString m_strFullFilePath;
   CString m_strFileName;
	//CFileInfoPtr m_spFileInfo;		// fi at CurrIndex
	int m_iCurrentBufferIndex;
	size_t m_stIndex;
	size_t m_stSize;

	CDestPath* m_pdpDestPath;
	CFiltersArray* m_pafFilters;

	UINT	m_uiStatus;
	DWORD	m_dwOsErrorCode;
	CString m_strErrorDesc;

	const BUFFERSIZES* m_pbsSizes;
	int		m_nPriority;

	ull_t	m_ullProcessedSize;
	ull_t	m_ullSizeAll;
	int		m_nPercent;

	long	m_lTimeElapsed;

	unsigned char m_ucCurrentCopy;
	unsigned char m_ucCopies;

	const CString* m_pstrUniqueName;	// doesn't change from first setting

	TCHAR m_szStatusText[_MAX_PATH];
};

struct TASK_MINI_DISPLAY_DATA
{
   CString m_strPath;
//	CFileInfoPtr m_spFileInfo;		// fi at CurrIndex

	UINT m_uiStatus;
	int m_nPercent;
};

struct CUSTOM_COPY_PARAMS
{
	CFileInfoPtr spSrcFile;	// CFileInfo - src file
	CString strDstFile;			// dest path with filename

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
		_ASSERTE(m_hHandle == INVALID_HANDLE_VALUE);
		Close();
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

	void IncreaseGlobalTasksSize(unsigned long long ullModify);
	void DecreaseGlobalTasksSize(unsigned long long ullModify);
	unsigned long long GetGlobalTasksSize() const;

	void IncreaseGlobalTasksPosition(unsigned long long ullModify);
	void DecreaseGlobalTasksPosition(unsigned long long ullModify);
	unsigned long long GetGlobalTasksPosition() const;

	void IncreaseGlobalProgressData(unsigned long long ullTasksPosition, unsigned long long ullTasksSize);
	void DecreaseGlobalProgressData(unsigned long long ullTasksPosition, unsigned long long ullTasksSize);

	int GetProgressPercents() const;

	void IncreaseRunningTasks();
	void DecreaseRunningTasks();
	size_t GetRunningTasksCount() const;

private:
	volatile unsigned long long m_ullGlobalTasksSize;
	volatile unsigned long long m_ullGlobalTasksPosition;

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

private:
	volatile unsigned long long m_ullProcessedSize;
	volatile unsigned long long m_ullTotalSize;

	volatile bool m_bTaskIsRunning;

	mutable boost::shared_mutex m_lock;
	TTasksGlobalStats* m_prtGlobalStats;
};

///////////////////////////////////////////////////////////////////////////
// CTask
class CTask
{
public:
	CTask(chcore::IFeedbackHandler* piFeedbackHandler, size_t stSessionUniqueID);
	~CTask();

	// methods are called when task is being added or removed from the global task array
	void OnRegisterTask(TTasksGlobalStats& rtGlobalStats);
	void OnUnregisterTask();

	// m_clipboard
	void AddClipboardData(const CClipboardEntryPtr& spEntry);
	CClipboardEntryPtr GetClipboardData(size_t stIndex);
	size_t GetClipboardDataSize();
	int ReplaceClipboardStrings(CString strOld, CString strNew);

	// m_files
	int FilesAddDir(CString strDirName, size_t stSrcIndex, bool bRecurse, bool bIncludeDirs);
	void FilesAdd(const CFileInfoPtr& spFileInfo);
	CFileInfoPtr FilesGetAt(size_t stIndex);
	CFileInfoPtr FilesGetAtCurrentIndex();
	void FilesRemoveAll();
	size_t FilesGetSize();

	// m_stCurrentIndex
	void IncreaseCurrentIndex();
	size_t GetCurrentIndex();
	void SetCurrentIndex(size_t stIndex);

	// m_strDestPath
	void SetDestPath(LPCTSTR lpszPath);
	const CDestPath& GetDestPath();
	int GetDestDriveNumber();

	// m_nStatus
	void SetStatus(UINT nStatus, UINT nMask);
	UINT GetStatus(UINT nMask = 0xffffffff);

	// m_nBufferSize
	void SetBufferSizes(const BUFFERSIZES* bsSizes);
	const BUFFERSIZES* GetBufferSizes();
	int GetCurrentBufferIndex();

	// m_pThread
	// m_nPriority
	int  GetPriority();
	void SetPriority(int nPriority);

	void CalculateTotalSize();

	// m_bKill
	void SetKillFlag(bool bKill = true);
	bool GetKillFlag();

	// m_bKilled
	void SetKilledFlag(bool bKilled = true);
	bool GetKilledFlag();

	void KillThread();
	void CleanupAfterKill();

	// m_strUniqueName
	CString GetUniqueName();

	void Load(const CString& strPath, bool bData);
	void Store(bool bData);

	void BeginProcessing();

	void PauseProcessing();		// pause
	void ResumeProcessing();	// resume
	bool RetryProcessing(bool bOnlyErrors=false, UINT uiInterval=0);		// retry
	void RestartProcessing();	// from beginning
	void CancelProcessing();	// cancel

	void GetSnapshot(TASK_DISPLAY_DATA *pData);
	void GetMiniSnapshot(TASK_MINI_DISPLAY_DATA *pData);

	void DeleteProgress(LPCTSTR lpszDirectory);

	void SetOsErrorCode(DWORD dwError, LPCTSTR lpszErrDesc);
	void CalculateProcessedSize();

	bool CanBegin();

	void UpdateTime();

	void SetFilters(const CFiltersArray* pFilters);

	void SetCopies(unsigned char ucCopies);
	unsigned char GetCopies();
	void SetCurrentCopy(unsigned char ucCopy);
	unsigned char GetCurrentCopy();

	CClipboardArray* GetClipboard() { return &m_clipboard; };

	bool GetRequiredFreeSpace(ull_t *pi64Needed, ull_t *pi64Available);

	void SetTaskPath(const tchar_t* pszDir);
	const tchar_t* GetTaskPath() const;

	chcore::IFeedbackHandler* GetFeedbackHandler() const { return m_piFeedbackHandler; }

	void SetForceFlag(bool bFlag = true);
	bool GetForceFlag();
	void SetContinueFlag(bool bFlag = true);
	bool GetContinueFlag();

	size_t GetSessionUniqueID() const { return m_stSessionUniqueID; }

protected:
	static UINT ThrdProc(LPVOID pParam);

   void OnBeginOperation();
   void OnEndOperation();

	void CheckForWaitState();
	void ProcessFiles();
	void CustomCopyFile(CUSTOM_COPY_PARAMS* pData);
	void DeleteFiles();
	void RecurseDirectories();
	static bool SetFileDirectoryTime(LPCTSTR lpszName, const CFileInfoPtr& spFileInfo);

	void IncreaseCurrentIndexNL();
	size_t GetCurrentIndexNL();
	void SetCurrentIndexNL(size_t stIndex);

	// m_strDestPath
	void SetDestPathNL(LPCTSTR lpszPath);
	const CDestPath& GetDestPathNL();
	int GetDestDriveNumberNL();

	// m_nStatus
	void SetStatusNL(UINT nStatus, UINT nMask);
	UINT GetStatusNL(UINT nMask = 0xffffffff);

	// m_nBufferSize
	void SetBufferSizesNL(const BUFFERSIZES* bsSizes);
	const BUFFERSIZES* GetBufferSizesNL();
	int GetCurrentBufferIndexNL();

	// m_pThread
	// m_nPriority
	int  GetPriorityNL();
	void SetPriorityNL(int nPriority);

	void CalculateTotalSizeNL();

	void SetKillFlagNL(bool bKill = true);
	bool GetKillFlagNL();

	void SetKilledFlagNL(bool bKilled = true);
	bool GetKilledFlagNL();

	void CleanupAfterKillNL();
	void UpdateTimeNL();

	CString GetUniqueNameNL();

	void SetForceFlagNL(bool bFlag = true);
	bool GetForceFlagNL();
	void SetContinueFlagNL(bool bFlag = true);
	bool GetContinueFlagNL();

private:
	icpf::log_file m_log;
	mutable boost::shared_mutex m_lock;	// protection for this class

	UINT m_uiResumeInterval;	// works only if the thread is off

	// time
	long m_lTimeElapsed;	// store
	long m_lLastTime;		// not store

	// feedback
	chcore::IFeedbackHandler* m_piFeedbackHandler;

	// ptr to count of currently started tasks
	bool m_bForce;		// if the continuation of tasks should be independent of limitation
	bool m_bContinue;	// used by ClipboardMonitorProc

	CClipboardArray m_clipboard;
	CFileInfoArray m_files;

	volatile size_t m_stCurrentIndex;

	CDestPath m_dpDestPath;

	volatile UINT m_nStatus;

	// info about last error
	DWORD m_lOsError;
	CString m_strErrorDesc;

	// buffers
	BUFFERSIZES m_bsSizes;

	CWinThread *m_pThread;
	int m_nPriority;

	volatile bool m_bKill;
	volatile bool m_bKilled;

	// other stuff
	CString m_strUniqueName;

	// mask (filter)
	CFiltersArray m_afFilters;

	// copies count
	unsigned char m_ucCopies;
	unsigned char m_ucCurrentCopy;

	tstring_t m_strTaskBasePath;	// base path at which the files will be stored
	bool m_bSaved;		// has the state been saved ('til next modification)

	size_t m_stSessionUniqueID;

	bool m_bRegisteredAsRunning;
	TTaskLocalStats m_localStats;

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
	bool TasksRetryProcessing(bool bOnlyErrors=false, UINT uiInterval=0);
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
