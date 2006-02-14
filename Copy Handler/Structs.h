/************************************************************************
	Copy Handler 1.x - program for copying data in Microsoft Windows
						 systems.
	Copyright (C) 2001-2004 Ixen Gerthannes (copyhandler@o2.pl)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*************************************************************************/
#ifndef __STRUCTS_H__
#define __STRUCTS_H__

#include "fileinfo.h"
#include "DestPath.h"
#include "DataBuffer.h"
//#include "LogFile.h"
#include <log.h>

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


/////////////////////////////////////////////////////////////////////////////
// priority

int PriorityToIndex(int nPriority);
int IndexToPriority(int nIndex);
int IndexToPriorityClass(int iIndex);
int PriorityClassToIndex(int iPriority);

///////////////////////////////////////////////////////////////////////////
// Exceptions

#define E_KILL_REQUEST		0x00
#define E_ERROR				0x01
#define E_CANCEL			0x02

/////////////////////////////////////////////////////////////////////
// CTask

struct TASK_CREATE_DATA
{
	__int64 *pTasksProcessed;
	__int64 *pTasksAll;

	UINT *puiOperationsPending;
	LONG *plFinished;

	CCriticalSection* pcs;
	
	UINT (*pfnTaskProc)(LPVOID pParam);
};

// structure for gettings status of a task
struct TASK_DISPLAY_DATA
{
	CFileInfo m_fi;		// fi at CurrIndex
	int m_iCurrentBufferIndex;
	int m_iIndex;
	int m_iSize;

	CDestPath* m_pdpDestPath;
	CFiltersArray* m_pafFilters;
	
	UINT	m_uiStatus;
	DWORD	m_dwOsErrorCode;
	CString m_strErrorDesc;

	const BUFFERSIZES* m_pbsSizes;
	int		m_nPriority;
	
	__int64	m_iProcessedSize;
	__int64	m_iSizeAll;
	int		m_nPercent;

	long	m_lTimeElapsed;

	unsigned char m_ucCurrentCopy;
	unsigned char m_ucCopies;

	const CString* m_pstrUniqueName;	// doesn't change from first setting

	TCHAR m_szStatusText[_MAX_PATH];
};

struct TASK_MINI_DISPLAY_DATA
{
	CFileInfo m_fi;		// fi at CurrIndex

	UINT	m_uiStatus;
	int		m_nPercent;
};

class CTask
{
public:
	CTask(const TASK_CREATE_DATA *pCreateData);
	~CTask();
public:

// Attributes
public:
	// m_clipboard
	int		AddClipboardData(CClipboardEntry* pEntry);
	CClipboardEntry* GetClipboardData(int nIndex);
	int		GetClipboardDataSize();
	int		ReplaceClipboardStrings(CString strOld, CString strNew);

	// m_files
	int FilesAddDir(const CString strDirName, const CFiltersArray* pFilters, int iSrcIndex,
					   const bool bRecurse, const bool bIncludeDirs);
	int FilesAdd(CFileInfo fi);
	CFileInfo FilesGetAt(int nIndex);
	CFileInfo FilesGetAtCurrentIndex();
	void FilesRemoveAll();
	int FilesGetSize();

	// m_nCurrentIndex
	void IncreaseCurrentIndex();
	int  GetCurrentIndex();
	void SetCurrentIndex(int nIndex);

	// m_strDestPath
	void SetDestPath(LPCTSTR lpszPath);
	const CDestPath& GetDestPath();
	int GetDestDriveNumber();

	// m_nStatus
	void SetStatus(UINT nStatus, UINT nMask);
	UINT GetStatus(UINT nMask=0xffffffff);

	// m_nBufferSize
	void SetBufferSizes(const BUFFERSIZES* bsSizes);
	const BUFFERSIZES* GetBufferSizes();
	int GetCurrentBufferIndex();

	// m_pThread
	// m_nPriority
	int  GetPriority();
	void SetPriority(int nPriority);

	// m_nProcessed
	void	IncreaseProcessedSize(__int64 nSize);
	void	SetProcessedSize(__int64 nSize);
	__int64 GetProcessedSize();

	// m_nAll
	void	SetAllSize(__int64 nSize);
	__int64 GetAllSize();
	void	CalcAllSize();

	// m_pnTasksProcessed
	void	IncreaseProcessedTasksSize(__int64 nSize);
	void	DecreaseProcessedTasksSize(__int64 nSize);

	// m_pnTasksAll
	void	IncreaseAllTasksSize(__int64 nSize);
	void	DecreaseAllTasksSize(__int64 nSize);

	// m_bKill
	void SetKillFlag(bool bKill=true);
	bool GetKillFlag();

	// m_bKilled
	void SetKilledFlag(bool bKilled=true);
	bool GetKilledFlag();

	void KillThread();
	void CleanupAfterKill();

	// m_strUniqueName
	CString GetUniqueName();

	void Load(CArchive& ar, bool bData);
	void Store(LPCTSTR lpszDirectory, bool bData);
	
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
	void CalcProcessedSize();

	void DecreaseOperationsPending(UINT uiBy=1);
	void IncreaseOperationsPending(UINT uiBy=1);

	bool CanBegin();

	void UpdateTime();

	void SetFilters(const CFiltersArray* pFilters);
	const CFiltersArray* GetFilters();

	void SetCopies(unsigned char ucCopies);
	unsigned char GetCopies();
	void SetCurrentCopy(unsigned char ucCopy);
	unsigned char GetCurrentCopy();

	CClipboardArray* GetClipboard() { return &m_clipboard; };

	void SetLastProcessedIndex(int iIndex);
	int GetLastProcessedIndex();

	CString GetLogName();

	bool GetRequiredFreeSpace(__int64 *pi64Needed, __int64 *pi64Available);

// Implementation
protected:
	CClipboardArray m_clipboard;
	CFileInfoArray m_files;
	volatile int m_nCurrentIndex;
	int m_iLastProcessedIndex;
	
	CDestPath m_dpDestPath;
	
	volatile UINT m_nStatus;
	
	// info about last error
	DWORD m_lOsError;
	CString m_strErrorDesc;

	// buffers
	BUFFERSIZES m_bsSizes;

	CWinThread *m_pThread;
	int m_nPriority;

	__int64 m_nProcessed;
	__int64 m_nAll;

	__int64 *m_pnTasksProcessed;
	__int64 *m_pnTasksAll;

	bool m_bQueued;		// has operations pending for this task been increased ?
	UINT *m_puiOperationsPending;

	volatile bool m_bKill;
	volatile bool m_bKilled;

	// other stuff
	CString m_strUniqueName;

	// mask (filter)
	CFiltersArray m_afFilters;

	// copiees count
	unsigned char m_ucCopies;
	unsigned char m_ucCurrentCopy;

	bool m_bSaved;		// has the state been saved ('til next modification)

public:
	UINT m_uiResumeInterval;	// works only if the thread is off
	// time
	long m_lTimeElapsed;	// store
	long m_lLastTime;		// not store
	
	// feedback
	int m_iIdentical;
	int m_iDestinationLess;
	int m_iDestinationGreater;
	int m_iMissingInput;
	int m_iOutputError;
	int m_iMoveFile;

	// ptr to count of currently started tasks
	LONG* m_plFinished;
	bool m_bForce;		// if the continuation of tasks should be independent of limitation
	bool m_bContinue;	// used by ClipboardMonitorProc

protected:
	CCriticalSection* m_pcs;	// protects *m_pnTasksProcessed & *m_pnTasksAll from external array

	UINT (*m_pfnTaskProc)(LPVOID pParam);	// external function that processes this task
public:
	void SetForceFlag(bool bFlag=true);
	bool GetForceFlag();
	void SetContinueFlag(bool bFlag=true);
	bool GetContinueFlag();

//	CLogFile m_log;
	icpf::log_file m_log;
	CCriticalSection m_cs;	// protection for this class
};

///////////////////////////////////////////////////////////////////////////
// CTaskArray

class CTaskArray : public CArray<CTask*, CTask*>
{
public:
	CTaskArray() : CArray<CTask*, CTask*>() { m_uhRange=0; m_uhPosition=0; m_uiOperationsPending=0; m_lFinished=0; };
	~CTaskArray();

	void Create(UINT (*pfnTaskProc)(LPVOID pParam));

	int GetSize( );
	int GetUpperBound( );
	void SetSize( int nNewSize, int nGrowBy = -1 );
	
	CTask* GetAt( int nIndex );
	void SetAt( int nIndex, CTask* newElement );
	int Add( CTask* newElement );

	void RemoveAt( int nIndex, int nCount = 1 );
	void RemoveAll();
	void RemoveAllFinished();
	void RemoveFinished(CTask** pSelTask);

	void SaveData(LPCTSTR lpszDirectory);
	void SaveProgress(LPCTSTR lpszDirectory);
	void LoadDataProgress(LPCTSTR lpszDirectory);

	void TasksBeginProcessing();
	void TasksPauseProcessing();
	void TasksResumeProcessing();
	void TasksRestartProcessing();
	bool TasksRetryProcessing(bool bOnlyErrors=false, UINT uiInterval=0);
	void TasksCancelProcessing();

	__int64 GetPosition();
	__int64 GetRange();
	int		GetPercent();

	UINT GetOperationsPending();
	void SetLimitOperations(UINT uiLimit);
	UINT GetLimitOperations();

	bool IsFinished();

public:
	__int64 m_uhRange, m_uhPosition;

	UINT m_uiOperationsPending;		// count of current operations
	LONG m_lFinished;				// count of finished tasks

	CCriticalSection m_cs;
	TASK_CREATE_DATA m_tcd;
};

///////////////////////////////////////////////////////////////////////////
// CLIPBOARDMONITORDATA

struct CLIPBOARDMONITORDATA
{
	HWND m_hwnd;	// hwnd to window
	CTaskArray *m_pTasks;

	volatile bool bKill, bKilled;
};

///////////////////////////////////////////////////////////////////////////
// CProcessingException

class CProcessingException
{
public:
	CProcessingException(int iType, CTask* pTask) { m_iType=iType; m_pTask=pTask; m_dwError=0; };
	CProcessingException(int iType, CTask* pTask, UINT uiFmtID, DWORD dwError, ...);
	void Cleanup();

// Implementation
public:
	int m_iType;	// kill request, error, ...
	CTask* m_pTask;

	CString m_strErrorDesc;
	DWORD m_dwError;
};

#endif