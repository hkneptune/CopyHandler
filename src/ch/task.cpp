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
#include "Stdafx.h"
#include "task.h"
#include "StringHelpers.h"
#include "../common/FileSupport.h"
#include "ch.h"
#include "FeedbackHandler.h"
#include <boost/serialization/serialization.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/make_shared.hpp>
#include <fstream>

// assume max sectors of 4kB (for rounding)
#define MAXSECTORSIZE			4096

///////////////////////////////////////////////////////////////////////
// CProcessingException

CProcessingException::CProcessingException(int iType, UINT uiFmtID, DWORD dwError, ...)
{
	// std values
	m_iType=iType;
	m_dwError=dwError;

	// format some text
	CString strFormat = GetResManager().LoadString(uiFmtID);
	ExpandFormatString(&strFormat, dwError);

	// get param list
	va_list marker;
	va_start(marker, dwError);
	m_strErrorDesc.FormatV(strFormat, marker);
	va_end(marker);
}

CProcessingException::CProcessingException(int iType, DWORD dwError, const tchar_t* pszDesc)
{
	// std values
	m_iType=iType;
	m_dwError=dwError;

	// format some text
	m_strErrorDesc = pszDesc;
}

////////////////////////////////////////////////////////////////////////////////
// TTasksGlobalStats members

TTasksGlobalStats::TTasksGlobalStats() :
m_ullGlobalTotalSize(0),
m_ullGlobalProcessedSize(0),
m_stRunningTasks(0)
{
}

TTasksGlobalStats::~TTasksGlobalStats()
{
}

void TTasksGlobalStats::IncreaseGlobalTotalSize(unsigned long long ullModify)
{
	m_lock.lock();
	m_ullGlobalTotalSize += ullModify;
	m_lock.unlock();
}

void TTasksGlobalStats::DecreaseGlobalTotalSize(unsigned long long ullModify)
{
	m_lock.lock();
	m_ullGlobalTotalSize -= ullModify;
	m_lock.unlock();
}

unsigned long long TTasksGlobalStats::GetGlobalTotalSize() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_ullGlobalTotalSize;
}

void TTasksGlobalStats::IncreaseGlobalProcessedSize(unsigned long long ullModify)
{
	m_lock.lock();
	m_ullGlobalProcessedSize += ullModify;
	m_lock.unlock();
}

void TTasksGlobalStats::DecreaseGlobalProcessedSize(unsigned long long ullModify)
{
	m_lock.lock();
	m_ullGlobalProcessedSize -= ullModify;
	m_lock.unlock();
}

unsigned long long TTasksGlobalStats::GetGlobalProcessedSize() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_ullGlobalProcessedSize;
}

void TTasksGlobalStats::IncreaseGlobalProgressData(unsigned long long ullTasksPosition, unsigned long long ullTasksSize)
{
	m_lock.lock();
	m_ullGlobalTotalSize += ullTasksSize;
	m_ullGlobalProcessedSize += ullTasksPosition;
	m_lock.unlock();

}

void TTasksGlobalStats::DecreaseGlobalProgressData(unsigned long long ullTasksPosition, unsigned long long ullTasksSize)
{
	m_lock.lock();
	m_ullGlobalTotalSize -= ullTasksSize;
	m_ullGlobalProcessedSize -= ullTasksPosition;
	m_lock.unlock();
}

int TTasksGlobalStats::GetProgressPercents() const
{
	unsigned long long llPercent = 0;

	boost::shared_lock<boost::shared_mutex> lock(m_lock);

	if(m_ullGlobalTotalSize != 0)
		llPercent = m_ullGlobalProcessedSize * 100 / m_ullGlobalTotalSize;

	return boost::numeric_cast<int>(llPercent);
}

void TTasksGlobalStats::IncreaseRunningTasks()
{
	m_lock.lock();
	++m_stRunningTasks;
	m_lock.unlock();
}

void TTasksGlobalStats::DecreaseRunningTasks()
{
	m_lock.lock();
	--m_stRunningTasks;
	m_lock.unlock();
}

size_t TTasksGlobalStats::GetRunningTasksCount() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_stRunningTasks;
}

////////////////////////////////////////////////////////////////////////////////
// TTasksGlobalStats members
TTaskLocalStats::TTaskLocalStats() :
	m_prtGlobalStats(NULL),
	m_ullProcessedSize(0),
	m_ullTotalSize(0),
	m_bTaskIsRunning(false),
	m_timeElapsed(0),
	m_timeLast(-1)
{
}

TTaskLocalStats::~TTaskLocalStats()
{
	DisconnectGlobalStats();
}

void TTaskLocalStats::ConnectGlobalStats(TTasksGlobalStats& rtGlobalStats)
{
	DisconnectGlobalStats();

	boost::unique_lock<boost::shared_mutex> lock(m_lock);

	m_prtGlobalStats = &rtGlobalStats;
	m_prtGlobalStats->IncreaseGlobalProgressData(m_ullProcessedSize, m_ullTotalSize);
	if(m_bTaskIsRunning)
		m_prtGlobalStats->IncreaseRunningTasks();
}

void TTaskLocalStats::DisconnectGlobalStats()
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	if(m_prtGlobalStats)
	{
		m_prtGlobalStats->DecreaseGlobalProgressData(m_ullProcessedSize, m_ullTotalSize);
		m_prtGlobalStats = NULL;
		if(m_bTaskIsRunning)
			m_prtGlobalStats->DecreaseRunningTasks();
	}
}

void TTaskLocalStats::IncreaseProcessedSize(unsigned long long ullAdd)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);

	if(m_prtGlobalStats)
		m_prtGlobalStats->IncreaseGlobalProcessedSize(ullAdd);

	m_ullProcessedSize += ullAdd;
}

void TTaskLocalStats::DecreaseProcessedSize(unsigned long long ullSub)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	if(m_prtGlobalStats)
		m_prtGlobalStats->DecreaseGlobalProcessedSize(ullSub);

	m_ullProcessedSize -= ullSub;
}

void TTaskLocalStats::SetProcessedSize(unsigned long long ullSet)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);

	if(m_prtGlobalStats)
	{
		if(ullSet < m_ullProcessedSize)
			m_prtGlobalStats->DecreaseGlobalProcessedSize(m_ullProcessedSize - ullSet);
		else
			m_prtGlobalStats->IncreaseGlobalProcessedSize(ullSet - m_ullProcessedSize);
	}

	m_ullProcessedSize = ullSet;
}

unsigned long long TTaskLocalStats::GetProcessedSize() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_ullProcessedSize;
}

unsigned long long TTaskLocalStats::GetUnProcessedSize() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_ullTotalSize - m_ullProcessedSize;
}

void TTaskLocalStats::IncreaseTotalSize(unsigned long long ullAdd)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);

	if(m_prtGlobalStats)
		m_prtGlobalStats->IncreaseGlobalTotalSize(ullAdd);
	m_ullTotalSize += ullAdd;
}

void TTaskLocalStats::DecreaseTotalSize(unsigned long long ullSub)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);

	if(m_prtGlobalStats)
		m_prtGlobalStats->DecreaseGlobalTotalSize(ullSub);

	m_ullTotalSize -= ullSub;
}

void TTaskLocalStats::SetTotalSize(unsigned long long ullSet)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);

	if(m_prtGlobalStats)
	{
		if(ullSet < m_ullTotalSize)
			m_prtGlobalStats->DecreaseGlobalTotalSize(m_ullTotalSize - ullSet);
		else
			m_prtGlobalStats->IncreaseGlobalTotalSize(ullSet - m_ullTotalSize);
	}

	m_ullTotalSize = ullSet;
}

unsigned long long TTaskLocalStats::GetTotalSize() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_ullTotalSize;
}

int TTaskLocalStats::GetProgressInPercent() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);

	unsigned long long ullPercent = 0;

	if(m_ullTotalSize != 0)
		ullPercent = m_ullProcessedSize * 100 / m_ullTotalSize;

	return boost::numeric_cast<int>(ullPercent);
}

void TTaskLocalStats::MarkTaskAsRunning()
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	if(!m_bTaskIsRunning)
	{
		if(m_prtGlobalStats)
			m_prtGlobalStats->IncreaseRunningTasks();
		m_bTaskIsRunning = true;
	}
}

void TTaskLocalStats::MarkTaskAsNotRunning()
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	if(m_bTaskIsRunning)
	{
		if(m_prtGlobalStats)
			m_prtGlobalStats->DecreaseRunningTasks();
		m_bTaskIsRunning = false;
	}
}

bool TTaskLocalStats::IsRunning() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_bTaskIsRunning;
}

void TTaskLocalStats::SetTimeElapsed(time_t timeElapsed)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_timeElapsed = timeElapsed;
}

time_t TTaskLocalStats::GetTimeElapsed()
{
	UpdateTime();

	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_timeElapsed;
}

void TTaskLocalStats::EnableTimeTracking()
{
	boost::upgrade_lock<boost::shared_mutex> lock(m_lock);
	if(m_timeLast == -1)
	{
		boost::upgrade_to_unique_lock<boost::shared_mutex> lock_upgraded(lock);
		m_timeLast = time(NULL);
	}
}

void TTaskLocalStats::DisableTimeTracking()
{
	UpdateTime();

	boost::upgrade_lock<boost::shared_mutex> lock(m_lock);
	if(m_timeLast != -1)
	{
		boost::upgrade_to_unique_lock<boost::shared_mutex> lock_upgraded(lock);
		m_timeLast = -1;
	}
}

void TTaskLocalStats::UpdateTime()
{
	boost::upgrade_lock<boost::shared_mutex> lock(m_lock);
	if(m_timeLast != -1)
	{
		time_t timeCurrent = time(NULL);

		boost::upgrade_to_unique_lock<boost::shared_mutex> lock_upgraded(lock);
		m_timeElapsed += timeCurrent - m_timeLast;
		m_timeLast = timeCurrent;
	}
}

TTaskProgressInfo::TTaskProgressInfo() :
	m_stCurrentIndex(0),
	m_ullCurrentFileProcessedSize(0)
{
}

TTaskProgressInfo::~TTaskProgressInfo()
{
}

void TTaskProgressInfo::SetCurrentIndex(size_t stIndex)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_stCurrentIndex = stIndex;
	m_ullCurrentFileProcessedSize = 0;
}

void TTaskProgressInfo::IncreaseCurrentIndex()
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	++m_stCurrentIndex;
	m_ullCurrentFileProcessedSize = 0;
}

size_t TTaskProgressInfo::GetCurrentIndex() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_stCurrentIndex;
}

void TTaskProgressInfo::SetCurrentFileProcessedSize(unsigned long long ullSize)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_ullCurrentFileProcessedSize = ullSize;
}

unsigned long long TTaskProgressInfo::GetCurrentFileProcessedSize() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_ullCurrentFileProcessedSize;
}

void TTaskProgressInfo::IncreaseCurrentFileProcessedSize(unsigned long long ullSizeToAdd)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_ullCurrentFileProcessedSize += ullSizeToAdd;
}

////////////////////////////////////////////////////////////////////////////
// CTask members
CTask::CTask(chcore::IFeedbackHandler* piFeedbackHandler, size_t stSessionUniqueID) :
	m_log(),
	m_piFeedbackHandler(piFeedbackHandler),
	m_files(m_clipboard),
	m_nStatus(ST_NULL_STATUS),
	m_nPriority(THREAD_PRIORITY_NORMAL),
	m_bForce(false),
	m_bContinue(false),
	m_bSaved(false),
	m_stSessionUniqueID(stSessionUniqueID),
	m_localStats()
{
	BOOST_ASSERT(piFeedbackHandler);

	m_bsSizes.m_uiDefaultSize=65536;
	m_bsSizes.m_uiOneDiskSize=4194304;
	m_bsSizes.m_uiTwoDisksSize=262144;
	m_bsSizes.m_uiCDSize=262144;
	m_bsSizes.m_uiLANSize=65536;

	_itot((int)time(NULL), m_strUniqueName.GetBufferSetLength(16), 10);
	m_strUniqueName.ReleaseBuffer();
}

CTask::~CTask()
{
	KillThread();
	if(m_piFeedbackHandler)
		m_piFeedbackHandler->Delete();
}

void CTask::OnRegisterTask(TTasksGlobalStats& rtGlobalStats)
{
	m_localStats.ConnectGlobalStats(rtGlobalStats);
}

void CTask::OnUnregisterTask()
{
	m_localStats.DisconnectGlobalStats();
}

// m_clipboard
void CTask::AddClipboardData(const CClipboardEntryPtr& spEntry)
{
	m_clipboard.Add(spEntry);
}

CClipboardEntryPtr CTask::GetClipboardData(size_t stIndex)
{
	return m_clipboard.GetAt(stIndex);
}

size_t CTask::GetClipboardDataSize()
{
	return m_clipboard.GetSize();
}

int CTask::ReplaceClipboardStrings(CString strOld, CString strNew)
{
	return m_clipboard.ReplacePathsPrefix(strOld, strNew);
}

// m_files
int CTask::ScanDirectory(CString strDirName, size_t stSrcIndex, bool bRecurse, bool bIncludeDirs)
{
	WIN32_FIND_DATA wfd;
	CString strText;

	// append '\\' at the end of path if needed
	if(strDirName.Right(1) != _T("\\"))
		strDirName += _T("\\");

	strText = strDirName + _T("*");

	// Iterate through dirs & files
	HANDLE hFind = FindFirstFile(strText, &wfd);
	if(hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if(!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				CFileInfoPtr spFileInfo(boost::make_shared<CFileInfo>());
				spFileInfo->SetClipboard(&m_clipboard);	// this is the link table (CClipboardArray)
				
				spFileInfo->Create(&wfd, strDirName, stSrcIndex);
				if(m_afFilters.Match(spFileInfo))
					m_files.AddFileInfo(spFileInfo);
			}
			else if(wfd.cFileName[0] != _T('.') || (wfd.cFileName[1] != _T('\0') && (wfd.cFileName[1] != _T('.') || wfd.cFileName[2] != _T('\0'))))
			{
				if(bIncludeDirs)
				{
					CFileInfoPtr spFileInfo(boost::make_shared<CFileInfo>());
					spFileInfo->SetClipboard(&m_clipboard);	// this is the link table (CClipboardArray)

					// Add directory itself
					spFileInfo->Create(&wfd, strDirName, stSrcIndex);
					m_files.AddFileInfo(spFileInfo);
				}
				if(bRecurse)
				{
					strText = strDirName + wfd.cFileName + _T("\\");
					// Recurse Dirs
					ScanDirectory(strText, stSrcIndex, bRecurse, bIncludeDirs);
				}
			}

			if(m_workerThread.KillRequested())
				break;
		}
		while(FindNextFile(hFind, &wfd));
		
		FindClose(hFind);
	}

	return 0;
}

// m_strDestPath - adds '\\'
void CTask::SetDestPath(LPCTSTR lpszPath)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_dpDestPath.SetPath(lpszPath);
}

// guaranteed '\\'
const CDestPath& CTask::GetDestPath()
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_dpDestPath;
}

int CTask::GetDestDriveNumber()
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_dpDestPath.GetDriveNumber();
}

// m_nStatus
void CTask::SetStatus(UINT nStatus, UINT nMask)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_nStatus &= ~nMask;
	m_nStatus |= nStatus;
}

UINT CTask::GetStatus(UINT nMask)
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_nStatus & nMask;
}

void CTask::SetTaskState(ETaskCurrentState eTaskState)
{
	// NOTE: we could check some transition rules here
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_eCurrentState = eTaskState;
}

ETaskCurrentState CTask::GetTaskState() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_eCurrentState;
}

void CTask::SetOperationType(EOperationType eOperationType)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_eOperation = eOperationType;
}

EOperationType CTask::GetOperationType() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_eOperation;
}

// m_nBufferSize
void CTask::SetBufferSizes(const BUFFERSIZES* bsSizes)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_bsSizes=*bsSizes;
	m_bSaved=false;
}

const BUFFERSIZES* CTask::GetBufferSizes()
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return &m_bsSizes;
}

int CTask::GetCurrentBufferIndex()
{
	return m_files.GetBufferIndexAt(m_tTaskProgressInfo.GetCurrentIndex(),m_dpDestPath);
}

// m_pThread
// m_nPriority
int CTask::GetPriority()
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);

	return m_nPriority;
}

void CTask::SetPriority(int nPriority)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	SetPriorityNL(nPriority);
}

void CTask::CalculateTotalSize()
{
	unsigned long long ullTotalSize = 0;

	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	size_t nSize = m_files.GetSize();
	for(size_t i = 0; i < nSize; i++)
	{
		ullTotalSize += m_files.GetAt(i)->GetLength64();
	}

	m_localStats.SetTotalSize(ullTotalSize);
}

void CTask::CalculateProcessedSize()
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	CalculateProcessedSizeNL();
}

void CTask::CalculateProcessedSizeNL()
{
	m_localStats.SetProcessedSize(m_files.CalculatePartialSize(m_tTaskProgressInfo.GetCurrentIndex()));
}

// m_strUniqueName

CString CTask::GetUniqueName()
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_strUniqueName;
}

void CTask::Load(const CString& strPath, bool bData)
{
	std::ifstream ifs(strPath, ios_base::in | ios_base::binary);
	boost::archive::binary_iarchive ar(ifs);

	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	if(bData)
	{
		m_clipboard.Load(ar, 0, bData);

		m_files.Load(ar, 0, false);

		CalculateTotalSizeNL();

		ar >> m_dpDestPath;

		ar >> m_strUniqueName;
		ar >> m_afFilters;
	}
	else
	{
		UINT uiData = 0;
		int iState = eTaskState_None;

		ar >> m_tTaskProgressInfo;

		CalculateProcessedSizeNL();

		ar >> uiData;
		m_nStatus = uiData;

		// load task state, convert "waiting" state to "processing"
		ar >> iState;
		if(iState >= eTaskState_None && iState < eTaskState_Max)
		{
			if(iState == eTaskState_Waiting)
				iState = eTaskState_Processing;
			m_eCurrentState = (ETaskCurrentState)iState;
		}
		else
		{
			BOOST_ASSERT(false);
			THROW(_T("Wrong data read from stream"), 0, 0, 0);
		}

		ar >> iState;
		if(iState >= eOperation_Copy && iState <= eOperation_Move)
			m_eOperation = (EOperationType)iState;
		else
		{
			BOOST_ASSERT(false);
			THROW(_T("Wrong data read from stream"), 0, 0, 0);
		}

		ar >> m_bsSizes;
		ar >> m_nPriority;

		time_t timeElapsed = 0;
		ar >> timeElapsed;
		m_localStats.SetTimeElapsed(timeElapsed);

		m_clipboard.Load(ar, 0, bData);
		m_files.Load(ar, 0, true);

		ar >> m_bSaved;
	}
}

void CTask::Store(bool bData)
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	BOOST_ASSERT(!m_strTaskBasePath.empty());
	if(m_strTaskBasePath.empty())
		THROW(_t("Missing task path."), 0, 0, 0);

	if(!bData && m_bSaved)
		return;

	if(!bData && !m_bSaved && (m_eCurrentState == eTaskState_Finished || m_eCurrentState == eTaskState_Cancelled || m_nStatus == eTaskState_Paused))
	{
		m_bSaved = true;
	}

	CString strPath = m_strTaskBasePath.c_str() + m_strUniqueName + (bData ? _T(".atd") : _T(".atp"));

	std::ofstream ofs(strPath, ios_base::out | ios_base::binary);
	boost::archive::binary_oarchive ar(ofs);

	if(bData)
	{
		m_clipboard.Store(ar, 0, bData);

		if(GetStatusNL(ST_STEP_MASK) > ST_SEARCHING)
			m_files.Store(ar, 0, false);
		else
		{
			size_t st(0);
			ar << st;
		}

		ar << m_dpDestPath;
		ar << m_strUniqueName;
		ar << m_afFilters;
	}
	else
	{
		ar << m_tTaskProgressInfo;

		UINT uiStatus = (m_nStatus & ST_WRITE_MASK);
		ar << uiStatus;

		// store current state (convert from waiting to processing state before storing)
		int iState = m_eCurrentState;
		if(iState == eTaskState_Waiting)
			iState = eTaskState_Processing;

		ar << iState;

		iState = m_eOperation;
		ar << iState;

		ar << m_bsSizes;
		ar << m_nPriority;

		time_t timeElapsed = m_localStats.GetTimeElapsed();
		ar << timeElapsed;

		m_clipboard.Store(ar, 0, bData);
		if(GetStatusNL(ST_STEP_MASK) > ST_SEARCHING)
			m_files.Store(ar, 0, true);
		else
		{
			size_t st(0);
			ar << st;
		}
		ar << m_bSaved;
	}
}

void CTask::KillThread()
{
	m_workerThread.StopThread();
}

void CTask::BeginProcessing()
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);

	m_bSaved = false;		// save

	m_workerThread.StartThread(DelegateThreadProc, this, m_nPriority);
}

void CTask::ResumeProcessing()
{
	// the same as retry but less demanding
	if(GetTaskState() == eTaskState_Paused)
	{
		SetTaskState(eTaskState_Processing);
		BeginProcessing();
	}
}

bool CTask::RetryProcessing()
{
	// retry used to auto-resume, after loading
	if(GetTaskState() != eTaskState_Paused && GetTaskState() != eTaskState_Finished && GetTaskState() != eTaskState_Cancelled)
	{
		BeginProcessing();
		return true;
	}
	return false;
}

void CTask::RestartProcessing()
{
	KillThread();

	SetTaskState(eTaskState_None);

	m_localStats.SetTimeElapsed(0);
	m_tTaskProgressInfo.SetCurrentIndex(0);

	BeginProcessing();
}

void CTask::PauseProcessing()
{
	if(GetTaskState() != eTaskState_Finished && GetTaskState() != eTaskState_Cancelled)
	{
		KillThread();
		SetTaskState(eTaskState_Paused);
		m_bSaved = false;
	}
}

void CTask::CancelProcessing()
{
	// change to ST_CANCELLED
	if(GetTaskState() != eTaskState_Finished)
	{
		KillThread();
		SetTaskState(eTaskState_Cancelled);
		m_bSaved=false;
	}
}

void CTask::GetMiniSnapshot(TASK_MINI_DISPLAY_DATA *pData)
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	size_t stCurrentIndex = m_tTaskProgressInfo.GetCurrentIndex();

	if(stCurrentIndex < m_files.GetSize())
		pData->m_strPath = m_files.GetAt(stCurrentIndex)->GetFileName();
	else
	{
		if(m_files.GetSize() > 0)
			pData->m_strPath = m_files.GetAt(0)->GetFileName();
		else
		{
			if(m_clipboard.GetSize() > 0)
				pData->m_strPath = m_clipboard.GetAt(0)->GetFileName();
			else
				pData->m_strPath = GetResManager().LoadString(IDS_NONEINPUTFILE_STRING);
		}
	}

	pData->m_eTaskState = m_eCurrentState;

	// percents
	pData->m_nPercent = m_localStats.GetProgressInPercent();
}

void CTask::GetSnapshot(TASK_DISPLAY_DATA *pData)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);

	size_t stCurrentIndex = m_tTaskProgressInfo.GetCurrentIndex();
	if(stCurrentIndex < m_files.GetSize())
	{
		pData->m_strFullFilePath = m_files.GetAt(stCurrentIndex)->GetFullFilePath();
		pData->m_strFileName = m_files.GetAt(stCurrentIndex)->GetFileName();
	}
	else
	{
		if(m_files.GetSize() > 0)
		{
			pData->m_strFullFilePath = m_files.GetAt(0)->GetFullFilePath();
			pData->m_strFileName = m_files.GetAt(0)->GetFileName();
		}
		else
		{
			if(m_clipboard.GetSize() > 0)
			{
				pData->m_strFullFilePath = m_clipboard.GetAt(0)->GetPath();
				pData->m_strFileName = m_clipboard.GetAt(0)->GetFileName();
			}
			else
			{
				pData->m_strFullFilePath = GetResManager().LoadString(IDS_NONEINPUTFILE_STRING);
				pData->m_strFileName = pData->m_strFullFilePath;
			}
		}
	}

	pData->m_pbsSizes=&m_bsSizes;
	pData->m_nPriority=m_nPriority;
	pData->m_pdpDestPath=&m_dpDestPath;
	pData->m_pafFilters=&m_afFilters;
	pData->m_eTaskState = m_eCurrentState;
	pData->m_stIndex = stCurrentIndex;
	pData->m_ullProcessedSize = m_localStats.GetProcessedSize();
	pData->m_stSize=m_files.GetSize();
	pData->m_ullSizeAll = m_localStats.GetTotalSize();
	pData->m_pstrUniqueName=&m_strUniqueName;

	if(m_files.GetSize() > 0)
		pData->m_iCurrentBufferIndex=m_bsSizes.m_bOnlyDefault ? 0 : m_files.GetAt((stCurrentIndex < m_files.GetSize()) ? stCurrentIndex : 0)->GetBufferIndex(m_dpDestPath);
	else
		pData->m_iCurrentBufferIndex=0;

	// percents
	pData->m_nPercent = m_localStats.GetProgressInPercent();

	// status string
	// first
	switch(m_eCurrentState)
	{
	case eTaskState_Error:
		{
			GetResManager().LoadStringCopy(IDS_STATUS0_STRING+4, pData->m_szStatusText, _MAX_PATH);
			_tcscat(pData->m_szStatusText, _T("/"));
			break;
		}
	case eTaskState_Paused:
		{
			GetResManager().LoadStringCopy(IDS_STATUS0_STRING+5, pData->m_szStatusText, _MAX_PATH);
			_tcscat(pData->m_szStatusText, _T("/"));
			break;
		}
	case eTaskState_Finished:
		{
			GetResManager().LoadStringCopy(IDS_STATUS0_STRING+3, pData->m_szStatusText, _MAX_PATH);
			_tcscat(pData->m_szStatusText, _T("/"));
			break;
		}
	case eTaskState_Waiting:
		{
			GetResManager().LoadStringCopy(IDS_STATUS0_STRING+9, pData->m_szStatusText, _MAX_PATH);
			_tcscat(pData->m_szStatusText, _T("/"));
			break;
		}
	case eTaskState_Cancelled:
		{
			GetResManager().LoadStringCopy(IDS_STATUS0_STRING+8, pData->m_szStatusText, _MAX_PATH);
			_tcscat(pData->m_szStatusText, _T("/"));
			break;
		}
	default:
		_tcscpy(pData->m_szStatusText, _T(""));
	}

	// second part
	if( (m_nStatus & ST_STEP_MASK) == ST_DELETING )
		_tcscat(pData->m_szStatusText, GetResManager().LoadString(IDS_STATUS0_STRING+6));
	else if( (m_nStatus & ST_STEP_MASK) == ST_SEARCHING )
		_tcscat(pData->m_szStatusText, GetResManager().LoadString(IDS_STATUS0_STRING+0));
	else if(m_eOperation == eOperation_Copy)
	{
		_tcscat(pData->m_szStatusText, GetResManager().LoadString(IDS_STATUS0_STRING+1));
		if(!m_afFilters.IsEmpty())
			_tcscat(pData->m_szStatusText, GetResManager().LoadString(IDS_FILTERING_STRING));
	}
	else if(m_eOperation == eOperation_Move)
	{
		_tcscat(pData->m_szStatusText, GetResManager().LoadString(IDS_STATUS0_STRING+2));
		if(!m_afFilters.IsEmpty())
			_tcscat(pData->m_szStatusText, GetResManager().LoadString(IDS_FILTERING_STRING));
	}
	else
		_tcscat(pData->m_szStatusText, GetResManager().LoadString(IDS_STATUS0_STRING+7));

	// third part
	if( (m_nStatus & ST_SPECIAL_MASK) & ST_IGNORE_DIRS )
	{
		_tcscat(pData->m_szStatusText, _T("/"));
		_tcscat(pData->m_szStatusText, GetResManager().LoadString(IDS_STATUS0_STRING+10));
	}
	if( (m_nStatus & ST_SPECIAL_MASK) & ST_IGNORE_CONTENT )
	{
		_tcscat(pData->m_szStatusText, _T("/"));
		_tcscat(pData->m_szStatusText, GetResManager().LoadString(IDS_STATUS0_STRING+11));
	}

	// time
	pData->m_timeElapsed = m_localStats.GetTimeElapsed();
}

void CTask::DeleteProgress(LPCTSTR lpszDirectory)
{
	m_lock.lock_shared();

	CString strDel1 = lpszDirectory+m_strUniqueName+_T(".atd");
	CString strDel2 = lpszDirectory+m_strUniqueName+_T(".atp");
	CString strDel3 = lpszDirectory+m_strUniqueName+_T(".log");

	m_lock.unlock_shared();

	DeleteFile(strDel1);
	DeleteFile(strDel2);
	DeleteFile(strDel3);
}

void CTask::SetFilters(const CFiltersArray* pFilters)
{
	BOOST_ASSERT(pFilters);
	if(!pFilters)
		THROW(_T("Invalid argument"), 0, 0, 0);

	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_afFilters = *pFilters;
}

bool CTask::CanBegin()
{
	bool bRet=true;
	boost::unique_lock<boost::shared_mutex> lock(m_lock);

	if(GetContinueFlagNL() || GetForceFlagNL())
	{
		m_localStats.MarkTaskAsRunning();
		SetForceFlagNL(false);
		SetContinueFlagNL(false);
	}
	else
		bRet = false;

	return bRet;
}

bool CTask::GetRequiredFreeSpace(ull_t *pullNeeded, ull_t *pullAvailable)
{
	BOOST_ASSERT(pullNeeded && pullAvailable);
	if(!pullNeeded || !pullAvailable)
		THROW(_T("Invalid argument"), 0, 0, 0);

	*pullNeeded = m_localStats.GetUnProcessedSize(); // it'd be nice to round up to take cluster size into consideration,
	// but GetDiskFreeSpace returns false values

	// get free space
	if(!GetDynamicFreeSpace(GetDestPath().GetPath(), pullAvailable, NULL))
		return true;

	return (*pullNeeded <= *pullAvailable);
}

void CTask::SetTaskPath(const tchar_t* pszDir)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_strTaskBasePath = pszDir;
}

const tchar_t* CTask::GetTaskPath() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_strTaskBasePath.c_str();
}

void CTask::SetForceFlag(bool bFlag)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_bForce=bFlag;
}

bool CTask::GetForceFlag()
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_bForce;
}

void CTask::SetContinueFlag(bool bFlag)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_bContinue=bFlag;
}

bool CTask::GetContinueFlag()
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_bContinue;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CTask::SetFileDirectoryTime(LPCTSTR lpszName, const CFileInfoPtr& spFileInfo)
{
	TAutoFileHandle hFile = CreateFile(lpszName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | (spFileInfo->IsDirectory() ? FILE_FLAG_BACKUP_SEMANTICS : 0), NULL);
	if(hFile == INVALID_HANDLE_VALUE)
		return false;

	BOOL bResult = (!SetFileTime(hFile, &spFileInfo->GetCreationTime(), &spFileInfo->GetLastAccessTime(), &spFileInfo->GetLastWriteTime()));

	if(!hFile.Close())
		return false;

	return bResult != 0;
}

// m_nStatus
void CTask::SetStatusNL(UINT nStatus, UINT nMask)
{
	m_nStatus &= ~nMask;
	m_nStatus |= nStatus;
}

UINT CTask::GetStatusNL(UINT nMask)
{
	return m_nStatus & nMask;
}

// m_nBufferSize
void CTask::SetBufferSizesNL(const BUFFERSIZES* bsSizes)
{
	m_bsSizes = *bsSizes;
	m_bSaved = false;
}

const BUFFERSIZES* CTask::GetBufferSizesNL()
{
	return &m_bsSizes;
}

// m_pThread
// m_nPriority
int CTask::GetPriorityNL()
{
	return m_nPriority;
}

void CTask::SetPriorityNL(int nPriority)
{
	m_workerThread.ChangePriority(nPriority);

	m_nPriority = nPriority;
	m_bSaved = false;
}

void CTask::CalculateTotalSizeNL()
{
	unsigned long long ullTotalSize = 0;

	size_t nSize = m_files.GetSize();
	for(size_t stIndex = 0; stIndex < nSize; stIndex++)
	{
		ullTotalSize += m_files.GetAt(stIndex)->GetLength64();
	}

	m_localStats.SetTotalSize(ullTotalSize);
}

void CTask::SetForceFlagNL(bool bFlag)
{
	m_bForce=bFlag;
}

bool CTask::GetForceFlagNL()
{
	return m_bForce;
}

void CTask::SetContinueFlagNL(bool bFlag)
{
	m_bContinue=bFlag;
}

bool CTask::GetContinueFlagNL()
{
	return m_bContinue;
}

// searching for files
void CTask::RecurseDirectories()
{
	// log
	m_log.logi(_T("Searching for files..."));

	// update status
	SetStatus(ST_SEARCHING, ST_STEP_MASK);

	// delete the content of m_files
	m_files.Clear();

	// enter some data to m_files
	int iDestDrvNumber = GetDestDriveNumber();
	bool bIgnoreDirs = (GetStatus(ST_SPECIAL_MASK) & ST_IGNORE_DIRS) != 0;
	bool bForceDirectories = (GetStatus(ST_SPECIAL_MASK) & ST_FORCE_DIRS) != 0;
	bool bMove = GetOperationType() == eOperation_Move;

	// add everything
	ictranslate::CFormat fmt;
	bool bRetry = true;
	bool bSkipInputPath = false;

	size_t stSize = GetClipboardDataSize();
	for(size_t stIndex = 0; stIndex < stSize ; stIndex++)
	{
		CFileInfoPtr spFileInfo;

		bSkipInputPath = false;

		spFileInfo.reset(new CFileInfo());
		spFileInfo->SetClipboard(GetClipboard());

		// try to get some info about the input path; let user know if the path does not exist.
		do
		{
			bRetry = false;

			// read attributes of src file/folder
			bool bExists = spFileInfo->Create(GetClipboardData(stIndex)->GetPath(), stIndex);
			if(!bExists)
			{
				CString strSrcFile = GetClipboardData(stIndex)->GetPath();
				FEEDBACK_FILEERROR ferr = { (PCTSTR)strSrcFile, NULL, eFastMoveError, ERROR_FILE_NOT_FOUND };
				CFeedbackHandler::EFeedbackResult frResult = (CFeedbackHandler::EFeedbackResult)m_piFeedbackHandler->RequestFeedback(CFeedbackHandler::eFT_FileError, &ferr);
				switch(frResult)
				{
				case CFeedbackHandler::eResult_Cancel:
					throw new CProcessingException(E_CANCEL);

				case CFeedbackHandler::eResult_Retry:
					bRetry = true;
					break;

				case CFeedbackHandler::eResult_Pause:
					throw new CProcessingException(E_PAUSE);

				case CFeedbackHandler::eResult_Skip:
					bSkipInputPath = true;
					break;		// just do nothing

				default:
					BOOST_ASSERT(FALSE);		// unknown result
					throw new CProcessingException(E_ERROR, 0, _t("Unknown feedback result type"));
				}
			}
		}
		while(bRetry);

		// if we have chosen to skip the input path then there's nothing to do
		if(bSkipInputPath)
			continue;

		// log
		fmt.SetFormat(_T("Adding file/folder (clipboard) : %path ..."));
		fmt.SetParam(_t("%path"), GetClipboardData(stIndex)->GetPath());
		m_log.logi(fmt);

		// found file/folder - check if the dest name has been generated
		if(!GetClipboardData(stIndex)->IsDestinationPathSet())
		{
			// generate something - if dest folder == src folder - search for copy
			if(GetDestPath().GetPath() == spFileInfo->GetFileRoot())
			{
				CString strSubst;
				FindFreeSubstituteName(spFileInfo->GetFullFilePath(), GetDestPath().GetPath(), &strSubst);
				GetClipboardData(stIndex)->SetDestinationPath(strSubst);
			}
			else
				GetClipboardData(stIndex)->SetDestinationPath(spFileInfo->GetFileName());
		}

		// add if needed
		if(spFileInfo->IsDirectory())
		{
			// add if folder's aren't ignored
			if(!bIgnoreDirs && !bForceDirectories)
			{
				// add directory info; it is not to be filtered with m_afFilters
				m_files.AddFileInfo(spFileInfo);

				// log
				fmt.SetFormat(_T("Added folder %path"));
				fmt.SetParam(_t("%path"), spFileInfo->GetFullFilePath());
				m_log.logi(fmt);
			}

			// don't add folder contents when moving inside one disk boundary
			if(bIgnoreDirs || !bMove || iDestDrvNumber == -1
				|| iDestDrvNumber != spFileInfo->GetDriveNumber() || CFileInfo::Exist(spFileInfo->GetDestinationPath(GetDestPath().GetPath(), ((int)bForceDirectories) << 1)) )
			{
				// log
				fmt.SetFormat(_T("Recursing folder %path"));
				fmt.SetParam(_t("%path"), spFileInfo->GetFullFilePath());
				m_log.logi(fmt);

				// no movefile possibility - use CustomCopyFile
				GetClipboardData(stIndex)->SetMove(false);

				ScanDirectory(spFileInfo->GetFullFilePath(), stIndex, true, !bIgnoreDirs || bForceDirectories);
			}

			// check for kill need
			if(m_workerThread.KillRequested())
			{
				// log
				m_log.logi(_T("Kill request while adding data to files array (RecurseDirectories)"));
				throw new CProcessingException(E_KILL_REQUEST);
			}
		}
		else
		{
			if(bMove && iDestDrvNumber != -1 && iDestDrvNumber == spFileInfo->GetDriveNumber() &&
				!CFileInfo::Exist(spFileInfo->GetDestinationPath(GetDestPath().GetPath(), ((int)bForceDirectories) << 1)) )
			{
				// if moving within one partition boundary set the file size to 0 so the overall size will
				// be ok
				spFileInfo->SetLength64(0);
			}
			else
				GetClipboardData(stIndex)->SetMove(false);	// no MoveFile

			// add file info if passes filters
			if(m_afFilters.Match(spFileInfo))
				m_files.AddFileInfo(spFileInfo);

			// log
			fmt.SetFormat(_T("Added file %path"));
			fmt.SetParam(_t("%path"), spFileInfo->GetFullFilePath());
			m_log.logi(fmt);
		}
	}

	// calc size of all files
	CalculateTotalSize();

	// change state to ST_COPYING - finished searching for files
	SetStatus(ST_COPYING, ST_STEP_MASK);

	// save task status
	Store(true);
	Store(false);

	// log
	m_log.logi(_T("Searching for files finished"));
}

// delete files - after copying
void CTask::DeleteFiles()
{
	// log
	m_log.logi(_T("Deleting files (DeleteFiles)..."));

	// current processed path
	BOOL bSuccess;
	CFileInfoPtr spFileInfo;
	ictranslate::CFormat fmt;

	// index points to 0 or next item to process
	size_t stIndex = m_tTaskProgressInfo.GetCurrentIndex();
	while(stIndex < m_files.GetSize())
	{
		// set index in pTask to currently deleted element
		m_tTaskProgressInfo.SetCurrentIndex(stIndex);

		// check for kill flag
		if(m_workerThread.KillRequested())
		{
			// log
			m_log.logi(_T("Kill request while deleting files (Delete Files)"));
			throw new CProcessingException(E_KILL_REQUEST);
		}

		// current processed element
		spFileInfo = m_files.GetAt(m_files.GetSize() - stIndex - 1);
		if(!(spFileInfo->GetFlags() & FIF_PROCESSED))
		{
			++stIndex;
			continue;
		}

		// delete data
		if(spFileInfo->IsDirectory())
		{
			if(!GetConfig().get_bool(PP_CMPROTECTROFILES))
				SetFileAttributes(spFileInfo->GetFullFilePath(), FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_DIRECTORY);
			bSuccess=RemoveDirectory(spFileInfo->GetFullFilePath());
		}
		else
		{
			// set files attributes to normal - it'd slow processing a bit, but it's better.
			if(!GetConfig().get_bool(PP_CMPROTECTROFILES))
				SetFileAttributes(spFileInfo->GetFullFilePath(), FILE_ATTRIBUTE_NORMAL);
			bSuccess=DeleteFile(spFileInfo->GetFullFilePath());
		}

		// operation failed
		DWORD dwLastError=GetLastError();
		if(!bSuccess && dwLastError != ERROR_PATH_NOT_FOUND && dwLastError != ERROR_FILE_NOT_FOUND)
		{
			// log
			fmt.SetFormat(_T("Error #%errno while deleting file/folder %path"));
			fmt.SetParam(_t("%errno"), dwLastError);
			fmt.SetParam(_t("%path"), spFileInfo->GetFullFilePath());
			m_log.loge(fmt);

			CString strFile = spFileInfo->GetFullFilePath();
			FEEDBACK_FILEERROR ferr = { (PCTSTR)strFile, NULL, eDeleteError, dwLastError };
			CFeedbackHandler::EFeedbackResult frResult = (CFeedbackHandler::EFeedbackResult)m_piFeedbackHandler->RequestFeedback(CFeedbackHandler::eFT_FileError, &ferr);
			switch(frResult)
			{
			case CFeedbackHandler::eResult_Cancel:
				m_log.logi(_T("Cancel request while deleting file."));
				throw new CProcessingException(E_CANCEL);
				break;
			case CFeedbackHandler::eResult_Retry:
				continue;	// no stIndex bump, since we are trying again
				break;
			case CFeedbackHandler::eResult_Pause:
				throw new CProcessingException(E_PAUSE);
				break;
			case CFeedbackHandler::eResult_Skip:
				break;		// just do nothing
			default:
				BOOST_ASSERT(FALSE);		// unknown result
				throw new CProcessingException(E_ERROR, 0, _t("Unknown feedback result type"));
			}
		}

		++stIndex;
	}//while

	// change status to finished
	SetTaskState(eTaskState_Finished);

	// add 1 to current index
	m_tTaskProgressInfo.IncreaseCurrentIndex();

	// log
	m_log.logi(_T("Deleting files finished"));
}

HANDLE CTask::OpenSourceFileFB(const CFileInfoPtr& spSrcFileInfo, bool bNoBuffering)
{
	BOOST_ASSERT(spSrcFileInfo);
	if(!spSrcFileInfo)
		THROW(_T("Invalid argument"), 0, 0, 0);

	bool bRetry = false;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	CString strPath = spSrcFileInfo->GetFullFilePath();

	do
	{
		bRetry = false;

		hFile = CreateFile(strPath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN | (bNoBuffering ? FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH : 0), NULL);
		if(hFile == INVALID_HANDLE_VALUE)
		{
			DWORD dwLastError = GetLastError();

			FEEDBACK_FILEERROR feedStruct = { (PCTSTR)strPath, NULL, eCreateError, dwLastError };
			CFeedbackHandler::EFeedbackResult frResult = (CFeedbackHandler::EFeedbackResult)m_piFeedbackHandler->RequestFeedback(CFeedbackHandler::eFT_FileError, &feedStruct);

			switch(frResult)
			{
			case CFeedbackHandler::eResult_Skip:
				break;	// will return INVALID_HANDLE_VALUE

			case CFeedbackHandler::eResult_Cancel:
				{
					// log
					ictranslate::CFormat fmt;
					fmt.SetFormat(_T("Cancel request [error %errno] while opening source file %path (CustomCopyFile)"));
					fmt.SetParam(_t("%errno"), dwLastError);
					fmt.SetParam(_t("%path"), strPath);
					m_log.loge(fmt);
					throw new CProcessingException(E_CANCEL);
				}

			case CFeedbackHandler::eResult_Pause:
				throw new CProcessingException(E_PAUSE);

			case CFeedbackHandler::eResult_Retry:
				{
					// log
					ictranslate::CFormat fmt;
					fmt.SetFormat(_T("Retrying [error %errno] to open source file %path (CustomCopyFile)"));
					fmt.SetParam(_t("%errno"), dwLastError);
					fmt.SetParam(_t("%path"), strPath);
					m_log.loge(fmt);

					bRetry = true;
					break;
				}

			default:
				BOOST_ASSERT(FALSE);		// unknown result
				THROW(_T("Unhandled case"), 0, 0, 0);
			}
		}
	}
	while(bRetry);

	return hFile;
}

HANDLE CTask::OpenDestinationFileFB(const CString& strDstFilePath, bool bNoBuffering, const CFileInfoPtr& spSrcFileInfo, unsigned long long& ullSeekTo, bool& bFreshlyCreated)
{
	bool bRetry = false;
	TAutoFileHandle hFile;

	ullSeekTo = 0;
	bFreshlyCreated = true;

	do
	{
		bRetry = false;

		hFile = CreateFile(strDstFilePath, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN | (bNoBuffering ? FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH : 0), NULL);
		if(hFile == INVALID_HANDLE_VALUE)
		{
			DWORD dwLastError = GetLastError();
			if(dwLastError == ERROR_FILE_EXISTS)
			{
				bFreshlyCreated = false;

				// pass it to the specialized method
				hFile = OpenExistingDestinationFileFB(strDstFilePath, bNoBuffering);
				if(hFile == INVALID_HANDLE_VALUE)
					return hFile;

				// read info about the existing destination file,
				// NOTE: it is not known which one would be faster - reading file parameters
				//       by using spDstFileInfo->Create() (which uses FindFirstFile()) or by
				//       reading parameters using opened handle; need to be tested in the future
				CFileInfoPtr spDstFileInfo(boost::make_shared<CFileInfo>());
				if(!spDstFileInfo->Create(strDstFilePath, std::numeric_limits<size_t>::max()))
					THROW(_T("Cannot get information about file which has already been opened!"), 0, GetLastError(), 0);

				// src and dst files are the same
				FEEDBACK_ALREADYEXISTS feedStruct = { spSrcFileInfo, spDstFileInfo };
				CFeedbackHandler::EFeedbackResult frResult = (CFeedbackHandler::EFeedbackResult)m_piFeedbackHandler->RequestFeedback(CFeedbackHandler::eFT_FileAlreadyExists, &feedStruct);
				// check for dialog result
				switch(frResult)
				{
				case CFeedbackHandler::eResult_Overwrite:
					ullSeekTo = 0;
					break;

				case CFeedbackHandler::eResult_CopyRest:
					ullSeekTo = spDstFileInfo->GetLength64();
					break;

				case CFeedbackHandler::eResult_Skip:
					return INVALID_HANDLE_VALUE;

				case CFeedbackHandler::eResult_Cancel:
					{
						// log
						ictranslate::CFormat fmt;
						fmt.SetFormat(_T("Cancel request while checking result of dialog before opening source file %path (CustomCopyFile)"));
						fmt.SetParam(_t("%path"), strDstFilePath);
						m_log.logi(fmt);

						throw new CProcessingException(E_CANCEL);
					}
				case CFeedbackHandler::eResult_Pause:
					throw new CProcessingException(E_PAUSE);

				default:
					BOOST_ASSERT(FALSE);		// unknown result
					THROW(_T("Unhandled case"), 0, 0, 0);
				}
			}
			else
			{
				FEEDBACK_FILEERROR feedStruct = { (PCTSTR)strDstFilePath, NULL, eCreateError, dwLastError };
				CFeedbackHandler::EFeedbackResult frResult = (CFeedbackHandler::EFeedbackResult)m_piFeedbackHandler->RequestFeedback(CFeedbackHandler::eFT_FileError, &feedStruct);
				switch (frResult)
				{
				case CFeedbackHandler::eResult_Retry:
					{
						// log
						ictranslate::CFormat fmt;
						fmt.SetFormat(_T("Retrying [error %errno] to open destination file %path (CustomCopyFile)"));
						fmt.SetParam(_t("%errno"), dwLastError);
						fmt.SetParam(_t("%path"), strDstFilePath);
						m_log.loge(fmt);

						bRetry = true;

						break;
					}
				case CFeedbackHandler::eResult_Cancel:
					{
						// log
						ictranslate::CFormat fmt;

						fmt.SetFormat(_T("Cancel request [error %errno] while opening destination file %path (CustomCopyFile)"));
						fmt.SetParam(_t("%errno"), dwLastError);
						fmt.SetParam(_t("%path"), strDstFilePath);
						m_log.loge(fmt);

						throw new CProcessingException(E_CANCEL);
					}

				case CFeedbackHandler::eResult_Skip:
					break;		// will return invalid handle value

				case CFeedbackHandler::eResult_Pause:
					throw new CProcessingException(E_PAUSE);

				default:
					BOOST_ASSERT(FALSE);		// unknown result
					THROW(_T("Unhandled case"), 0, 0, 0);
				}
			}
		}
	}
	while(bRetry);

	return hFile.Detach();

}

HANDLE CTask::OpenExistingDestinationFileFB(const CString& strDstFilePath, bool bNoBuffering)
{
	bool bRetry = false;
	HANDLE hFile = INVALID_HANDLE_VALUE;

	do
	{
		bRetry = false;

		hFile = CreateFile(strDstFilePath, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN | (bNoBuffering ? FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH : 0), NULL);
		if(hFile == INVALID_HANDLE_VALUE)
		{
			DWORD dwLastError = GetLastError();
			FEEDBACK_FILEERROR feedStruct = { (PCTSTR)strDstFilePath, NULL, eCreateError, dwLastError };
			CFeedbackHandler::EFeedbackResult frResult = (CFeedbackHandler::EFeedbackResult)m_piFeedbackHandler->RequestFeedback(CFeedbackHandler::eFT_FileError, &feedStruct);
			switch (frResult)
			{
			case CFeedbackHandler::eResult_Retry:
				{
					// log
					ictranslate::CFormat fmt;
					fmt.SetFormat(_T("Retrying [error %errno] to open destination file %path (CustomCopyFile)"));
					fmt.SetParam(_t("%errno"), dwLastError);
					fmt.SetParam(_t("%path"), strDstFilePath);
					m_log.loge(fmt);

					bRetry = true;

					break;
				}
			case CFeedbackHandler::eResult_Cancel:
				{
					// log
					ictranslate::CFormat fmt;

					fmt.SetFormat(_T("Cancel request [error %errno] while opening destination file %path (CustomCopyFile)"));
					fmt.SetParam(_t("%errno"), dwLastError);
					fmt.SetParam(_t("%path"), strDstFilePath);
					m_log.loge(fmt);

					throw new CProcessingException(E_CANCEL);
				}

			case CFeedbackHandler::eResult_Skip:
				break;		// will return invalid handle value

			case CFeedbackHandler::eResult_Pause:
				throw new CProcessingException(E_PAUSE);

			default:
				BOOST_ASSERT(FALSE);		// unknown result
				THROW(_T("Unhandled case"), 0, 0, 0);
			}
		}
	}
	while(bRetry);

	return hFile;
}

bool CTask::SetFilePointerFB(HANDLE hFile, long long llDistance, const CString& strFilePath)
{
	bool bRetry = false;
	do
	{
		bRetry = false;

		if(SetFilePointer64(hFile, llDistance, FILE_BEGIN) == -1)
		{
			DWORD dwLastError = GetLastError();

			// log
			ictranslate::CFormat fmt;

			fmt.SetFormat(_T("Error %errno while moving file pointer of %path to %pos"));
			fmt.SetParam(_t("%errno"), dwLastError);
			fmt.SetParam(_t("%path"), strFilePath);
			fmt.SetParam(_t("%pos"), llDistance);
			m_log.loge(fmt);

			FEEDBACK_FILEERROR ferr = { (PCTSTR)strFilePath, NULL, eSeekError, dwLastError };
			CFeedbackHandler::EFeedbackResult frResult = (CFeedbackHandler::EFeedbackResult)m_piFeedbackHandler->RequestFeedback(CFeedbackHandler::eFT_FileError, &ferr);
			switch(frResult)
			{
			case CFeedbackHandler::eResult_Cancel:
				throw new CProcessingException(E_CANCEL);

			case CFeedbackHandler::eResult_Retry:
				bRetry = true;
				break;

			case CFeedbackHandler::eResult_Pause:
				throw new CProcessingException(E_PAUSE);

			case CFeedbackHandler::eResult_Skip:
				return false;

			default:
				BOOST_ASSERT(FALSE);		// unknown result
				THROW(_T("Unhandled case"), 0, 0, 0);
			}
		}
	}
	while(bRetry);

	return true;
}

bool CTask::SetEndOfFileFB(HANDLE hFile, const CString& strFilePath)
{
	bool bRetry = false;
	do
	{
		if(!SetEndOfFile(hFile))
		{
			// log
			DWORD dwLastError = GetLastError();

			ictranslate::CFormat fmt;
			fmt.SetFormat(_T("Error %errno while setting size of file %path to 0"));
			fmt.SetParam(_t("%errno"), dwLastError);
			fmt.SetParam(_t("%path"), strFilePath);
			m_log.loge(fmt);

			FEEDBACK_FILEERROR ferr = { (PCTSTR)strFilePath, NULL, eResizeError, dwLastError };
			CFeedbackHandler::EFeedbackResult frResult = (CFeedbackHandler::EFeedbackResult)m_piFeedbackHandler->RequestFeedback(CFeedbackHandler::eFT_FileError, &ferr);
			switch(frResult)
			{
			case CFeedbackHandler::eResult_Cancel:
				throw new CProcessingException(E_CANCEL);

			case CFeedbackHandler::eResult_Retry:
				bRetry = true;

			case CFeedbackHandler::eResult_Pause:
				throw new CProcessingException(E_PAUSE);

			case CFeedbackHandler::eResult_Skip:
				break;		// just do nothing

			default:
				BOOST_ASSERT(FALSE);		// unknown result
				THROW(_T("Unhandled case"), 0, 0, 0);
			}
		}
	}
	while(bRetry);

	return true;
}

bool CTask::ReadFileFB(HANDLE hFile, CDataBuffer& rBuffer, DWORD dwToRead, DWORD& rdwBytesRead, const CString& strFilePath)
{
	bool bRetry = false;
	do
	{
		bRetry = false;

		if(!ReadFile(hFile, rBuffer, dwToRead, &rdwBytesRead, NULL))
		{
			// log
			DWORD dwLastError = GetLastError();

			ictranslate::CFormat fmt;
			fmt.SetFormat(_T("Error %errno while trying to read %count bytes from source file %path (CustomCopyFile)"));
			fmt.SetParam(_t("%errno"), dwLastError);
			fmt.SetParam(_t("%count"), dwToRead);
			fmt.SetParam(_t("%path"), strFilePath);
			m_log.loge(fmt);

			FEEDBACK_FILEERROR ferr = { (PCTSTR)strFilePath, NULL, eReadError, dwLastError };
			CFeedbackHandler::EFeedbackResult frResult = (CFeedbackHandler::EFeedbackResult)m_piFeedbackHandler->RequestFeedback(CFeedbackHandler::eFT_FileError, &ferr);
			switch(frResult)
			{
			case CFeedbackHandler::eResult_Cancel:
				throw new CProcessingException(E_CANCEL);

			case CFeedbackHandler::eResult_Retry:
				bRetry = true;
				break;

			case CFeedbackHandler::eResult_Pause:
				throw new CProcessingException(E_PAUSE);

			case CFeedbackHandler::eResult_Skip:
				return false;

			default:
				BOOST_ASSERT(FALSE);		// unknown result
				THROW(_T("Unhandled case"), 0, 0, 0);
			}
		}
	}
	while(bRetry);

	return true;
}

bool CTask::WriteFileFB(HANDLE hFile, CDataBuffer& rBuffer, DWORD dwToWrite, DWORD& rdwBytesWritten, const CString& strFilePath)
{
	bool bRetry = false;
	do
	{
		bRetry = false;

		if(!WriteFile(hFile, rBuffer, dwToWrite, &rdwBytesWritten, NULL) || dwToWrite != rdwBytesWritten)
		{
			// log
			DWORD dwLastError = GetLastError();

			ictranslate::CFormat fmt;
			fmt.SetFormat(_T("Error %errno while trying to write %count bytes to destination file %path (CustomCopyFile)"));
			fmt.SetParam(_t("%errno"), dwLastError);
			fmt.SetParam(_t("%count"), dwToWrite);
			fmt.SetParam(_t("%path"), strFilePath);
			m_log.loge(fmt);

			FEEDBACK_FILEERROR ferr = { (PCTSTR)strFilePath, NULL, eWriteError, dwLastError };
			CFeedbackHandler::EFeedbackResult frResult = (CFeedbackHandler::EFeedbackResult)m_piFeedbackHandler->RequestFeedback(CFeedbackHandler::eFT_FileError, &ferr);
			switch(frResult)
			{
			case CFeedbackHandler::eResult_Cancel:
				throw new CProcessingException(E_CANCEL);

			case CFeedbackHandler::eResult_Retry:
				bRetry = true;
				break;

			case CFeedbackHandler::eResult_Pause:
				throw new CProcessingException(E_PAUSE);

			case CFeedbackHandler::eResult_Skip:
				return false;

			default:
				BOOST_ASSERT(FALSE);		// unknown result
				THROW(_T("Unhandled case"), 0, 0, 0);
			}
		}
	}
	while(bRetry);

	return true;
}

void CTask::CustomCopyFile(CUSTOM_COPY_PARAMS* pData)
{
	TAutoFileHandle hSrc = INVALID_HANDLE_VALUE,
					hDst = INVALID_HANDLE_VALUE;
	ictranslate::CFormat fmt;

	// calculate if we want to disable buffering for file transfer
	// NOTE: we are using here the file size read when scanning directories for files; it might be
	//       outdated at this point, but at present we don't want to re-read file size since it
	//       will cost additional disk access
	bool bNoBuffer = (GetConfig().get_bool(PP_BFUSENOBUFFERING) && pData->spSrcFile->GetLength64() >= (unsigned long long)GetConfig().get_signed_num(PP_BFBOUNDARYLIMIT));

	// first open the source file and handle any failures
	hSrc = OpenSourceFileFB(pData->spSrcFile, bNoBuffer);
	if(hSrc == INVALID_HANDLE_VALUE)
	{
		// invalid handle = operation skipped by user
		m_localStats.IncreaseProcessedSize(pData->spSrcFile->GetLength64() - m_tTaskProgressInfo.GetCurrentFileProcessedSize());
		pData->bProcessed = false;
		return;
	}

	// change attributes of a dest file
	// NOTE: probably should be removed from here and report problems with read-only files
	//       directly to the user (as feedback request)
	if(!GetConfig().get_bool(PP_CMPROTECTROFILES))
		SetFileAttributes(pData->strDstFile, FILE_ATTRIBUTE_NORMAL);

	// open destination file, handle the failures and possibly existence of the destination file
	unsigned long long ullSeekTo = 0;
	bool bDstFileFreshlyCreated = false;

	if(m_tTaskProgressInfo.GetCurrentFileProcessedSize() == 0)
	{
		// open destination file for case, when we start operation on this file (i.e. it is not resume of the
		// old operation)
		hDst = OpenDestinationFileFB(pData->strDstFile, bNoBuffer, pData->spSrcFile, ullSeekTo, bDstFileFreshlyCreated);
		if(hDst == INVALID_HANDLE_VALUE)
		{
			m_localStats.IncreaseProcessedSize(pData->spSrcFile->GetLength64() - m_tTaskProgressInfo.GetCurrentFileProcessedSize());
			pData->bProcessed = false;
			return;
		}
	}
	else
	{
		// we are resuming previous operation
		hDst = OpenExistingDestinationFileFB(pData->strDstFile, bNoBuffer);
		if(hDst == INVALID_HANDLE_VALUE)
		{
			m_localStats.IncreaseProcessedSize(pData->spSrcFile->GetLength64() - m_tTaskProgressInfo.GetCurrentFileProcessedSize());
			pData->bProcessed = false;
			return;
		}

		ullSeekTo = m_tTaskProgressInfo.GetCurrentFileProcessedSize();
	}

	if(!pData->bOnlyCreate)
	{
		// seek to the position where copying will start
		if(ullSeekTo != 0)		// src and dst files exists, requested resume at the specified index
		{
			// try to move file pointers to the end
			ULONGLONG ullMove = (bNoBuffer ? ROUNDDOWN(ullSeekTo, MAXSECTORSIZE) : ullSeekTo);

			if(!SetFilePointerFB(hSrc, ullMove, pData->spSrcFile->GetFullFilePath()) || !SetFilePointerFB(hDst, ullMove, pData->strDstFile))
			{
				// with either first or second seek we got 'skip' answer...
				m_localStats.IncreaseProcessedSize(pData->spSrcFile->GetLength64() - m_tTaskProgressInfo.GetCurrentFileProcessedSize());
				pData->bProcessed = false;
				return;
			}

			m_tTaskProgressInfo.IncreaseCurrentFileProcessedSize(ullMove);
			m_localStats.IncreaseProcessedSize(ullMove);
		}

		// if the destination file already exists - truncate it to the current file position
		if(!bDstFileFreshlyCreated)
		{
			// if destination file was opened (as opposed to newly created)
			if(!SetEndOfFileFB(hDst, pData->strDstFile))
			{
				pData->bProcessed = false;
				return;
			}
		}

		// copying
		unsigned long ulToRead = 0;
		unsigned long ulRead = 0;
		unsigned long ulWritten = 0;
		int iBufferIndex = 0;
		bool bLastPart = false;

		do
		{
			// kill flag checks
			if(m_workerThread.KillRequested())
			{
				// log
				fmt.SetFormat(_T("Kill request while main copying file %srcpath -> %dstpath"));
				fmt.SetParam(_t("%srcpath"), pData->spSrcFile->GetFullFilePath());
				fmt.SetParam(_t("%dstpath"), pData->strDstFile);
				m_log.logi(fmt);
				throw new CProcessingException(E_KILL_REQUEST);
			}

			// recreate buffer if needed
			if(!(*pData->dbBuffer.GetSizes() == *GetBufferSizes()))
			{
				// log
				const BUFFERSIZES* pbs1 = pData->dbBuffer.GetSizes();
				const BUFFERSIZES* pbs2 = GetBufferSizes();

				fmt.SetFormat(_T("Changing buffer size from [Def:%defsize, One:%onesize, Two:%twosize, CD:%cdsize, LAN:%lansize] to [Def:%defsize2, One:%onesize2, Two:%twosize2, CD:%cdsize2, LAN:%lansize2] wile copying %srcfile -> %dstfile (CustomCopyFile)"));

				fmt.SetParam(_t("%defsize"), pbs1->m_uiDefaultSize);
				fmt.SetParam(_t("%onesize"), pbs1->m_uiOneDiskSize);
				fmt.SetParam(_t("%twosize"), pbs1->m_uiTwoDisksSize);
				fmt.SetParam(_t("%cdsize"), pbs1->m_uiCDSize);
				fmt.SetParam(_t("%lansize"), pbs1->m_uiLANSize);
				fmt.SetParam(_t("%defsize2"), pbs2->m_uiDefaultSize);
				fmt.SetParam(_t("%onesize2"), pbs2->m_uiOneDiskSize);
				fmt.SetParam(_t("%twosize2"), pbs2->m_uiTwoDisksSize);
				fmt.SetParam(_t("%cdsize2"), pbs2->m_uiCDSize);
				fmt.SetParam(_t("%lansize2"), pbs2->m_uiLANSize);
				fmt.SetParam(_t("%srcfile"), pData->spSrcFile->GetFullFilePath());
				fmt.SetParam(_t("%dstfile"), pData->strDstFile);

				m_log.logi(fmt);
				SetBufferSizes(pData->dbBuffer.Create(GetBufferSizes()));
			}

			// establish count of data to read
			if(GetBufferSizes()->m_bOnlyDefault)
				iBufferIndex = BI_DEFAULT;
			else
				iBufferIndex = pData->spSrcFile->GetBufferIndex(m_dpDestPath);

			ulToRead = bNoBuffer ? ROUNDUP(pData->dbBuffer.GetSizes()->m_auiSizes[iBufferIndex], MAXSECTORSIZE) : pData->dbBuffer.GetSizes()->m_auiSizes[iBufferIndex];

			// read data from file to buffer
			if(!ReadFileFB(hSrc, pData->dbBuffer, ulToRead, ulRead, pData->spSrcFile->GetFullFilePath()))
			{
				m_localStats.IncreaseProcessedSize(pData->spSrcFile->GetLength64() - m_tTaskProgressInfo.GetCurrentFileProcessedSize());
				pData->bProcessed = false;
				return;
			}

			if(ulRead > 0)
			{
				// determine if this is the last chunk of data we could get from the source file (EOF condition)
				bLastPart = (ulToRead != ulRead);

				// handle not aligned part at the end of file when no buffering is enabled
				if(bNoBuffer && bLastPart)
				{
					// count of data read from the file is less than requested - we're at the end of source file
					// and this is the operation with system buffering turned off

					// write as much as possible to the destination file with no buffering
					// NOTE: as an alternative, we could write more data to the destination file and then truncate the file
					unsigned long ulDataToWrite = ROUNDDOWN(ulRead, MAXSECTORSIZE);
					if(ulDataToWrite > 0)
					{
						if(!WriteFileFB(hDst, pData->dbBuffer, ulDataToWrite, ulWritten, pData->strDstFile))
						{
							m_localStats.IncreaseProcessedSize(pData->spSrcFile->GetLength64() - m_tTaskProgressInfo.GetCurrentFileProcessedSize());
							pData->bProcessed = false;
							return;
						}

						// increase count of processed data
						m_tTaskProgressInfo.IncreaseCurrentFileProcessedSize(ulWritten);
						m_localStats.IncreaseProcessedSize(ulWritten);

						// calculate count of bytes left to be written
						ulRead -= ulWritten;

						// now remove part of data from buffer (ulWritten bytes)
						pData->dbBuffer.CutDataFromBuffer(ulWritten);
					}

					// close and re-open the destination file with buffering option for append
					hDst.Close();

					// are there any more data to be written?
					if(ulRead != 0)
					{
						// re-open the destination file, this time with standard buffering to allow writing not aligned part of file data
						hDst = OpenExistingDestinationFileFB(pData->strDstFile, false);
						if(hDst == INVALID_HANDLE_VALUE)
						{
							m_localStats.IncreaseProcessedSize(pData->spSrcFile->GetLength64() - m_tTaskProgressInfo.GetCurrentFileProcessedSize());
							pData->bProcessed = false;
							return;
						}

						// move file pointer to the end of destination file
						if(!SetFilePointerFB(hDst, m_tTaskProgressInfo.GetCurrentFileProcessedSize(), pData->strDstFile))
						{
							// with either first or second seek we got 'skip' answer...
							m_localStats.IncreaseProcessedSize(pData->spSrcFile->GetLength64() - m_tTaskProgressInfo.GetCurrentFileProcessedSize());
							pData->bProcessed = false;
							return;
						}
					}
				}

				// write
				if(ulRead != 0)
				{
					if(!WriteFileFB(hDst, pData->dbBuffer, ulRead, ulWritten, pData->strDstFile))
					{
						m_localStats.IncreaseProcessedSize(pData->spSrcFile->GetLength64() - m_tTaskProgressInfo.GetCurrentFileProcessedSize());
						pData->bProcessed = false;
						return;
					}

					// increase count of processed data
					m_tTaskProgressInfo.IncreaseCurrentFileProcessedSize(ulRead);
					m_localStats.IncreaseProcessedSize(ulRead);
				}
			}
		}
		while(ulRead != 0 && !bLastPart);
	}
	else
	{
		// we don't copy contents, but need to increase processed size
		m_localStats.IncreaseProcessedSize(pData->spSrcFile->GetLength64() - m_tTaskProgressInfo.GetCurrentFileProcessedSize());
	}

	pData->bProcessed = true;
	m_tTaskProgressInfo.SetCurrentFileProcessedSize(0);
}

// function processes files/folders
void CTask::ProcessFiles()
{
	// log
	m_log.logi(_T("Processing files/folders (ProcessFiles)"));

	// count how much has been done (updates also a member in CTaskArray)
	CalculateProcessedSize();

	// begin at index which wasn't processed previously
	size_t stSize = m_files.GetSize();
	bool bIgnoreFolders = (GetStatus(ST_SPECIAL_MASK) & ST_IGNORE_DIRS) != 0;
	bool bForceDirectories = (GetStatus(ST_SPECIAL_MASK) & ST_FORCE_DIRS) != 0;
	const CDestPath& dpDestPath = GetDestPath();

	// create a buffer of size m_nBufferSize
	CUSTOM_COPY_PARAMS ccp;
	ccp.bProcessed = false;
	ccp.bOnlyCreate=(GetStatus(ST_SPECIAL_MASK) & ST_IGNORE_CONTENT) != 0;
	ccp.dbBuffer.Create(GetBufferSizes());
	ccp.pDestPath = &dpDestPath;

	// helpers
	DWORD dwLastError = 0;

	// log
	const BUFFERSIZES* pbs = ccp.dbBuffer.GetSizes();

	ictranslate::CFormat fmt;
	fmt.SetFormat(_T("Processing files/folders (ProcessFiles):\r\n\tOnlyCreate: %create\r\n\tBufferSize: [Def:%defsize, One:%onesize, Two:%twosize, CD:%cdsize, LAN:%lansize]\r\n\tFiles/folders count: %filecount\r\n\tIgnore Folders: %ignorefolders\r\n\tDest path: %dstpath\r\n\tCurrent index (0-based): %currindex"));
	fmt.SetParam(_t("%create"), ccp.bOnlyCreate);
	fmt.SetParam(_t("%defsize"), pbs->m_uiDefaultSize);
	fmt.SetParam(_t("%onesize"), pbs->m_uiOneDiskSize);
	fmt.SetParam(_t("%twosize"), pbs->m_uiTwoDisksSize);
	fmt.SetParam(_t("%cdsize"), pbs->m_uiCDSize);
	fmt.SetParam(_t("%lansize"), pbs->m_uiLANSize);
	fmt.SetParam(_t("%filecount"), stSize);
	fmt.SetParam(_t("%ignorefolders"), bIgnoreFolders);
	fmt.SetParam(_t("%dstpath"), dpDestPath.GetPath());
	fmt.SetParam(_t("%currindex"), m_tTaskProgressInfo.GetCurrentIndex());

	m_log.logi(fmt);

	for(size_t stIndex = m_tTaskProgressInfo.GetCurrentIndex(); stIndex < stSize; stIndex++)
	{
		// should we kill ?
		if(m_workerThread.KillRequested())
		{
			// log
			m_log.logi(_T("Kill request while processing file in ProcessFiles"));
			throw new CProcessingException(E_KILL_REQUEST);
		}

		// update m_stCurrentIndex, getting current CFileInfo
		CFileInfoPtr spFileInfo = m_files.GetAt(m_tTaskProgressInfo.GetCurrentIndex());

		// set dest path with filename
		ccp.strDstFile = spFileInfo->GetDestinationPath(dpDestPath.GetPath(), ((int)bForceDirectories) << 1 | (int)bIgnoreFolders);

		// are the files/folders lie on the same partition ?
		bool bMove = GetOperationType() == eOperation_Move;
		if(bMove && dpDestPath.GetDriveNumber() != -1 && dpDestPath.GetDriveNumber() == spFileInfo->GetDriveNumber() && spFileInfo->GetMove())
		{
			bool bRetry = true;
			if(bRetry && !MoveFile(spFileInfo->GetFullFilePath(), ccp.strDstFile))
			{
				dwLastError=GetLastError();
				//log
				fmt.SetFormat(_T("Error %errno while calling MoveFile %srcpath -> %dstpath (ProcessFiles)"));
				fmt.SetParam(_t("%errno"), dwLastError);
				fmt.SetParam(_t("%srcpath"), spFileInfo->GetFullFilePath());
				fmt.SetParam(_t("%dstpath"), ccp.strDstFile);
				m_log.loge(fmt);

				CString strSrcFile = spFileInfo->GetFullFilePath();
				CString strDstFile = ccp.strDstFile;
				FEEDBACK_FILEERROR ferr = { (PCTSTR)strSrcFile, (PCTSTR)strDstFile, eFastMoveError, dwLastError };
				CFeedbackHandler::EFeedbackResult frResult = (CFeedbackHandler::EFeedbackResult)m_piFeedbackHandler->RequestFeedback(CFeedbackHandler::eFT_FileError, &ferr);
				switch(frResult)
				{
				case CFeedbackHandler::eResult_Cancel:
					throw new CProcessingException(E_CANCEL);
					break;
				case CFeedbackHandler::eResult_Retry:
					continue;
					break;
				case CFeedbackHandler::eResult_Pause:
					throw new CProcessingException(E_PAUSE);
					break;
				case CFeedbackHandler::eResult_Skip:
					bRetry = false;
					break;		// just do nothing
				default:
					BOOST_ASSERT(FALSE);		// unknown result
					throw new CProcessingException(E_ERROR, 0, _t("Unknown feedback result type"));
				}
			}
			else
				spFileInfo->SetFlags(FIF_PROCESSED, FIF_PROCESSED);
		}
		else
		{
			// if folder - create it
			if(spFileInfo->IsDirectory())
			{
				bool bRetry = true;
				if(bRetry && !CreateDirectory(ccp.strDstFile, NULL) && (dwLastError=GetLastError()) != ERROR_ALREADY_EXISTS )
				{
					// log
					fmt.SetFormat(_T("Error %errno while calling CreateDirectory %path (ProcessFiles)"));
					fmt.SetParam(_t("%errno"), dwLastError);
					fmt.SetParam(_t("%path"), ccp.strDstFile);
					m_log.loge(fmt);

					CString strFile = ccp.strDstFile;
					FEEDBACK_FILEERROR ferr = { (PCTSTR)strFile, NULL, eCreateError, dwLastError };
					CFeedbackHandler::EFeedbackResult frResult = (CFeedbackHandler::EFeedbackResult)m_piFeedbackHandler->RequestFeedback(CFeedbackHandler::eFT_FileError, &ferr);
					switch(frResult)
					{
					case CFeedbackHandler::eResult_Cancel:
						throw new CProcessingException(E_CANCEL);
						break;
					case CFeedbackHandler::eResult_Retry:
						continue;
						break;
					case CFeedbackHandler::eResult_Pause:
						throw new CProcessingException(E_PAUSE);
						break;
					case CFeedbackHandler::eResult_Skip:
						bRetry = false;
						break;		// just do nothing
					default:
						BOOST_ASSERT(FALSE);		// unknown result
						throw new CProcessingException(E_ERROR, 0, _t("Unknown feedback result type"));
					}
				}

				m_localStats.IncreaseProcessedSize(spFileInfo->GetLength64());
				spFileInfo->SetFlags(FIF_PROCESSED, FIF_PROCESSED);
			}
			else
			{
				// start copying/moving file
				ccp.spSrcFile = spFileInfo;
				ccp.bProcessed = false;

				// kopiuj dane
				CustomCopyFile(&ccp);
				spFileInfo->SetFlags(ccp.bProcessed ? FIF_PROCESSED : 0, FIF_PROCESSED);

				// if moving - delete file (only if config flag is set)
				if(bMove && spFileInfo->GetFlags() & FIF_PROCESSED && !GetConfig().get_bool(PP_CMDELETEAFTERFINISHED))
				{
					if(!GetConfig().get_bool(PP_CMPROTECTROFILES))
						SetFileAttributes(spFileInfo->GetFullFilePath(), FILE_ATTRIBUTE_NORMAL);
					DeleteFile(spFileInfo->GetFullFilePath());	// there will be another try later, so I don't check
					// if succeeded
				}
			}

			// set a time
			if(GetConfig().get_bool(PP_CMSETDESTDATE))
				SetFileDirectoryTime(ccp.strDstFile, spFileInfo); // no error checking (but most probably it should be checked)

			// attributes
			if(GetConfig().get_bool(PP_CMSETDESTATTRIBUTES))
				SetFileAttributes(ccp.strDstFile, spFileInfo->GetAttributes());	// as above
		}

		m_tTaskProgressInfo.SetCurrentIndex(stIndex + 1);
	}

	// delete buffer - it's not needed
	ccp.dbBuffer.Delete();

	// change status
	if(GetOperationType() == eOperation_Move)
	{
		SetStatus(ST_DELETING, ST_STEP_MASK);
		// set the index to 0 before deleting
		m_tTaskProgressInfo.SetCurrentIndex(0);
	}
	else
	{
		SetTaskState(eTaskState_Finished);

		// to look better - increase current index by 1
		m_tTaskProgressInfo.SetCurrentIndex(stSize);
	}
	// log
	m_log.logi(_T("Finished processing in ProcessFiles"));
}

void CTask::CheckForWaitState()
{
	// limiting operation count
	SetTaskState(eTaskState_Waiting);
	bool bContinue = false;
	while(!bContinue)
	{
		if(CanBegin())
		{
			SetTaskState(eTaskState_Processing);
			bContinue = true;

			m_log.logi(_T("Finished waiting for begin permission"));

			//			return; // skips sleep and kill flag checking
		}

		Sleep(50);	// not to make it too hard for processor

		if(m_workerThread.KillRequested())
		{
			// log
			m_log.logi(_T("Kill request while waiting for begin permission (wait state)"));
			throw new CProcessingException(E_KILL_REQUEST);
		}
	}
}

DWORD WINAPI CTask::DelegateThreadProc(LPVOID pParam)
{
	BOOST_ASSERT(pParam);
	if(!pParam)
		return 1;

	CTask* pTask = (CTask*)pParam;
	return pTask->ThrdProc();
}

DWORD CTask::ThrdProc()
{
	tstring_t strPath = GetTaskPath();
	strPath += GetUniqueName()+_T(".log");

	m_log.init(strPath.c_str(), 262144, icpf::log_file::level_debug, false, false);

	OnBeginOperation();

	// set thread boost
	HANDLE hThread=GetCurrentThread();
	::SetThreadPriorityBoost(hThread, GetConfig().get_bool(PP_CMDISABLEPRIORITYBOOST));

	ictranslate::CFormat fmt;
	try
	{
		// to make the value stable
		bool bReadTasksSize = GetConfig().get_bool(PP_CMREADSIZEBEFOREBLOCKING);

		if(!bReadTasksSize)
			CheckForWaitState();	// operation limiting

		// start tracking time
		m_localStats.EnableTimeTracking();

		// search for files if needed
		if((GetStatus(ST_STEP_MASK) == ST_NULL_STATUS || GetStatus(ST_STEP_MASK) == ST_SEARCHING))
		{
			// get rid of info about processed sizes
			m_localStats.SetProcessedSize(0);
			m_localStats.SetTotalSize(0);

			// start searching
			RecurseDirectories();
		}

		// check for free space
		ull_t ullNeededSize = 0, ullAvailableSize = 0;
		bool bRetry = false;

		do
		{
			bRetry = false;

			m_log.logi(_T("Checking for free space on destination disk..."));

			if(!GetRequiredFreeSpace(&ullNeededSize, &ullAvailableSize))
			{
				fmt.SetFormat(_T("Not enough free space on disk - needed %needsize bytes for data, available: %availablesize bytes."));
				fmt.SetParam(_t("%needsize"), ullNeededSize);
				fmt.SetParam(_t("%availablesize"), ullAvailableSize);
				m_log.logw(fmt);

				if(GetClipboardDataSize() > 0)
				{
					CString strSrcPath = GetClipboardData(0)->GetPath();
					CString strDstPath = GetDestPath().GetPath();
					FEEDBACK_NOTENOUGHSPACE feedStruct = { ullNeededSize, (PCTSTR)strSrcPath, (PCTSTR)strDstPath };
					CFeedbackHandler::EFeedbackResult frResult = (CFeedbackHandler::EFeedbackResult)m_piFeedbackHandler->RequestFeedback(CFeedbackHandler::eFT_NotEnoughSpace, &feedStruct);

					// default
					switch (frResult)
					{
					case CFeedbackHandler::eResult_Cancel:
						{
							m_log.logi(_T("Cancel request while checking for free space on disk."));
							throw new CProcessingException(E_CANCEL);
							break;
						}
					case CFeedbackHandler::eResult_Retry:
						m_log.logi(_T("Retrying to read drive's free space..."));
						bRetry = true;
						break;
					case CFeedbackHandler::eResult_Skip:
						m_log.logi(_T("Ignored warning about not enough place on disk to copy data."));
						break;
					default:
						BOOST_ASSERT(FALSE);		// unknown result
						throw new CProcessingException(E_ERROR, 0, _t("Unknown feedback result type"));
						break;
					}
				}
			}
		}
		while(bRetry);

		if(bReadTasksSize)
		{
			m_localStats.DisableTimeTracking();

			CheckForWaitState();

			m_localStats.EnableTimeTracking();
		}

		// Phase II - copying/moving
		if(GetStatus(ST_STEP_MASK) == ST_COPYING)
		{
			// decrease processed in ctaskarray - the rest will be done in ProcessFiles
			//m_rtGlobalStats.DecreaseGlobalProcessedSize(GetProcessedSize());
			ProcessFiles();
		}

		// deleting data - III phase
		if(GetStatus(ST_STEP_MASK) == ST_DELETING)
			DeleteFiles();

		// refresh time
		m_localStats.DisableTimeTracking();

		// save progress before killed
		Store(false);

		// we are ending
		m_localStats.MarkTaskAsNotRunning();

		// play sound
		m_piFeedbackHandler->RequestFeedback(CFeedbackHandler::eFT_OperationFinished, NULL);

		OnEndOperation();
	}
	catch(CProcessingException* e)
	{
		// refresh time
		m_localStats.DisableTimeTracking();

		// log
		fmt.SetFormat(_T("Caught exception in ThrdProc [last error: %errno, type: %type]"));
		fmt.SetParam(_t("%errno"), e->m_dwError);
		fmt.SetParam(_t("%type"), e->m_iType);
		m_log.loge(fmt);

		if(e->m_iType == E_ERROR)
			m_piFeedbackHandler->RequestFeedback(CFeedbackHandler::eFT_OperationError, NULL);

		// perform some adjustments depending on exception type
		switch(e->m_iType)
		{
		case E_ERROR:
			SetTaskState(eTaskState_Error);
			break;
		case E_CANCEL:
			SetTaskState(eTaskState_Cancelled);
			break;
		case E_PAUSE:
			SetTaskState(eTaskState_Paused);
			break;
		case E_KILL_REQUEST:
			{
				// the only operation 
				if(GetTaskState() == eTaskState_Waiting)
					SetTaskState(eTaskState_Processing);
			}
		default:
			BOOST_ASSERT(false);    // unhandled case
		}

		// change flags and calls cleanup for a task
		switch(GetStatus(ST_STEP_MASK))
		{
		case ST_NULL_STATUS:
		case ST_SEARCHING:
			// get rid of m_files contents
			m_files.Clear();

			// save state of a task
			Store(true);
			Store(false);

			break;
		case ST_COPYING:
		case ST_DELETING:
			Store(false);
			break;
		}

		m_localStats.MarkTaskAsNotRunning();
		SetContinueFlag(false);
		SetForceFlag(false);

		OnEndOperation();

		delete e;

		return 0xffffffff;	// almost like -1
	}

	return 0;
}

void CTask::OnBeginOperation()
{
	CTime tm=CTime::GetCurrentTime();

	ictranslate::CFormat fmt;
	fmt.SetFormat(_T("\r\n# COPYING THREAD STARTED #\r\nBegan processing data (dd:mm:yyyy) %day.%month.%year at %hour:%minute.%second"));
	fmt.SetParam(_t("%year"), tm.GetYear());
	fmt.SetParam(_t("%month"), tm.GetMonth());
	fmt.SetParam(_t("%day"), tm.GetDay());
	fmt.SetParam(_t("%hour"), tm.GetHour());
	fmt.SetParam(_t("%minute"), tm.GetMinute());
	fmt.SetParam(_t("%second"), tm.GetSecond());
	m_log.logi(fmt);
}

void CTask::OnEndOperation()
{
	CTime tm=CTime::GetCurrentTime();

	ictranslate::CFormat fmt;
	fmt.SetFormat(_T("Finished processing data (dd:mm:yyyy) %day.%month.%year at %hour:%minute.%second"));
	fmt.SetParam(_t("%year"), tm.GetYear());
	fmt.SetParam(_t("%month"), tm.GetMonth());
	fmt.SetParam(_t("%day"), tm.GetDay());
	fmt.SetParam(_t("%hour"), tm.GetHour());
	fmt.SetParam(_t("%minute"), tm.GetMinute());
	fmt.SetParam(_t("%second"), tm.GetSecond());
	m_log.logi(fmt);
}

void CTask::RequestStopThread()
{
	m_workerThread.SignalThreadToStop();
}

////////////////////////////////////////////////////////////////////////////////
// CTaskArray members
CTaskArray::CTaskArray() :
	m_piFeedbackFactory(NULL),
	m_stNextSessionUniqueID(NO_TASK_SESSION_UNIQUE_ID + 1)
{
}

CTaskArray::~CTaskArray()
{
	// NOTE: do not delete the feedback factory, since we are not responsible for releasing it
}

void CTaskArray::Create(chcore::IFeedbackHandlerFactory* piFeedbackHandlerFactory)
{
	BOOST_ASSERT(piFeedbackHandlerFactory);
	
	m_piFeedbackFactory = piFeedbackHandlerFactory;
}

CTaskPtr CTaskArray::CreateTask()
{
	BOOST_ASSERT(m_piFeedbackFactory);
	if(!m_piFeedbackFactory)
		return CTaskPtr();
	
	chcore::IFeedbackHandler* piHandler = m_piFeedbackFactory->Create();
	if(!piHandler)
		return CTaskPtr();

	BOOST_ASSERT(m_stNextSessionUniqueID != NO_TASK_SESSION_UNIQUE_ID);
	CTaskPtr spTask(boost::make_shared<CTask>(piHandler, m_stNextSessionUniqueID++));

	// NO_TASK_SESSION_UNIQUE_ID is a special value so it should not be used to identify tasks
	if(m_stNextSessionUniqueID == NO_TASK_SESSION_UNIQUE_ID)
		++m_stNextSessionUniqueID;

	return spTask;
}

size_t CTaskArray::GetSize() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_vTasks.size();
}

CTaskPtr CTaskArray::GetAt(size_t nIndex) const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	
	_ASSERTE(nIndex >= 0 && nIndex < m_vTasks.size());
	if(nIndex >= m_vTasks.size())
		THROW(_t("Invalid argument"), 0, 0, 0);
	
	return m_vTasks.at(nIndex);
}

CTaskPtr CTaskArray::GetTaskBySessionUniqueID(size_t stSessionUniqueID) const
{
	if(stSessionUniqueID == NO_TASK_SESSION_UNIQUE_ID)
		return CTaskPtr();

	CTaskPtr spFoundTask;

	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	BOOST_FOREACH(const CTaskPtr& spTask, m_vTasks)
	{
		if(spTask->GetSessionUniqueID() == stSessionUniqueID)
		{
			spFoundTask = spTask;
			break;
		}
	}
	
	return spFoundTask;
}

size_t CTaskArray::Add(const CTaskPtr& spNewTask)
{
	if(!spNewTask)
		THROW(_t("Invalid argument"), 0, 0, 0);
	
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	// here we know load succeeded
	spNewTask->SetTaskPath(m_strTasksDir.c_str());
	
	m_vTasks.push_back(spNewTask);

	spNewTask->OnRegisterTask(m_globalStats);

	return m_vTasks.size() - 1;
}

void CTaskArray::RemoveAt(size_t stIndex, size_t stCount)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	
	_ASSERTE(stIndex >= m_vTasks.size() || stIndex + stCount > m_vTasks.size());
	if(stIndex >= m_vTasks.size() || stIndex + stCount > m_vTasks.size())
		THROW(_t("Invalid argument"), 0, 0, 0);
	
	for(std::vector<CTaskPtr>::iterator iterTask = m_vTasks.begin() + stIndex; iterTask != m_vTasks.begin() + stIndex + stCount; ++iterTask)
	{
		CTaskPtr& spTask = *iterTask;
		
		// kill task if needed
		spTask->KillThread();

		spTask->OnUnregisterTask();
	}

	// remove elements from array
	m_vTasks.erase(m_vTasks.begin() + stIndex, m_vTasks.begin() + stIndex + stCount);
}

void CTaskArray::RemoveAll()
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);

	StopAllTasksNL();

	m_vTasks.clear();
}

void CTaskArray::RemoveAllFinished()
{
	std::vector<CTaskPtr> vTasksToRemove;

	m_lock.lock();
	
	size_t stIndex = m_vTasks.size();
	while(stIndex--)
	{
		CTaskPtr spTask = m_vTasks.at(stIndex);
		
		// delete only when the thread is finished
		if((spTask->GetTaskState() == eTaskState_Finished || spTask->GetTaskState() == eTaskState_Cancelled))
		{
			spTask->OnUnregisterTask();

			vTasksToRemove.push_back(spTask);
			m_vTasks.erase(m_vTasks.begin() + stIndex);
		}
	}
	
	m_lock.unlock();

	BOOST_FOREACH(CTaskPtr& spTask, vTasksToRemove)
	{
		// delete associated files
		spTask->DeleteProgress(m_strTasksDir.c_str());
	}
}

void CTaskArray::RemoveFinished(const CTaskPtr& spSelTask)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	
	// this might be optimized by copying tasks to a local table in critical section, and then deleting progress files outside of the critical section
	for(std::vector<CTaskPtr>::iterator iterTask = m_vTasks.begin(); iterTask != m_vTasks.end(); ++iterTask)
	{
		CTaskPtr& spTask = *iterTask;
		
		if(spTask == spSelTask && (spTask->GetTaskState() == eTaskState_Finished || spTask->GetTaskState() == eTaskState_Cancelled))
		{
			// kill task if needed
			spTask->KillThread();

			spTask->OnUnregisterTask();

			// delete associated files
			spTask->DeleteProgress(m_strTasksDir.c_str());
			
			m_vTasks.erase(iterTask);
			
			return;
		}
	}
}

void CTaskArray::StopAllTasks()
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	
	StopAllTasksNL();
}

void CTaskArray::ResumeWaitingTasks(size_t stMaxRunningTasks)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);

	if(stMaxRunningTasks == 0 || m_globalStats.GetRunningTasksCount() < stMaxRunningTasks)
	{
		BOOST_FOREACH(CTaskPtr& spTask, m_vTasks)
		{
			// turn on some thread - find something with wait state
			if(spTask->GetTaskState() == eTaskState_Waiting && (stMaxRunningTasks == 0 || m_globalStats.GetRunningTasksCount() < stMaxRunningTasks))
			{
				spTask->m_localStats.MarkTaskAsRunning();
				spTask->SetContinueFlagNL(true);
			}
		}
	}
}

void CTaskArray::SaveData()
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	BOOST_FOREACH(CTaskPtr& spTask, m_vTasks)
	{
		spTask->Store(true);
	}
}

void CTaskArray::SaveProgress()
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	BOOST_FOREACH(CTaskPtr& spTask, m_vTasks)
	{
		spTask->Store(false);
	}
}

void CTaskArray::LoadDataProgress()
{
	CFileFind finder;
	CTaskPtr spTask;
	CString strPath;
	
	BOOL bWorking=finder.FindFile(CString(m_strTasksDir.c_str())+_T("*.atd"));
	while(bWorking)
	{
		bWorking = finder.FindNextFile();
		
		// load data
		spTask = CreateTask();
		try
		{
			strPath = finder.GetFilePath();
			
			spTask->Load(strPath, true);
			
			strPath = strPath.Left(strPath.GetLength() - 4);
			strPath += _T(".atp");
			
			spTask->Load(strPath, false);
			
			// add read task to array
			Add(spTask);
		}
		catch(std::exception& e)
		{
			CString strFmt;
			strFmt.Format(_T("Cannot load task data: %s (reason: %S)"), strPath, e.what());
			LOG_ERROR(strFmt);
		}
		catch(icpf::exception& e)
		{
			tchar_t szBuffer[65536];
			e.get_info(szBuffer, 65536);
			szBuffer[65535] = _T('\0');

			CString strFmt;
			strFmt.Format(_T("Cannot load task data: %s (reason: %s)"), strPath, szBuffer);
			LOG_ERROR(strFmt);
		}
	}
	finder.Close();
}

void CTaskArray::TasksBeginProcessing()
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	BOOST_FOREACH(CTaskPtr& spTask, m_vTasks)
	{
		spTask->BeginProcessing();
	}
}

void CTaskArray::TasksPauseProcessing()
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	BOOST_FOREACH(CTaskPtr& spTask, m_vTasks)
	{
		spTask->PauseProcessing();
	}
}

void CTaskArray::TasksResumeProcessing()
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	BOOST_FOREACH(CTaskPtr& spTask, m_vTasks)
	{
		spTask->ResumeProcessing();
	}
}

void CTaskArray::TasksRestartProcessing()
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	BOOST_FOREACH(CTaskPtr& spTask, m_vTasks)
	{
		spTask->RestartProcessing();
	}
}

bool CTaskArray::TasksRetryProcessing()
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	bool bChanged=false;
	BOOST_FOREACH(CTaskPtr& spTask, m_vTasks)
	{
		if(spTask->RetryProcessing())
			bChanged = true;
	}
	
	return bChanged;
}

void CTaskArray::TasksCancelProcessing()
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	BOOST_FOREACH(CTaskPtr& spTask, m_vTasks)
	{
		spTask->CancelProcessing();
	}
}

ull_t CTaskArray::GetPosition()
{
	return m_globalStats.GetGlobalProcessedSize();
}

ull_t CTaskArray::GetRange()
{
	return m_globalStats.GetGlobalTotalSize();
}

int CTaskArray::GetPercent()
{
	return m_globalStats.GetProgressPercents();
}

bool CTaskArray::AreAllFinished()
{
	bool bFlag=true;
	
	if(m_globalStats.GetRunningTasksCount() != 0)
		bFlag = false;
	else
	{
		boost::shared_lock<boost::shared_mutex> lock(m_lock);
		BOOST_FOREACH(CTaskPtr& spTask, m_vTasks)
		{
			ETaskCurrentState eState = spTask->GetTaskState();
			bFlag = (eState == eTaskState_Finished || eState == eTaskState_Cancelled || eState == eTaskState_Paused || eState == eTaskState_Error);

			if(!bFlag)
				break;
		}
	}
	
	return bFlag;
}

void CTaskArray::SetTasksDir(const tchar_t* pszPath)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_strTasksDir = pszPath;
}

void CTaskArray::StopAllTasksNL()
{
	// kill all unfinished tasks - send kill request
	BOOST_FOREACH(CTaskPtr& spTask, m_vTasks)
	{
		spTask->RequestStopThread();
	}
	
	// wait for finishing
	BOOST_FOREACH(CTaskPtr& spTask, m_vTasks)
	{
		spTask->KillThread();
	}
}
