/***************************************************************************
*   Copyright (C) 2001-2010 by Jozef Starosczyk                           *
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

#include <boost/serialization/serialization.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <fstream>

#include "StringHelpers.h"
#include "../common/FileSupport.h"
#include "FeedbackHandler.h"

#include "TTaskConfiguration.h"
#include "TSubTaskContext.h"

// assume max sectors of 4kB (for rounding)
#define MAXSECTORSIZE			4096

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
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_ullGlobalTotalSize += ullModify;
}

void TTasksGlobalStats::DecreaseGlobalTotalSize(unsigned long long ullModify)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_ullGlobalTotalSize -= ullModify;
}

unsigned long long TTasksGlobalStats::GetGlobalTotalSize() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_ullGlobalTotalSize;
}

void TTasksGlobalStats::IncreaseGlobalProcessedSize(unsigned long long ullModify)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_ullGlobalProcessedSize += ullModify;
}

void TTasksGlobalStats::DecreaseGlobalProcessedSize(unsigned long long ullModify)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_ullGlobalProcessedSize -= ullModify;
}

unsigned long long TTasksGlobalStats::GetGlobalProcessedSize() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_ullGlobalProcessedSize;
}

void TTasksGlobalStats::IncreaseGlobalProgressData(unsigned long long ullTasksPosition, unsigned long long ullTasksSize)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_ullGlobalTotalSize += ullTasksSize;
	m_ullGlobalProcessedSize += ullTasksPosition;
}

void TTasksGlobalStats::DecreaseGlobalProgressData(unsigned long long ullTasksPosition, unsigned long long ullTasksSize)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_ullGlobalTotalSize -= ullTasksSize;
	m_ullGlobalProcessedSize -= ullTasksPosition;
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
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	++m_stRunningTasks;
}

void TTasksGlobalStats::DecreaseRunningTasks()
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	--m_stRunningTasks;
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
		if(m_bTaskIsRunning)
			m_prtGlobalStats->DecreaseRunningTasks();
		m_prtGlobalStats = NULL;
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

TTaskBasicProgressInfo::TTaskBasicProgressInfo() :
	m_stCurrentIndex(0),
	m_ullCurrentFileProcessedSize(0),
	m_stSubOperationIndex(0)
{
}

TTaskBasicProgressInfo::~TTaskBasicProgressInfo()
{
}

void TTaskBasicProgressInfo::SetCurrentIndex(size_t stIndex)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_stCurrentIndex = stIndex;
	m_ullCurrentFileProcessedSize = 0;
}

void TTaskBasicProgressInfo::IncreaseCurrentIndex()
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	++m_stCurrentIndex;
	m_ullCurrentFileProcessedSize = 0;
}

size_t TTaskBasicProgressInfo::GetCurrentIndex() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_stCurrentIndex;
}

void TTaskBasicProgressInfo::SetCurrentFileProcessedSize(unsigned long long ullSize)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_ullCurrentFileProcessedSize = ullSize;
}

unsigned long long TTaskBasicProgressInfo::GetCurrentFileProcessedSize() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_ullCurrentFileProcessedSize;
}

void TTaskBasicProgressInfo::IncreaseCurrentFileProcessedSize(unsigned long long ullSizeToAdd)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_ullCurrentFileProcessedSize += ullSizeToAdd;
}

void TTaskBasicProgressInfo::SetSubOperationIndex(size_t stSubOperationIndex)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_stSubOperationIndex = stSubOperationIndex;
}

size_t TTaskBasicProgressInfo::GetSubOperationIndex() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_stSubOperationIndex;
}

void TTaskBasicProgressInfo::IncreaseSubOperationIndex()
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	++m_stSubOperationIndex;
}

////////////////////////////////////////////////////////////////////////////
// CTask members
CTask::CTask(chcore::IFeedbackHandler* piFeedbackHandler, size_t stSessionUniqueID) :
	m_log(),
	m_piFeedbackHandler(piFeedbackHandler),
	m_files(m_arrSourcePaths),
	m_bForce(false),
	m_bContinue(false),
	m_bRareStateModified(false),
	m_bOftenStateModified(false),
	m_stSessionUniqueID(stSessionUniqueID),
	m_localStats(),
	m_eCurrentState(eTaskState_None)
{
	BOOST_ASSERT(piFeedbackHandler);
}

CTask::~CTask()
{
	KillThread();
	if(m_piFeedbackHandler)
		m_piFeedbackHandler->Delete();
}

void CTask::SetTaskDefinition(const TTaskDefinition& rTaskDefinition)
{
	m_tTaskDefinition = rTaskDefinition;

	m_arrSourcePaths.RemoveAll();
	m_files.Clear();

	for(size_t stIndex = 0; stIndex < m_tTaskDefinition.GetSourcePaths().GetCount(); ++stIndex)
	{
		CClipboardEntryPtr spEntry(new CClipboardEntry);
		spEntry->SetPath(m_tTaskDefinition.GetSourcePaths().GetAt(stIndex));

		m_arrSourcePaths.Add(spEntry);
	}
}

void CTask::OnRegisterTask(TTasksGlobalStats& rtGlobalStats)
{
	m_localStats.ConnectGlobalStats(rtGlobalStats);
}

void CTask::OnUnregisterTask()
{
	m_localStats.DisconnectGlobalStats();
}

// m_files
int CTask::ScanDirectory(chcore::TSmartPath pathDirName, size_t stSrcIndex, bool bRecurse, bool bIncludeDirs)
{
	WIN32_FIND_DATA wfd;
	chcore::TSmartPath pathCurrent;

	// append '\\' at the end of path if needed
	pathDirName.AppendIfNotExists(_T("\\"), false);

	pathCurrent = pathDirName + chcore::PathFromString(_T("*"));

	// Iterate through dirs & files
	HANDLE hFind = FindFirstFile(pathCurrent.ToString(), &wfd);
	if(hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if(!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				CFileInfoPtr spFileInfo(boost::make_shared<CFileInfo>());
				spFileInfo->SetClipboard(&m_arrSourcePaths);	// this is the link table (CClipboardArray)
				
				spFileInfo->Create(&wfd, pathDirName, stSrcIndex);
				if(m_afFilters.Match(spFileInfo))
					m_files.AddFileInfo(spFileInfo);
			}
			else if(wfd.cFileName[0] != _T('.') || (wfd.cFileName[1] != _T('\0') && (wfd.cFileName[1] != _T('.') || wfd.cFileName[2] != _T('\0'))))
			{
				if(bIncludeDirs)
				{
					CFileInfoPtr spFileInfo(boost::make_shared<CFileInfo>());
					spFileInfo->SetClipboard(&m_arrSourcePaths);	// this is the link table (CClipboardArray)

					// Add directory itself
					spFileInfo->Create(&wfd, pathDirName, stSrcIndex);
					m_files.AddFileInfo(spFileInfo);
				}
				if(bRecurse)
				{
					pathCurrent = pathDirName + chcore::PathFromString(wfd.cFileName) + chcore::PathFromString(_T("\\"));
					// Recurse Dirs
					ScanDirectory(pathCurrent, stSrcIndex, bRecurse, bIncludeDirs);
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

void CTask::SetBufferSizes(const BUFFERSIZES& bsSizes)
{
	m_tTaskDefinition.GetConfiguration().DelayNotifications();
	SetTaskPropValue<eTO_DefaultBufferSize>(m_tTaskDefinition.GetConfiguration(), bsSizes.m_uiDefaultSize);
	SetTaskPropValue<eTO_OneDiskBufferSize>(m_tTaskDefinition.GetConfiguration(), bsSizes.m_uiOneDiskSize);
	SetTaskPropValue<eTO_TwoDisksBufferSize>(m_tTaskDefinition.GetConfiguration(), bsSizes.m_uiTwoDisksSize);
	SetTaskPropValue<eTO_CDBufferSize>(m_tTaskDefinition.GetConfiguration(), bsSizes.m_uiCDSize);
	SetTaskPropValue<eTO_LANBufferSize>(m_tTaskDefinition.GetConfiguration(), bsSizes.m_uiLANSize);
	SetTaskPropValue<eTO_UseOnlyDefaultBuffer>(m_tTaskDefinition.GetConfiguration(), bsSizes.m_bOnlyDefault);
	m_tTaskDefinition.GetConfiguration().ResumeNotifications();
}

void CTask::GetBufferSizes(BUFFERSIZES& bsSizes)
{
	bsSizes.m_uiDefaultSize = GetTaskPropValue<eTO_DefaultBufferSize>(m_tTaskDefinition.GetConfiguration());
	bsSizes.m_uiOneDiskSize = GetTaskPropValue<eTO_OneDiskBufferSize>(m_tTaskDefinition.GetConfiguration());
	bsSizes.m_uiTwoDisksSize = GetTaskPropValue<eTO_TwoDisksBufferSize>(m_tTaskDefinition.GetConfiguration());
	bsSizes.m_uiCDSize = GetTaskPropValue<eTO_CDBufferSize>(m_tTaskDefinition.GetConfiguration());
	bsSizes.m_uiLANSize = GetTaskPropValue<eTO_LANBufferSize>(m_tTaskDefinition.GetConfiguration());
	bsSizes.m_bOnlyDefault = GetTaskPropValue<eTO_UseOnlyDefaultBuffer>(m_tTaskDefinition.GetConfiguration());
}

int CTask::GetCurrentBufferIndex()
{
	return m_files.GetBufferIndexAt(m_tTaskBasicProgressInfo.GetCurrentIndex(), m_tTaskDefinition.GetDestinationPath());
}

// thread
void CTask::SetPriority(int nPriority)
{
	SetTaskPropValue<eTO_ThreadPriority>(m_tTaskDefinition.GetConfiguration(), nPriority);
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
	m_localStats.SetProcessedSize(m_files.CalculatePartialSize(m_tTaskBasicProgressInfo.GetCurrentIndex()));
}

void CTask::Load(const CString& strPath)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);

	////////////////////////////////
	// First load task description
	m_tTaskDefinition.Load(strPath);
	m_strFilePath = strPath;

	////////////////////////////////
	// now rarely changing task progress data
	CString strRarelyChangingPath = GetRelatedPathNL(ePathType_TaskRarelyChangingState);
	std::ifstream ifs(strRarelyChangingPath, ios_base::in | ios_base::binary);
	boost::archive::binary_iarchive ar(ifs);

	m_arrSourcePaths.Load(ar, 0, true);
	m_files.Load(ar, 0, false);

	CalculateTotalSizeNL();

	ar >> m_afFilters;

	///////////////////////////////////
	// and often changing data
	CString strOftenChangingPath = GetRelatedPathNL(ePathType_TaskOftenChangingState);
	std::ifstream ifs2(strOftenChangingPath, ios_base::in | ios_base::binary);
	boost::archive::binary_iarchive ar2(ifs2);

	ar2 >> m_tTaskBasicProgressInfo;

	CalculateProcessedSizeNL();

	// load task state, convert "waiting" state to "processing"
	int iState = eTaskState_None;
	ar2 >> iState;
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

	time_t timeElapsed = 0;
	ar2 >> timeElapsed;
	m_localStats.SetTimeElapsed(timeElapsed);

	m_arrSourcePaths.Load(ar2, 0, false);
	m_files.Load(ar2, 0, true);
}

void CTask::Store()
{
	boost::upgrade_lock<boost::shared_mutex> lock(m_lock);

	BOOST_ASSERT(!m_strTaskDirectory.IsEmpty());
	if(m_strTaskDirectory.IsEmpty())
		THROW(_t("Missing task path."), 0, 0, 0);

	// generate file path if not available yet
	if(m_strFilePath.IsEmpty())
	{
		boost::upgrade_to_unique_lock<boost::shared_mutex> upgraded_lock(lock);
		m_strFilePath = m_strTaskDirectory + m_tTaskDefinition.GetTaskUniqueID() + _T(".cht");
	}

	// store task definition only if changed
	m_tTaskDefinition.Store(GetRelatedPathNL(ePathType_TaskDefinition), true);

	// rarely changing data
	if(m_bRareStateModified)
	{
		std::ofstream ofs(GetRelatedPathNL(ePathType_TaskRarelyChangingState), ios_base::out | ios_base::binary);
		boost::archive::binary_oarchive ar(ofs);

		m_arrSourcePaths.Store(ar, 0, true);
		m_files.Store(ar, 0, false);

		ar << m_afFilters;
	}

	if(m_bOftenStateModified)
	{
		std::ofstream ofs(GetRelatedPathNL(ePathType_TaskOftenChangingState), ios_base::out | ios_base::binary);
		boost::archive::binary_oarchive ar(ofs);

		ar << m_tTaskBasicProgressInfo;

		// store current state (convert from waiting to processing state before storing)
		int iState = m_eCurrentState;
		if(iState == eTaskState_Waiting)
			iState = eTaskState_Processing;

		ar << iState;

		time_t timeElapsed = m_localStats.GetTimeElapsed();
		ar << timeElapsed;

		m_arrSourcePaths.Store(ar, 0, false);

		ESubOperationType eSubOperation = m_tTaskDefinition.GetOperationPlan().GetSubOperationAt(m_tTaskBasicProgressInfo.GetSubOperationIndex());
		if(eSubOperation != eSubOperation_Scanning)
			m_files.Store(ar, 0, true);
		else
		{
			size_t st(0);
			ar << st;
		}
	}
}

void CTask::KillThread()
{
	m_workerThread.StopThread();
}

void CTask::BeginProcessing()
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);

	m_bRareStateModified = true;
	m_bOftenStateModified = true;

	m_workerThread.StartThread(DelegateThreadProc, this, GetTaskPropValue<eTO_ThreadPriority>(m_tTaskDefinition.GetConfiguration()));
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
	m_tTaskBasicProgressInfo.SetCurrentIndex(0);

	BeginProcessing();
}

void CTask::PauseProcessing()
{
	if(GetTaskState() != eTaskState_Finished && GetTaskState() != eTaskState_Cancelled)
	{
		KillThread();
		SetTaskState(eTaskState_Paused);

		m_bOftenStateModified = true;
	}
}

void CTask::CancelProcessing()
{
	// change to ST_CANCELLED
	if(GetTaskState() != eTaskState_Finished)
	{
		KillThread();
		SetTaskState(eTaskState_Cancelled);
		m_bOftenStateModified = true;
	}
}

void CTask::GetMiniSnapshot(TASK_MINI_DISPLAY_DATA *pData)
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	size_t stCurrentIndex = m_tTaskBasicProgressInfo.GetCurrentIndex();

	if(stCurrentIndex < m_files.GetSize())
		pData->m_strPath = m_files.GetAt(stCurrentIndex)->GetFileName().ToString();
	else
	{
		if(m_files.GetSize() > 0)
			pData->m_strPath = m_files.GetAt(0)->GetFileName().ToString();
		else
		{
			if(m_tTaskDefinition.GetSourcePathCount() > 0)
				pData->m_strPath = m_arrSourcePaths.GetAt(0)->GetFileName().ToString();
			else
				pData->m_strPath.Empty();
		}
	}

	pData->m_eTaskState = m_eCurrentState;

	// percents
	pData->m_nPercent = m_localStats.GetProgressInPercent();
}

void CTask::GetSnapshot(TASK_DISPLAY_DATA *pData)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);

	size_t stCurrentIndex = m_tTaskBasicProgressInfo.GetCurrentIndex();
	if(stCurrentIndex < m_files.GetSize())
	{
		pData->m_strFullFilePath = m_files.GetAt(stCurrentIndex)->GetFullFilePath().ToString();
		pData->m_strFileName = m_files.GetAt(stCurrentIndex)->GetFileName().ToString();
	}
	else
	{
		if(m_files.GetSize() > 0)
		{
			pData->m_strFullFilePath = m_files.GetAt(0)->GetFullFilePath().ToString();
			pData->m_strFileName = m_files.GetAt(0)->GetFileName().ToString();
		}
		else
		{
			if(m_tTaskDefinition.GetSourcePathCount() > 0)
			{
				pData->m_strFullFilePath = m_arrSourcePaths.GetAt(0)->GetPath().ToString();
				pData->m_strFileName = m_arrSourcePaths.GetAt(0)->GetFileName().ToString();
			}
			else
			{
				pData->m_strFullFilePath.Empty();
				pData->m_strFileName.Empty();
			}
		}
	}

	pData->m_nPriority = GetTaskPropValue<eTO_ThreadPriority>(m_tTaskDefinition.GetConfiguration());
	pData->m_pathDstPath = m_tTaskDefinition.GetDestinationPath();
	pData->m_pafFilters=&m_afFilters;
	pData->m_eTaskState = m_eCurrentState;
	pData->m_stIndex = stCurrentIndex;
	pData->m_ullProcessedSize = m_localStats.GetProcessedSize();
	pData->m_stSize=m_files.GetSize();
	pData->m_ullSizeAll = m_localStats.GetTotalSize();
	pData->m_strUniqueName = m_tTaskDefinition.GetTaskUniqueID();
	pData->m_eOperationType = m_tTaskDefinition.GetOperationType();
	pData->m_eSubOperationType = m_tTaskDefinition.GetOperationPlan().GetSubOperationAt(m_tTaskBasicProgressInfo.GetSubOperationIndex());

	pData->m_bIgnoreDirectories = GetTaskPropValue<eTO_IgnoreDirectories>(m_tTaskDefinition.GetConfiguration());
	pData->m_bCreateEmptyFiles = GetTaskPropValue<eTO_CreateEmptyFiles>(m_tTaskDefinition.GetConfiguration());

	if(m_files.GetSize() > 0)
		pData->m_iCurrentBufferIndex = GetTaskPropValue<eTO_UseOnlyDefaultBuffer>(m_tTaskDefinition.GetConfiguration()) ? 0 : m_files.GetAt((stCurrentIndex < m_files.GetSize()) ? stCurrentIndex : 0)->GetBufferIndex(m_tTaskDefinition.GetDestinationPath());
	else
		pData->m_iCurrentBufferIndex = 0;

	switch(pData->m_iCurrentBufferIndex)
	{
	case BI_DEFAULT:
		pData->m_iCurrentBufferSize = GetTaskPropValue<eTO_DefaultBufferSize>(m_tTaskDefinition.GetConfiguration());
		break;
	case BI_ONEDISK:
		pData->m_iCurrentBufferSize = GetTaskPropValue<eTO_OneDiskBufferSize>(m_tTaskDefinition.GetConfiguration());
		break;
	case BI_TWODISKS:
		pData->m_iCurrentBufferSize = GetTaskPropValue<eTO_TwoDisksBufferSize>(m_tTaskDefinition.GetConfiguration());
		break;
	case BI_CD:
		pData->m_iCurrentBufferSize = GetTaskPropValue<eTO_CDBufferSize>(m_tTaskDefinition.GetConfiguration());
		break;
	case BI_LAN:
		pData->m_iCurrentBufferSize = GetTaskPropValue<eTO_LANBufferSize>(m_tTaskDefinition.GetConfiguration());
		break;
	default:
		THROW(_T("Unhandled case"), 0, 0, 0);
		//BOOST_ASSERT(false);		// assertions are dangerous here, because we're inside critical section
		// (and there could be conflict with Get(Mini)Snapshot called OnTimer in several places.
	}

	// percents
	pData->m_nPercent = m_localStats.GetProgressInPercent();

	// time
	pData->m_timeElapsed = m_localStats.GetTimeElapsed();
}

void CTask::DeleteProgress()
{
	std::vector<CString> vFilesToRemove;

	// separate scope for shared locking
	{
		boost::shared_lock<boost::shared_mutex> lock(m_lock);

		vFilesToRemove.push_back(GetRelatedPath(ePathType_TaskDefinition));
		vFilesToRemove.push_back(GetRelatedPath(ePathType_TaskRarelyChangingState));
		vFilesToRemove.push_back(GetRelatedPath(ePathType_TaskOftenChangingState));
		vFilesToRemove.push_back(GetRelatedPath(ePathType_TaskLogFile));
	}

	BOOST_FOREACH(const CString& strFile, vFilesToRemove)
	{
		DeleteFile(strFile);
	}
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
	if(!GetDynamicFreeSpace(m_tTaskDefinition.GetDestinationPath().ToString(), pullAvailable, NULL))
		return true;

	return (*pullNeeded <= *pullAvailable);
}

void CTask::SetTaskDirectory(const CString& strDir)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_strTaskDirectory = strDir;
}

CString CTask::GetTaskDirectory() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_strTaskDirectory;
}

void CTask::SetTaskFilePath(const CString& strFilePath)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_strFilePath = strFilePath;
}

CString CTask::GetTaskFilePath() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_strFilePath;
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
CTask::ESubOperationResult CTask::RecurseDirectories()
{
	// log
	m_log.logi(_T("Searching for files..."));

	// delete the content of m_files
	m_files.Clear();

	// enter some data to m_files
	int iDestDrvNumber = 0;
	GetDriveData(m_tTaskDefinition.GetDestinationPath(), &iDestDrvNumber, NULL);

	bool bIgnoreDirs = GetTaskPropValue<eTO_IgnoreDirectories>(m_tTaskDefinition.GetConfiguration());
	bool bForceDirectories = GetTaskPropValue<eTO_CreateDirectoriesRelativeToRoot>(m_tTaskDefinition.GetConfiguration());
	bool bMove = m_tTaskDefinition.GetOperationType() == eOperation_Move;

	// add everything
	ictranslate::CFormat fmt;
	bool bRetry = true;
	bool bSkipInputPath = false;

	size_t stSize = m_tTaskDefinition.GetSourcePathCount();
	for(size_t stIndex = 0; stIndex < stSize ; stIndex++)
	{
		CFileInfoPtr spFileInfo;

		bSkipInputPath = false;

		spFileInfo.reset(new CFileInfo());
		spFileInfo->SetClipboard(&m_arrSourcePaths);

		// try to get some info about the input path; let user know if the path does not exist.
		do
		{
			bRetry = false;

			// read attributes of src file/folder
			bool bExists = spFileInfo->Create(m_arrSourcePaths.GetAt(stIndex)->GetPath(), stIndex);
			if(!bExists)
			{
				CString strSrcFile = m_arrSourcePaths.GetAt(stIndex)->GetPath().ToString();
				FEEDBACK_FILEERROR ferr = { (PCTSTR)strSrcFile, NULL, eFastMoveError, ERROR_FILE_NOT_FOUND };
				CFeedbackHandler::EFeedbackResult frResult = (CFeedbackHandler::EFeedbackResult)m_piFeedbackHandler->RequestFeedback(CFeedbackHandler::eFT_FileError, &ferr);
				switch(frResult)
				{
				case CFeedbackHandler::eResult_Cancel:
					m_files.Clear();
					return eSubResult_CancelRequest;

				case CFeedbackHandler::eResult_Retry:
					bRetry = true;
					break;

				case CFeedbackHandler::eResult_Pause:
					m_files.Clear();
					return eSubResult_PauseRequest;

				case CFeedbackHandler::eResult_Skip:
					bSkipInputPath = true;
					break;		// just do nothing

				default:
					BOOST_ASSERT(FALSE);		// unknown result
					THROW(_T("Unhandled case"), 0, 0, 0);
				}
			}
		}
		while(bRetry);

		// if we have chosen to skip the input path then there's nothing to do
		if(bSkipInputPath)
			continue;

		// log
		fmt.SetFormat(_T("Adding file/folder (clipboard) : %path ..."));
		fmt.SetParam(_t("%path"), m_arrSourcePaths.GetAt(stIndex)->GetPath().ToString());
		m_log.logi(fmt);

		// found file/folder - check if the dest name has been generated
		if(!m_arrSourcePaths.GetAt(stIndex)->IsDestinationPathSet())
		{
			// generate something - if dest folder == src folder - search for copy
			if(m_tTaskDefinition.GetDestinationPath() == spFileInfo->GetFileRoot())
			{
				chcore::TSmartPath pathSubst = FindFreeSubstituteName(spFileInfo->GetFullFilePath(), m_tTaskDefinition.GetDestinationPath());
				m_arrSourcePaths.GetAt(stIndex)->SetDestinationPath(pathSubst);
			}
			else
				m_arrSourcePaths.GetAt(stIndex)->SetDestinationPath(spFileInfo->GetFileName());
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
				fmt.SetParam(_t("%path"), spFileInfo->GetFullFilePath().ToString());
				m_log.logi(fmt);
			}

			// don't add folder contents when moving inside one disk boundary
			if(bIgnoreDirs || !bMove || iDestDrvNumber == -1 || iDestDrvNumber != spFileInfo->GetDriveNumber() ||
				CFileInfo::Exist(GetDestinationPath(spFileInfo, m_tTaskDefinition.GetDestinationPath(), ((int)bForceDirectories) << 1)) )
			{
				// log
				fmt.SetFormat(_T("Recursing folder %path"));
				fmt.SetParam(_t("%path"), spFileInfo->GetFullFilePath().ToString());
				m_log.logi(fmt);

				// no movefile possibility - use CustomCopyFileFB
				m_arrSourcePaths.GetAt(stIndex)->SetMove(false);

				ScanDirectory(spFileInfo->GetFullFilePath(), stIndex, true, !bIgnoreDirs || bForceDirectories);
			}

			// check for kill need
			if(m_workerThread.KillRequested())
			{
				// log
				m_log.logi(_T("Kill request while adding data to files array (RecurseDirectories)"));
				m_files.Clear();
				return eSubResult_KillRequest;
			}
		}
		else
		{
			if(bMove && iDestDrvNumber != -1 && iDestDrvNumber == spFileInfo->GetDriveNumber() &&
				!CFileInfo::Exist(GetDestinationPath(spFileInfo, m_tTaskDefinition.GetDestinationPath(), ((int)bForceDirectories) << 1)) )
			{
				// if moving within one partition boundary set the file size to 0 so the overall size will
				// be ok
				spFileInfo->SetLength64(0);
			}
			else
				m_arrSourcePaths.GetAt(stIndex)->SetMove(false);	// no MoveFile

			// add file info if passes filters
			if(m_afFilters.Match(spFileInfo))
				m_files.AddFileInfo(spFileInfo);

			// log
			fmt.SetFormat(_T("Added file %path"));
			fmt.SetParam(_t("%path"), spFileInfo->GetFullFilePath().ToString());
			m_log.logi(fmt);
		}
	}

	// calc size of all files
	CalculateTotalSize();

	// save task status
	Store();

	// log
	m_log.logi(_T("Searching for files finished"));

	return eSubResult_Continue;
}

// delete files - after copying
CTask::ESubOperationResult CTask::DeleteFiles()
{
	// log
	m_log.logi(_T("Deleting files (DeleteFiles)..."));

	// current processed path
	BOOL bSuccess;
	CFileInfoPtr spFileInfo;
	ictranslate::CFormat fmt;

	// index points to 0 or next item to process
	size_t stIndex = m_tTaskBasicProgressInfo.GetCurrentIndex();
	while(stIndex < m_files.GetSize())
	{
		// set index in pTask to currently deleted element
		m_tTaskBasicProgressInfo.SetCurrentIndex(stIndex);

		// check for kill flag
		if(m_workerThread.KillRequested())
		{
			// log
			m_log.logi(_T("Kill request while deleting files (Delete Files)"));
			return eSubResult_KillRequest;
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
			if(!GetTaskPropValue<eTO_ProtectReadOnlyFiles>(m_tTaskDefinition.GetConfiguration()))
				SetFileAttributes(spFileInfo->GetFullFilePath().ToString(), FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_DIRECTORY);
			bSuccess=RemoveDirectory(spFileInfo->GetFullFilePath().ToString());
		}
		else
		{
			// set files attributes to normal - it'd slow processing a bit, but it's better.
			if(!GetTaskPropValue<eTO_ProtectReadOnlyFiles>(m_tTaskDefinition.GetConfiguration()))
				SetFileAttributes(spFileInfo->GetFullFilePath().ToString(), FILE_ATTRIBUTE_NORMAL);
			bSuccess=DeleteFile(spFileInfo->GetFullFilePath().ToString());
		}

		// operation failed
		DWORD dwLastError=GetLastError();
		if(!bSuccess && dwLastError != ERROR_PATH_NOT_FOUND && dwLastError != ERROR_FILE_NOT_FOUND)
		{
			// log
			fmt.SetFormat(_T("Error #%errno while deleting file/folder %path"));
			fmt.SetParam(_t("%errno"), dwLastError);
			fmt.SetParam(_t("%path"), spFileInfo->GetFullFilePath().ToString());
			m_log.loge(fmt);

			CString strFile = spFileInfo->GetFullFilePath().ToString();
			FEEDBACK_FILEERROR ferr = { (PCTSTR)strFile, NULL, eDeleteError, dwLastError };
			CFeedbackHandler::EFeedbackResult frResult = (CFeedbackHandler::EFeedbackResult)m_piFeedbackHandler->RequestFeedback(CFeedbackHandler::eFT_FileError, &ferr);
			switch(frResult)
			{
			case CFeedbackHandler::eResult_Cancel:
				m_log.logi(_T("Cancel request while deleting file."));
				return eSubResult_CancelRequest;

			case CFeedbackHandler::eResult_Retry:
				continue;	// no stIndex bump, since we are trying again

			case CFeedbackHandler::eResult_Pause:
				return eSubResult_PauseRequest;

			case CFeedbackHandler::eResult_Skip:
				break;		// just do nothing

			default:
				BOOST_ASSERT(FALSE);		// unknown result
				THROW(_T("Unhandled case"), 0, 0, 0);
			}
		}

		++stIndex;
	}//while

	// change status to finished
	SetTaskState(eTaskState_Finished);

	// add 1 to current index
	m_tTaskBasicProgressInfo.IncreaseCurrentIndex();

	// log
	m_log.logi(_T("Deleting files finished"));

	return eSubResult_Continue;
}

CTask::ESubOperationResult CTask::OpenSourceFileFB(TAutoFileHandle& hOutFile, const CFileInfoPtr& spSrcFileInfo, bool bNoBuffering)
{
	BOOST_ASSERT(spSrcFileInfo);
	if(!spSrcFileInfo)
		THROW(_T("Invalid argument"), 0, 0, 0);

	bool bRetry = false;
	CString strPath = spSrcFileInfo->GetFullFilePath().ToString();

	hOutFile = INVALID_HANDLE_VALUE;

	TAutoFileHandle hFile;
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
					fmt.SetFormat(_T("Cancel request [error %errno] while opening source file %path (CustomCopyFileFB)"));
					fmt.SetParam(_t("%errno"), dwLastError);
					fmt.SetParam(_t("%path"), strPath);
					m_log.loge(fmt);

					return eSubResult_CancelRequest;
				}

			case CFeedbackHandler::eResult_Pause:
				return eSubResult_PauseRequest;

			case CFeedbackHandler::eResult_Retry:
				{
					// log
					ictranslate::CFormat fmt;
					fmt.SetFormat(_T("Retrying [error %errno] to open source file %path (CustomCopyFileFB)"));
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

	hOutFile = hFile.Detach();

	return eSubResult_Continue;
}

CTask::ESubOperationResult CTask::OpenDestinationFileFB(TAutoFileHandle& hOutFile, const chcore::TSmartPath& pathDstFile, bool bNoBuffering, const CFileInfoPtr& spSrcFileInfo, unsigned long long& ullSeekTo, bool& bFreshlyCreated)
{
	bool bRetry = false;
	TAutoFileHandle hFile;

	ullSeekTo = 0;
	bFreshlyCreated = true;
	hOutFile = INVALID_HANDLE_VALUE;

	do
	{
		bRetry = false;

		hFile = ::CreateFile(pathDstFile.ToString(), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN | (bNoBuffering ? FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH : 0), NULL);
		if(hFile == INVALID_HANDLE_VALUE)
		{
			DWORD dwLastError = GetLastError();
			if(dwLastError == ERROR_FILE_EXISTS)
			{
				bFreshlyCreated = false;

				// pass it to the specialized method
				CTask::ESubOperationResult eResult = OpenExistingDestinationFileFB(hFile, pathDstFile, bNoBuffering);
				if(eResult != eSubResult_Continue)
					return eResult;
				else if(hFile == INVALID_HANDLE_VALUE)
					return eSubResult_Continue;

				// read info about the existing destination file,
				// NOTE: it is not known which one would be faster - reading file parameters
				//       by using spDstFileInfo->Create() (which uses FindFirstFile()) or by
				//       reading parameters using opened handle; need to be tested in the future
				CFileInfoPtr spDstFileInfo(boost::make_shared<CFileInfo>());
				if(!spDstFileInfo->Create(pathDstFile, std::numeric_limits<size_t>::max()))
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
					return eSubResult_Continue;

				case CFeedbackHandler::eResult_Cancel:
					{
						// log
						ictranslate::CFormat fmt;
						fmt.SetFormat(_T("Cancel request while checking result of dialog before opening source file %path (CustomCopyFileFB)"));
						fmt.SetParam(_t("%path"), pathDstFile.ToString());
						m_log.logi(fmt);

						return eSubResult_CancelRequest;
					}
				case CFeedbackHandler::eResult_Pause:
					return eSubResult_PauseRequest;

				default:
					BOOST_ASSERT(FALSE);		// unknown result
					THROW(_T("Unhandled case"), 0, 0, 0);
				}
			}
			else
			{
				FEEDBACK_FILEERROR feedStruct = { pathDstFile.ToString(), NULL, eCreateError, dwLastError };
				CFeedbackHandler::EFeedbackResult frResult = (CFeedbackHandler::EFeedbackResult)m_piFeedbackHandler->RequestFeedback(CFeedbackHandler::eFT_FileError, &feedStruct);
				switch (frResult)
				{
				case CFeedbackHandler::eResult_Retry:
					{
						// log
						ictranslate::CFormat fmt;
						fmt.SetFormat(_T("Retrying [error %errno] to open destination file %path (CustomCopyFileFB)"));
						fmt.SetParam(_t("%errno"), dwLastError);
						fmt.SetParam(_t("%path"), pathDstFile.ToString());
						m_log.loge(fmt);

						bRetry = true;

						break;
					}
				case CFeedbackHandler::eResult_Cancel:
					{
						// log
						ictranslate::CFormat fmt;

						fmt.SetFormat(_T("Cancel request [error %errno] while opening destination file %path (CustomCopyFileFB)"));
						fmt.SetParam(_t("%errno"), dwLastError);
						fmt.SetParam(_t("%path"), pathDstFile.ToString());
						m_log.loge(fmt);

						return eSubResult_CancelRequest;
					}

				case CFeedbackHandler::eResult_Skip:
					break;		// will return invalid handle value

				case CFeedbackHandler::eResult_Pause:
					return eSubResult_PauseRequest;

				default:
					BOOST_ASSERT(FALSE);		// unknown result
					THROW(_T("Unhandled case"), 0, 0, 0);
				}
			}
		}
	}
	while(bRetry);

	hOutFile = hFile.Detach();

	return eSubResult_Continue;
}

CTask::ESubOperationResult CTask::OpenExistingDestinationFileFB(TAutoFileHandle& hOutFile, const chcore::TSmartPath& pathDstFile, bool bNoBuffering)
{
	bool bRetry = false;
	TAutoFileHandle hFile;

	hOutFile = INVALID_HANDLE_VALUE;

	do
	{
		bRetry = false;

		hFile = CreateFile(pathDstFile.ToString(), GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN | (bNoBuffering ? FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH : 0), NULL);
		if(hFile == INVALID_HANDLE_VALUE)
		{
			DWORD dwLastError = GetLastError();
			FEEDBACK_FILEERROR feedStruct = { pathDstFile.ToString(), NULL, eCreateError, dwLastError };
			CFeedbackHandler::EFeedbackResult frResult = (CFeedbackHandler::EFeedbackResult)m_piFeedbackHandler->RequestFeedback(CFeedbackHandler::eFT_FileError, &feedStruct);
			switch (frResult)
			{
			case CFeedbackHandler::eResult_Retry:
				{
					// log
					ictranslate::CFormat fmt;
					fmt.SetFormat(_T("Retrying [error %errno] to open destination file %path (CustomCopyFileFB)"));
					fmt.SetParam(_t("%errno"), dwLastError);
					fmt.SetParam(_t("%path"), pathDstFile.ToString());
					m_log.loge(fmt);

					bRetry = true;

					break;
				}
			case CFeedbackHandler::eResult_Cancel:
				{
					// log
					ictranslate::CFormat fmt;

					fmt.SetFormat(_T("Cancel request [error %errno] while opening destination file %path (CustomCopyFileFB)"));
					fmt.SetParam(_t("%errno"), dwLastError);
					fmt.SetParam(_t("%path"), pathDstFile.ToString());
					m_log.loge(fmt);

					return eSubResult_CancelRequest;
				}

			case CFeedbackHandler::eResult_Skip:
				break;		// will return invalid handle value

			case CFeedbackHandler::eResult_Pause:
				return eSubResult_PauseRequest;

			default:
				BOOST_ASSERT(FALSE);		// unknown result
				THROW(_T("Unhandled case"), 0, 0, 0);
			}
		}
	}
	while(bRetry);

	hOutFile = hFile.Detach();

	return eSubResult_Continue;
}

CTask::ESubOperationResult CTask::SetFilePointerFB(HANDLE hFile, long long llDistance, const chcore::TSmartPath& pathFile, bool& bSkip)
{
	bSkip = false;
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
			fmt.SetParam(_t("%path"), pathFile.ToString());
			fmt.SetParam(_t("%pos"), llDistance);
			m_log.loge(fmt);

			FEEDBACK_FILEERROR ferr = { pathFile.ToString(), NULL, eSeekError, dwLastError };
			CFeedbackHandler::EFeedbackResult frResult = (CFeedbackHandler::EFeedbackResult)m_piFeedbackHandler->RequestFeedback(CFeedbackHandler::eFT_FileError, &ferr);
			switch(frResult)
			{
			case CFeedbackHandler::eResult_Cancel:
				return eSubResult_CancelRequest;

			case CFeedbackHandler::eResult_Retry:
				bRetry = true;
				break;

			case CFeedbackHandler::eResult_Pause:
				return eSubResult_PauseRequest;

			case CFeedbackHandler::eResult_Skip:
				bSkip = true;
				return eSubResult_Continue;

			default:
				BOOST_ASSERT(FALSE);		// unknown result
				THROW(_T("Unhandled case"), 0, 0, 0);
			}
		}
	}
	while(bRetry);

	return eSubResult_Continue;
}

CTask::ESubOperationResult CTask::SetEndOfFileFB(HANDLE hFile, const chcore::TSmartPath& pathFile, bool& bSkip)
{
	bSkip = false;

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
			fmt.SetParam(_t("%path"), pathFile.ToString());
			m_log.loge(fmt);

			FEEDBACK_FILEERROR ferr = { pathFile.ToString(), NULL, eResizeError, dwLastError };
			CFeedbackHandler::EFeedbackResult frResult = (CFeedbackHandler::EFeedbackResult)m_piFeedbackHandler->RequestFeedback(CFeedbackHandler::eFT_FileError, &ferr);
			switch(frResult)
			{
			case CFeedbackHandler::eResult_Cancel:
				return eSubResult_CancelRequest;

			case CFeedbackHandler::eResult_Retry:
				bRetry = true;

			case CFeedbackHandler::eResult_Pause:
				return eSubResult_PauseRequest;

			case CFeedbackHandler::eResult_Skip:
				bSkip = true;
				return eSubResult_Continue;

			default:
				BOOST_ASSERT(FALSE);		// unknown result
				THROW(_T("Unhandled case"), 0, 0, 0);
			}
		}
	}
	while(bRetry);

	return eSubResult_Continue;
}

CTask::ESubOperationResult CTask::ReadFileFB(HANDLE hFile, CDataBuffer& rBuffer, DWORD dwToRead, DWORD& rdwBytesRead, const chcore::TSmartPath& pathFile, bool& bSkip)
{
	bSkip = false;
	bool bRetry = false;
	do
	{
		bRetry = false;

		if(!ReadFile(hFile, rBuffer, dwToRead, &rdwBytesRead, NULL))
		{
			// log
			DWORD dwLastError = GetLastError();

			ictranslate::CFormat fmt;
			fmt.SetFormat(_T("Error %errno while trying to read %count bytes from source file %path (CustomCopyFileFB)"));
			fmt.SetParam(_t("%errno"), dwLastError);
			fmt.SetParam(_t("%count"), dwToRead);
			fmt.SetParam(_t("%path"), pathFile.ToString());
			m_log.loge(fmt);

			FEEDBACK_FILEERROR ferr = { pathFile.ToString(), NULL, eReadError, dwLastError };
			CFeedbackHandler::EFeedbackResult frResult = (CFeedbackHandler::EFeedbackResult)m_piFeedbackHandler->RequestFeedback(CFeedbackHandler::eFT_FileError, &ferr);
			switch(frResult)
			{
			case CFeedbackHandler::eResult_Cancel:
				return eSubResult_CancelRequest;

			case CFeedbackHandler::eResult_Retry:
				bRetry = true;
				break;

			case CFeedbackHandler::eResult_Pause:
				return eSubResult_PauseRequest;

			case CFeedbackHandler::eResult_Skip:
				bSkip = true;
				return eSubResult_Continue;

			default:
				BOOST_ASSERT(FALSE);		// unknown result
				THROW(_T("Unhandled case"), 0, 0, 0);
			}
		}
	}
	while(bRetry);

	return eSubResult_Continue;
}

CTask::ESubOperationResult CTask::WriteFileFB(HANDLE hFile, CDataBuffer& rBuffer, DWORD dwToWrite, DWORD& rdwBytesWritten, const chcore::TSmartPath& pathFile, bool& bSkip)
{
	bSkip = false;

	bool bRetry = false;
	do
	{
		bRetry = false;

		if(!WriteFile(hFile, rBuffer, dwToWrite, &rdwBytesWritten, NULL) || dwToWrite != rdwBytesWritten)
		{
			// log
			DWORD dwLastError = GetLastError();

			ictranslate::CFormat fmt;
			fmt.SetFormat(_T("Error %errno while trying to write %count bytes to destination file %path (CustomCopyFileFB)"));
			fmt.SetParam(_t("%errno"), dwLastError);
			fmt.SetParam(_t("%count"), dwToWrite);
			fmt.SetParam(_t("%path"), pathFile.ToString());
			m_log.loge(fmt);

			FEEDBACK_FILEERROR ferr = { pathFile.ToString(), NULL, eWriteError, dwLastError };
			CFeedbackHandler::EFeedbackResult frResult = (CFeedbackHandler::EFeedbackResult)m_piFeedbackHandler->RequestFeedback(CFeedbackHandler::eFT_FileError, &ferr);
			switch(frResult)
			{
			case CFeedbackHandler::eResult_Cancel:
				return eSubResult_CancelRequest;

			case CFeedbackHandler::eResult_Retry:
				bRetry = true;
				break;

			case CFeedbackHandler::eResult_Pause:
				return eSubResult_PauseRequest;

			case CFeedbackHandler::eResult_Skip:
				bSkip = true;
				return eSubResult_Continue;

			default:
				BOOST_ASSERT(FALSE);		// unknown result
				THROW(_T("Unhandled case"), 0, 0, 0);
			}
		}
	}
	while(bRetry);

	return eSubResult_Continue;
}

CTask::ESubOperationResult CTask::CustomCopyFileFB(CUSTOM_COPY_PARAMS* pData)
{
	TAutoFileHandle hSrc = INVALID_HANDLE_VALUE,
		hDst = INVALID_HANDLE_VALUE;
	ictranslate::CFormat fmt;
	CTask::ESubOperationResult eResult = eSubResult_Continue;
	bool bSkip = false;

	// calculate if we want to disable buffering for file transfer
	// NOTE: we are using here the file size read when scanning directories for files; it might be
	//       outdated at this point, but at present we don't want to re-read file size since it
	//       will cost additional disk access
	bool bNoBuffer = (GetTaskPropValue<eTO_DisableBuffering>(m_tTaskDefinition.GetConfiguration()) &&
						pData->spSrcFile->GetLength64() >= GetTaskPropValue<eTO_DisableBufferingMinSize>(m_tTaskDefinition.GetConfiguration()));

	// first open the source file and handle any failures
	eResult = OpenSourceFileFB(hSrc, pData->spSrcFile, bNoBuffer);
	if(eResult != eSubResult_Continue)
		return eResult;
	else if(hSrc == INVALID_HANDLE_VALUE)
	{
		// invalid handle = operation skipped by user
		m_localStats.IncreaseProcessedSize(pData->spSrcFile->GetLength64() - m_tTaskBasicProgressInfo.GetCurrentFileProcessedSize());
		pData->bProcessed = false;
		return eSubResult_Continue;
	}

	// change attributes of a dest file
	// NOTE: probably should be removed from here and report problems with read-only files
	//       directly to the user (as feedback request)
	if(!GetTaskPropValue<eTO_ProtectReadOnlyFiles>(m_tTaskDefinition.GetConfiguration()))
		SetFileAttributes(pData->pathDstFile.ToString(), FILE_ATTRIBUTE_NORMAL);

	// open destination file, handle the failures and possibly existence of the destination file
	unsigned long long ullSeekTo = 0;
	bool bDstFileFreshlyCreated = false;

	if(m_tTaskBasicProgressInfo.GetCurrentFileProcessedSize() == 0)
	{
		// open destination file for case, when we start operation on this file (i.e. it is not resume of the
		// old operation)
		eResult = OpenDestinationFileFB(hDst, pData->pathDstFile, bNoBuffer, pData->spSrcFile, ullSeekTo, bDstFileFreshlyCreated);
		if(eResult != eSubResult_Continue)
			return eResult;
		else if(hDst == INVALID_HANDLE_VALUE)
		{
			m_localStats.IncreaseProcessedSize(pData->spSrcFile->GetLength64() - m_tTaskBasicProgressInfo.GetCurrentFileProcessedSize());
			pData->bProcessed = false;
			return eSubResult_Continue;
		}
	}
	else
	{
		// we are resuming previous operation
		eResult = OpenExistingDestinationFileFB(hDst, pData->pathDstFile, bNoBuffer);
		if(eResult != eSubResult_Continue)
			return eResult;
		else if(hDst == INVALID_HANDLE_VALUE)
		{
			m_localStats.IncreaseProcessedSize(pData->spSrcFile->GetLength64() - m_tTaskBasicProgressInfo.GetCurrentFileProcessedSize());
			pData->bProcessed = false;
			return eSubResult_Continue;
		}

		ullSeekTo = m_tTaskBasicProgressInfo.GetCurrentFileProcessedSize();
	}

	if(!pData->bOnlyCreate)
	{
		// seek to the position where copying will start
		if(ullSeekTo != 0)		// src and dst files exists, requested resume at the specified index
		{
			// try to move file pointers to the end
			ULONGLONG ullMove = (bNoBuffer ? ROUNDDOWN(ullSeekTo, MAXSECTORSIZE) : ullSeekTo);

			eResult = SetFilePointerFB(hSrc, ullMove, pData->spSrcFile->GetFullFilePath(), bSkip);
			if(eResult != eSubResult_Continue)
				return eResult;
			else if(bSkip)
			{
				m_localStats.IncreaseProcessedSize(pData->spSrcFile->GetLength64() - m_tTaskBasicProgressInfo.GetCurrentFileProcessedSize());
				pData->bProcessed = false;
				return eSubResult_Continue;
			}

			eResult = SetFilePointerFB(hDst, ullMove, pData->pathDstFile, bSkip);
			if(eResult != eSubResult_Continue)
				return eResult;
			else if(bSkip)
			{
				// with either first or second seek we got 'skip' answer...
				m_localStats.IncreaseProcessedSize(pData->spSrcFile->GetLength64() - m_tTaskBasicProgressInfo.GetCurrentFileProcessedSize());
				pData->bProcessed = false;
				return eSubResult_Continue;
			}

			m_tTaskBasicProgressInfo.IncreaseCurrentFileProcessedSize(ullMove);
			m_localStats.IncreaseProcessedSize(ullMove);
		}

		// if the destination file already exists - truncate it to the current file position
		if(!bDstFileFreshlyCreated)
		{
			// if destination file was opened (as opposed to newly created)
			eResult = SetEndOfFileFB(hDst, pData->pathDstFile, bSkip);
			if(eResult != eSubResult_Continue)
				return eResult;
			else if(bSkip)
			{
				pData->bProcessed = false;
				return eSubResult_Continue;
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
				fmt.SetParam(_t("%srcpath"), pData->spSrcFile->GetFullFilePath().ToString());
				fmt.SetParam(_t("%dstpath"), pData->pathDstFile.ToString());
				m_log.logi(fmt);
				return eSubResult_KillRequest;
			}

			// recreate buffer if needed
			if(m_cfgTracker.IsModified() && m_cfgTracker.IsModified(TOptionsSet() % eTO_DefaultBufferSize % eTO_OneDiskBufferSize % eTO_TwoDisksBufferSize % eTO_CDBufferSize % eTO_LANBufferSize % eTO_UseOnlyDefaultBuffer, true))
			{
				BUFFERSIZES bs;
				bs.m_bOnlyDefault = GetTaskPropValue<eTO_UseOnlyDefaultBuffer>(m_tTaskDefinition.GetConfiguration());
				bs.m_uiDefaultSize = GetTaskPropValue<eTO_DefaultBufferSize>(m_tTaskDefinition.GetConfiguration());
				bs.m_uiOneDiskSize = GetTaskPropValue<eTO_OneDiskBufferSize>(m_tTaskDefinition.GetConfiguration());
				bs.m_uiTwoDisksSize = GetTaskPropValue<eTO_TwoDisksBufferSize>(m_tTaskDefinition.GetConfiguration());
				bs.m_uiCDSize = GetTaskPropValue<eTO_CDBufferSize>(m_tTaskDefinition.GetConfiguration());
				bs.m_uiLANSize = GetTaskPropValue<eTO_LANBufferSize>(m_tTaskDefinition.GetConfiguration());

				// log
				const BUFFERSIZES* pbs1 = pData->dbBuffer.GetSizes();

				fmt.SetFormat(_T("Changing buffer size from [Def:%defsize, One:%onesize, Two:%twosize, CD:%cdsize, LAN:%lansize] to [Def:%defsize2, One:%onesize2, Two:%twosize2, CD:%cdsize2, LAN:%lansize2] wile copying %srcfile -> %dstfile (CustomCopyFileFB)"));

				fmt.SetParam(_t("%defsize"), pbs1->m_uiDefaultSize);
				fmt.SetParam(_t("%onesize"), pbs1->m_uiOneDiskSize);
				fmt.SetParam(_t("%twosize"), pbs1->m_uiTwoDisksSize);
				fmt.SetParam(_t("%cdsize"), pbs1->m_uiCDSize);
				fmt.SetParam(_t("%lansize"), pbs1->m_uiLANSize);
				fmt.SetParam(_t("%defsize2"), bs.m_uiDefaultSize);
				fmt.SetParam(_t("%onesize2"), bs.m_uiOneDiskSize);
				fmt.SetParam(_t("%twosize2"), bs.m_uiTwoDisksSize);
				fmt.SetParam(_t("%cdsize2"), bs.m_uiCDSize);
				fmt.SetParam(_t("%lansize2"), bs.m_uiLANSize);
				fmt.SetParam(_t("%srcfile"), pData->spSrcFile->GetFullFilePath().ToString());
				fmt.SetParam(_t("%dstfile"), pData->pathDstFile.ToString());

				m_log.logi(fmt);
				pData->dbBuffer.Create(&bs);
			}

			// establish count of data to read
			if(GetTaskPropValue<eTO_UseOnlyDefaultBuffer>(m_tTaskDefinition.GetConfiguration()))
				iBufferIndex = BI_DEFAULT;
			else
				iBufferIndex = pData->spSrcFile->GetBufferIndex(m_tTaskDefinition.GetDestinationPath());

			ulToRead = bNoBuffer ? ROUNDUP(pData->dbBuffer.GetSizes()->m_auiSizes[iBufferIndex], MAXSECTORSIZE) : pData->dbBuffer.GetSizes()->m_auiSizes[iBufferIndex];

			// read data from file to buffer
			eResult = ReadFileFB(hSrc, pData->dbBuffer, ulToRead, ulRead, pData->spSrcFile->GetFullFilePath(), bSkip);
			if(eResult != eSubResult_Continue)
				return eResult;
			else if(bSkip)
			{
				m_localStats.IncreaseProcessedSize(pData->spSrcFile->GetLength64() - m_tTaskBasicProgressInfo.GetCurrentFileProcessedSize());
				pData->bProcessed = false;
				return eSubResult_Continue;
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
						eResult = WriteFileFB(hDst, pData->dbBuffer, ulDataToWrite, ulWritten, pData->pathDstFile, bSkip);
						if(eResult != eSubResult_Continue)
							return eResult;
						else if(bSkip)
						{
							m_localStats.IncreaseProcessedSize(pData->spSrcFile->GetLength64() - m_tTaskBasicProgressInfo.GetCurrentFileProcessedSize());
							pData->bProcessed = false;
							return eSubResult_Continue;
						}

						// increase count of processed data
						m_tTaskBasicProgressInfo.IncreaseCurrentFileProcessedSize(ulWritten);
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
						eResult = OpenExistingDestinationFileFB(hDst, pData->pathDstFile, false);
						if(eResult != eSubResult_Continue)
							return eResult;
						else if(hDst == INVALID_HANDLE_VALUE)
						{
							m_localStats.IncreaseProcessedSize(pData->spSrcFile->GetLength64() - m_tTaskBasicProgressInfo.GetCurrentFileProcessedSize());
							pData->bProcessed = false;
							return eSubResult_Continue;
						}

						// move file pointer to the end of destination file
						eResult = SetFilePointerFB(hDst, m_tTaskBasicProgressInfo.GetCurrentFileProcessedSize(), pData->pathDstFile, bSkip);
						if(eResult != eSubResult_Continue)
							return eResult;
						else if(bSkip)
						{
							// with either first or second seek we got 'skip' answer...
							m_localStats.IncreaseProcessedSize(pData->spSrcFile->GetLength64() - m_tTaskBasicProgressInfo.GetCurrentFileProcessedSize());
							pData->bProcessed = false;
							return eSubResult_Continue;
						}
					}
				}

				// write
				if(ulRead != 0)
				{
					eResult = WriteFileFB(hDst, pData->dbBuffer, ulRead, ulWritten, pData->pathDstFile, bSkip);
					if(eResult != eSubResult_Continue)
						return eResult;
					else if(bSkip)
					{
						m_localStats.IncreaseProcessedSize(pData->spSrcFile->GetLength64() - m_tTaskBasicProgressInfo.GetCurrentFileProcessedSize());
						pData->bProcessed = false;
						return eSubResult_Continue;
					}

					// increase count of processed data
					m_tTaskBasicProgressInfo.IncreaseCurrentFileProcessedSize(ulRead);
					m_localStats.IncreaseProcessedSize(ulRead);
				}
			}
		}
		while(ulRead != 0 && !bLastPart);
	}
	else
	{
		// we don't copy contents, but need to increase processed size
		m_localStats.IncreaseProcessedSize(pData->spSrcFile->GetLength64() - m_tTaskBasicProgressInfo.GetCurrentFileProcessedSize());
	}

	pData->bProcessed = true;
	m_tTaskBasicProgressInfo.SetCurrentFileProcessedSize(0);

	return eSubResult_Continue;
}

// function processes files/folders
CTask::ESubOperationResult CTask::ProcessFiles()
{
	// log
	m_log.logi(_T("Processing files/folders (ProcessFiles)"));

	// count how much has been done (updates also a member in CTaskArray)
	CalculateProcessedSize();

	// begin at index which wasn't processed previously
	size_t stSize = m_files.GetSize();
	bool bIgnoreFolders = GetTaskPropValue<eTO_IgnoreDirectories>(m_tTaskDefinition.GetConfiguration());
	bool bForceDirectories = GetTaskPropValue<eTO_CreateDirectoriesRelativeToRoot>(m_tTaskDefinition.GetConfiguration());

	// create a buffer of size m_nBufferSize
	CUSTOM_COPY_PARAMS ccp;
	ccp.bProcessed = false;
	ccp.bOnlyCreate = GetTaskPropValue<eTO_CreateEmptyFiles>(m_tTaskDefinition.GetConfiguration());

	// remove changes in buffer sizes to avoid re-creation later
	m_cfgTracker.RemoveModificationSet(TOptionsSet() % eTO_DefaultBufferSize % eTO_OneDiskBufferSize % eTO_TwoDisksBufferSize % eTO_CDBufferSize % eTO_LANBufferSize % eTO_UseOnlyDefaultBuffer);

	BUFFERSIZES bs;
	bs.m_bOnlyDefault = GetTaskPropValue<eTO_UseOnlyDefaultBuffer>(m_tTaskDefinition.GetConfiguration());
	bs.m_uiDefaultSize = GetTaskPropValue<eTO_DefaultBufferSize>(m_tTaskDefinition.GetConfiguration());
	bs.m_uiOneDiskSize = GetTaskPropValue<eTO_OneDiskBufferSize>(m_tTaskDefinition.GetConfiguration());
	bs.m_uiTwoDisksSize = GetTaskPropValue<eTO_TwoDisksBufferSize>(m_tTaskDefinition.GetConfiguration());
	bs.m_uiCDSize = GetTaskPropValue<eTO_CDBufferSize>(m_tTaskDefinition.GetConfiguration());
	bs.m_uiLANSize = GetTaskPropValue<eTO_LANBufferSize>(m_tTaskDefinition.GetConfiguration());

	ccp.dbBuffer.Create(&bs);

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
	fmt.SetParam(_t("%dstpath"), m_tTaskDefinition.GetDestinationPath().ToString());
	fmt.SetParam(_t("%currindex"), m_tTaskBasicProgressInfo.GetCurrentIndex());

	m_log.logi(fmt);

	for(size_t stIndex = m_tTaskBasicProgressInfo.GetCurrentIndex(); stIndex < stSize; stIndex++)
	{
		// should we kill ?
		if(m_workerThread.KillRequested())
		{
			// log
			m_log.logi(_T("Kill request while processing file in ProcessFiles"));
			return eSubResult_KillRequest;
		}

		// update m_stNextIndex, getting current CFileInfo
		CFileInfoPtr spFileInfo = m_files.GetAt(m_tTaskBasicProgressInfo.GetCurrentIndex());

		// set dest path with filename
		ccp.pathDstFile = GetDestinationPath(spFileInfo, m_tTaskDefinition.GetDestinationPath(), ((int)bForceDirectories) << 1 | (int)bIgnoreFolders);

		// are the files/folders lie on the same partition ?
		int iDstDriveNumber = 0;
		bool bMove = m_tTaskDefinition.GetOperationType() == eOperation_Move;
		if(bMove)
			GetDriveData(m_tTaskDefinition.GetDestinationPath(), &iDstDriveNumber, NULL);
		if(bMove && iDstDriveNumber != -1 && iDstDriveNumber == spFileInfo->GetDriveNumber() && spFileInfo->GetMove())
		{
			bool bRetry = true;
			if(bRetry && !MoveFile(spFileInfo->GetFullFilePath().ToString(), ccp.pathDstFile.ToString()))
			{
				dwLastError=GetLastError();
				//log
				fmt.SetFormat(_T("Error %errno while calling MoveFile %srcpath -> %dstpath (ProcessFiles)"));
				fmt.SetParam(_t("%errno"), dwLastError);
				fmt.SetParam(_t("%srcpath"), spFileInfo->GetFullFilePath().ToString());
				fmt.SetParam(_t("%dstpath"), ccp.pathDstFile.ToString());
				m_log.loge(fmt);

				FEEDBACK_FILEERROR ferr = { spFileInfo->GetFullFilePath().ToString(), ccp.pathDstFile.ToString(), eFastMoveError, dwLastError };
				CFeedbackHandler::EFeedbackResult frResult = (CFeedbackHandler::EFeedbackResult)m_piFeedbackHandler->RequestFeedback(CFeedbackHandler::eFT_FileError, &ferr);
				switch(frResult)
				{
				case CFeedbackHandler::eResult_Cancel:
					return eSubResult_CancelRequest;

				case CFeedbackHandler::eResult_Retry:
					continue;

				case CFeedbackHandler::eResult_Pause:
					return eSubResult_PauseRequest;

				case CFeedbackHandler::eResult_Skip:
					bRetry = false;
					break;		// just do nothing
				default:
					BOOST_ASSERT(FALSE);		// unknown result
					THROW(_T("Unhandled case"), 0, 0, 0);
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
				if(bRetry && !CreateDirectory(ccp.pathDstFile.ToString(), NULL) && (dwLastError=GetLastError()) != ERROR_ALREADY_EXISTS )
				{
					// log
					fmt.SetFormat(_T("Error %errno while calling CreateDirectory %path (ProcessFiles)"));
					fmt.SetParam(_t("%errno"), dwLastError);
					fmt.SetParam(_t("%path"), ccp.pathDstFile.ToString());
					m_log.loge(fmt);

					FEEDBACK_FILEERROR ferr = { ccp.pathDstFile.ToString(), NULL, eCreateError, dwLastError };
					CFeedbackHandler::EFeedbackResult frResult = (CFeedbackHandler::EFeedbackResult)m_piFeedbackHandler->RequestFeedback(CFeedbackHandler::eFT_FileError, &ferr);
					switch(frResult)
					{
					case CFeedbackHandler::eResult_Cancel:
						return eSubResult_CancelRequest;

					case CFeedbackHandler::eResult_Retry:
						continue;

					case CFeedbackHandler::eResult_Pause:
						return eSubResult_PauseRequest;

					case CFeedbackHandler::eResult_Skip:
						bRetry = false;
						break;		// just do nothing
					default:
						BOOST_ASSERT(FALSE);		// unknown result
						THROW(_T("Unhandled case"), 0, 0, 0);
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
				ESubOperationResult eResult = CustomCopyFileFB(&ccp);
				if(eResult != eSubResult_Continue)
					return eResult;

				spFileInfo->SetFlags(ccp.bProcessed ? FIF_PROCESSED : 0, FIF_PROCESSED);

				// if moving - delete file (only if config flag is set)
				if(bMove && spFileInfo->GetFlags() & FIF_PROCESSED && !GetTaskPropValue<eTO_DeleteInSeparateSubTask>(m_tTaskDefinition.GetConfiguration()))
				{
					if(!GetTaskPropValue<eTO_ProtectReadOnlyFiles>(m_tTaskDefinition.GetConfiguration()))
						SetFileAttributes(spFileInfo->GetFullFilePath().ToString(), FILE_ATTRIBUTE_NORMAL);
					DeleteFile(spFileInfo->GetFullFilePath().ToString());	// there will be another try later, so I don't check
					// if succeeded
				}
			}

			// set a time
			if(GetTaskPropValue<eTO_SetDestinationDateTime>(m_tTaskDefinition.GetConfiguration()))
				SetFileDirectoryTime(ccp.pathDstFile.ToString(), spFileInfo); // no error checking (but most probably it should be checked)

			// attributes
			if(GetTaskPropValue<eTO_SetDestinationAttributes>(m_tTaskDefinition.GetConfiguration()))
				SetFileAttributes(ccp.pathDstFile.ToString(), spFileInfo->GetAttributes());	// as above
		}

		m_tTaskBasicProgressInfo.SetCurrentIndex(stIndex + 1);
	}

	// delete buffer - it's not needed
	ccp.dbBuffer.Delete();

	// to look better (as 100%) - increase current index by 1
	m_tTaskBasicProgressInfo.SetCurrentIndex(stSize);

	// log
	m_log.logi(_T("Finished processing in ProcessFiles"));

	return eSubResult_Continue;
}

CTask::ESubOperationResult CTask::CheckForWaitState()
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
			return eSubResult_KillRequest;
		}
	}

	return eSubResult_Continue;
}

CTask::ESubOperationResult CTask::CheckForFreeSpaceFB()
{
	ull_t ullNeededSize = 0, ullAvailableSize = 0;
	bool bRetry = false;

	do
	{
		bRetry = false;

		m_log.logi(_T("Checking for free space on destination disk..."));

		if(!GetRequiredFreeSpace(&ullNeededSize, &ullAvailableSize))
		{
			ictranslate::CFormat fmt;
			fmt.SetFormat(_T("Not enough free space on disk - needed %needsize bytes for data, available: %availablesize bytes."));
			fmt.SetParam(_t("%needsize"), ullNeededSize);
			fmt.SetParam(_t("%availablesize"), ullAvailableSize);
			m_log.logw(fmt);

			if(m_tTaskDefinition.GetSourcePathCount() > 0)
			{
				FEEDBACK_NOTENOUGHSPACE feedStruct = { ullNeededSize, m_arrSourcePaths.GetAt(0)->GetPath().ToString(), m_tTaskDefinition.GetDestinationPath().ToString() };
				CFeedbackHandler::EFeedbackResult frResult = (CFeedbackHandler::EFeedbackResult)m_piFeedbackHandler->RequestFeedback(CFeedbackHandler::eFT_NotEnoughSpace, &feedStruct);

				// default
				switch(frResult)
				{
				case CFeedbackHandler::eResult_Cancel:
					m_log.logi(_T("Cancel request while checking for free space on disk."));
					return eSubResult_CancelRequest;

				case CFeedbackHandler::eResult_Retry:
					m_log.logi(_T("Retrying to read drive's free space..."));
					bRetry = true;
					break;

				case CFeedbackHandler::eResult_Ignore:
					m_log.logi(_T("Ignored warning about not enough place on disk to copy data."));
					return eSubResult_Continue;

				default:
					BOOST_ASSERT(FALSE);		// unknown result
					THROW(_T("Unhandled case"), 0, 0, 0);
				}
			}
		}
	}
	while(bRetry);

	return eSubResult_Continue;
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
	try
	{
		CTask::ESubOperationResult eResult = eSubResult_Continue;

		// initialize log file
		CString strPath = GetRelatedPath(ePathType_TaskLogFile);

		m_log.init(strPath, 262144, icpf::log_file::level_debug, false, false);

		// start operation
		OnBeginOperation();

		// enable configuration changes tracking
		m_tTaskDefinition.GetConfiguration().ConnectToNotifier(TTaskConfigTracker::NotificationProc, &m_cfgTracker);
		m_tTaskDefinition.GetConfiguration().ConnectToNotifier(CTask::OnCfgOptionChanged, this);

		// set thread options
		HANDLE hThread = GetCurrentThread();
		::SetThreadPriorityBoost(hThread, GetTaskPropValue<eTO_DisablePriorityBoost>(m_tTaskDefinition.GetConfiguration()));

		// determine when to scan directories
		bool bReadTasksSize = GetTaskPropValue<eTO_ScanDirectoriesBeforeBlocking>(m_tTaskDefinition.GetConfiguration());

		// wait for permission to really start (but only if search for files is not allowed to start regardless of the lock)
		size_t stSubOperationIndex = m_tTaskBasicProgressInfo.GetSubOperationIndex();
		if(!bReadTasksSize || stSubOperationIndex != 0 || m_tTaskDefinition.GetOperationPlan().GetSubOperationsCount() == 0 || m_tTaskDefinition.GetOperationPlan().GetSubOperationAt(0) != eSubOperation_Scanning)
			eResult = CheckForWaitState();	// operation limiting

		// start tracking time for this thread
		m_localStats.EnableTimeTracking();

		for(; stSubOperationIndex < m_tTaskDefinition.GetOperationPlan().GetSubOperationsCount() && eResult == eSubResult_Continue; ++stSubOperationIndex)
		{
			// set current sub-operation index to allow resuming
			m_tTaskBasicProgressInfo.SetSubOperationIndex(stSubOperationIndex);

			ESubOperationType eSubOperation = m_tTaskDefinition.GetOperationPlan().GetSubOperationAt(stSubOperationIndex);
			switch(eSubOperation)
			{
			case eSubOperation_Scanning:
				// get rid of info about processed sizes
				m_localStats.SetProcessedSize(0);
				m_localStats.SetTotalSize(0);

				// start searching
				eResult = RecurseDirectories();

				// check for free space
				if(eResult == eSubResult_Continue)
					eResult = CheckForFreeSpaceFB();

				// if we didn't wait for permission to start earlier, then ask now (but only in case this is the first search)
				if(eResult == eSubResult_Continue && bReadTasksSize && stSubOperationIndex == 0)
				{
					m_localStats.DisableTimeTracking();

					eResult = CheckForWaitState();

					m_localStats.EnableTimeTracking();
				}

				break;

			case eSubOperation_Copying:
				eResult = ProcessFiles();
				break;

			case eSubOperation_Deleting:
				eResult = DeleteFiles();
				break;

			default:
				BOOST_ASSERT(false);
				THROW(_T("Unhandled case"), 0, 0, 0);
			}
		}

		// refresh time
		m_localStats.DisableTimeTracking();

		// finishing processing
		// change task status
		switch(eResult)
		{
		case eSubResult_Error:
			m_piFeedbackHandler->RequestFeedback(CFeedbackHandler::eFT_OperationError, NULL);
			SetTaskState(eTaskState_Error);
			break;

		case eSubResult_CancelRequest:
			SetTaskState(eTaskState_Cancelled);
			break;

		case eSubResult_PauseRequest:
			SetTaskState(eTaskState_Paused);
			break;

		case eSubResult_KillRequest:
			// the only operation 
			if(GetTaskState() == eTaskState_Waiting)
				SetTaskState(eTaskState_Processing);
			break;

		case eSubResult_Continue:
			m_piFeedbackHandler->RequestFeedback(CFeedbackHandler::eFT_OperationFinished, NULL);
			SetTaskState(eTaskState_Finished);
			break;

		default:
			BOOST_ASSERT(false);
			THROW(_T("Unhandled case"), 0, 0, 0);
		}

		// perform cleanup dependent on currently executing subtask
		switch(m_tTaskDefinition.GetOperationPlan().GetSubOperationAt(m_tTaskBasicProgressInfo.GetSubOperationIndex()))
		{
		case eSubOperation_Scanning:
			m_files.Clear();		// get rid of m_files contents
			m_bRareStateModified = true;
			break;
		}

		// save progress before killed
		m_bOftenStateModified = true;
		Store();

		// reset flags
		SetContinueFlag(false);
		SetForceFlag(false);

		// mark this task as dead, so other can start
		m_localStats.MarkTaskAsNotRunning();

		m_tTaskDefinition.GetConfiguration().DisconnectFromNotifier(TTaskConfigTracker::NotificationProc);
		m_tTaskDefinition.GetConfiguration().DisconnectFromNotifier(CTask::OnCfgOptionChanged);

		// and the real end
		OnEndOperation();
	}
	catch(...)
	{
		m_tTaskDefinition.GetConfiguration().DisconnectFromNotifier(TTaskConfigTracker::NotificationProc);
		m_tTaskDefinition.GetConfiguration().DisconnectFromNotifier(CTask::OnCfgOptionChanged);

		// refresh time
		m_localStats.DisableTimeTracking();

		// log
		ictranslate::CFormat fmt;

		fmt.SetFormat(_T("Caught exception in ThrdProc"));
		m_log.loge(fmt);

		// let others know some error happened
		m_piFeedbackHandler->RequestFeedback(CFeedbackHandler::eFT_OperationError, NULL);
		SetTaskState(eTaskState_Error);

		m_localStats.MarkTaskAsNotRunning();

		SetContinueFlag(false);
		SetForceFlag(false);

		OnEndOperation();
		return 1;
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

CString CTask::GetRelatedPath(EPathType ePathType)
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);

	return GetRelatedPathNL(ePathType);
}

CString CTask::GetRelatedPathNL(EPathType ePathType)
{
	BOOST_ASSERT(!m_strTaskDirectory.IsEmpty() || !m_strFilePath.IsEmpty());
	if(m_strTaskDirectory.IsEmpty() && m_strFilePath.IsEmpty())
		THROW(_t("Missing task path."), 0, 0, 0);

	// in all cases we would like to have task definition path defined
	CString strFilePath = m_strFilePath;
	if(strFilePath.IsEmpty())
		strFilePath = m_strTaskDirectory + m_tTaskDefinition.GetTaskUniqueID() + _T(".cht");

	switch(ePathType)
	{
	case ePathType_TaskDefinition:
		return strFilePath;

	case ePathType_TaskRarelyChangingState:
		return strFilePath + _T(".rstate");

	case ePathType_TaskOftenChangingState:
		return strFilePath + _T(".ostate");

	case ePathType_TaskLogFile:
		return strFilePath + _T(".log");

	default:
		THROW(_t("Unhandled case"), 0, 0, 0);
	}
}

void CTask::OnCfgOptionChanged(const std::set<CString>& rsetChanges, void* pParam)
{
	CTask* pTask = (CTask*)pParam;
	if(!pTask)
		THROW(_T("Invalid pointer"), 0, 0, 0);

	if(rsetChanges.find(TaskPropData<eTO_ThreadPriority>::GetPropertyName()) != rsetChanges.end())
	{
		pTask->m_workerThread.ChangePriority(GetTaskPropValue<eTO_ThreadPriority>(pTask->GetTaskDefinition().GetConfiguration()));
	}
}

// finds another name for a copy of src file(folder) in dest location
chcore::TSmartPath CTask::FindFreeSubstituteName(chcore::TSmartPath pathSrcPath, chcore::TSmartPath pathDstPath) const
{
	// get the name from srcpath
	pathSrcPath.CutIfExists(_T("\\"));
	pathDstPath.AppendIfNotExists(_T("\\"));

	chcore::TSmartPath pathLastComponent = pathSrcPath.GetLastComponent();

	// set the dest path
	CString strCheckPath;
	ictranslate::CFormat fmt(GetTaskPropValue<eTO_AlternateFilenameFormatString_First>(m_tTaskDefinition.GetConfiguration()));
	fmt.SetParam(_t("%name"), pathLastComponent.ToString());
	chcore::TSmartPath pathCheckPath(chcore::PathFromString((PCTSTR)fmt));

	// when adding to strDstPath check if the path already exists - if so - try again
	int iCounter=1;
	CString strFmt = GetTaskPropValue<eTO_AlternateFilenameFormatString_AfterFirst>(m_tTaskDefinition.GetConfiguration());
	while(CFileInfo::Exist(pathDstPath + pathCheckPath))
	{
		fmt.SetFormat(strFmt);
		fmt.SetParam(_t("%name"), pathLastComponent.ToString());
		fmt.SetParam(_t("%count"), ++iCounter);
		pathCheckPath.FromString((PCTSTR)fmt);
	}

	return pathCheckPath;
}

chcore::TSmartPath CTask::GetDestinationPath(const CFileInfoPtr& spFileInfo, chcore::TSmartPath pathDst, int iFlags) const
{
	if(!spFileInfo)
		THROW(_T("Invalid pointer"), 0, 0, 0);

	// add '\\'
	pathDst.AppendIfNotExists(_T("\\"), false);

	// iFlags: bit 0-ignore folders; bit 1-force creating directories
	if (iFlags & 0x02)
	{
		// force create directories
		TCHAR dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];
		_tsplitpath(spFileInfo->GetFullFilePath().ToString(), NULL, dir, fname, ext);

		pathDst.CutIfExists(_T("\\"), false);

		// force create directory
		SHCreateDirectoryEx(NULL, CString(pathDst.ToString()) + dir, NULL);

		return pathDst + chcore::PathFromString(dir) + chcore::PathFromString(fname) + chcore::PathFromString(ext);
	}
	else
	{
		size_t stSrcIndex = spFileInfo->GetSrcIndex();

		if (!(iFlags & 0x01) && stSrcIndex != std::numeric_limits<size_t>::max())
		{
			// generate new dest name
			if(!m_arrSourcePaths.GetAt(stSrcIndex)->IsDestinationPathSet())
			{
				chcore::TSmartPath pathSubst = FindFreeSubstituteName(spFileInfo->GetFullFilePath(), pathDst);
				m_arrSourcePaths.GetAt(stSrcIndex)->SetDestinationPath(pathSubst);
			}

			return pathDst + m_arrSourcePaths.GetAt(stSrcIndex)->GetDestinationPath() + spFileInfo->GetFilePath();
		}
		else
			return pathDst + spFileInfo->GetFileName();
	}
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
	spNewTask->SetTaskDirectory(m_strTasksDir.c_str());
	
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

	// separate scope for locking
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);

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
	}

	BOOST_FOREACH(CTaskPtr& spTask, vTasksToRemove)
	{
		// delete associated files
		spTask->DeleteProgress();
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
			spTask->DeleteProgress();
			
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
		spTask->Store();
	}
}

void CTaskArray::LoadDataProgress()
{
	CFileFind finder;
	CTaskPtr spTask;
	CString strPath;

	// find all CH Task files
	BOOL bWorking = finder.FindFile(CString(m_strTasksDir.c_str()) + _T("*.cht"));
	while(bWorking)
	{
		bWorking = finder.FindNextFile();
		
		// load data
		spTask = CreateTask();
		try
		{
			spTask->Load(finder.GetFilePath());
			
			// add read task to array
			Add(spTask);
		}
		catch(std::exception& e)
		{
			CString strFmt;
			strFmt.Format(_T("Cannot load task data: %s (reason: %S)"), (PCTSTR)strPath, e.what());
			LOG_ERROR(strFmt);
		}
		catch(icpf::exception& e)
		{
			CString strMsg;
			e.get_info(strMsg.GetBufferSetLength(65536), 65536);
			strMsg.ReleaseBuffer();

			CString strFmt;
			strFmt.Format(_T("Cannot load task data: %s (reason: %s)"), (PCTSTR)strPath, (PCTSTR)strMsg);
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
