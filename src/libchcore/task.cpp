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

#pragma warning(push)
#pragma warning(disable: 4996)
	#include <boost/serialization/serialization.hpp>
	#include <boost/archive/binary_oarchive.hpp>
	#include <boost/archive/binary_iarchive.hpp>
#pragma warning(pop)

#include <fstream>
#include "TTaskConfiguration.h"
#include "TSubTaskContext.h"
#include "TLocalFilesystem.h"
#include "TSubTaskScanDirectory.h"
#include "TSubTaskCopyMove.h"
#include "TSubTaskDelete.h"
#include "TBinarySerializer.h"
#include "SerializationHelpers.h"
#include <boost/lexical_cast.hpp>
#include <boost/smart_ptr/shared_array.hpp>
#include "../libicpf/exception.h"
#include <atlconv.h>
#include "TLogger.h"

BEGIN_CHCORE_NAMESPACE

////////////////////////////////////////////////////////////////////////////
// CTask members

CTask::CTask(IFeedbackHandler* piFeedbackHandler, size_t stSessionUniqueID) :
	m_log(),
	m_piFeedbackHandler(piFeedbackHandler),
	m_arrSourcePathsInfo(m_tTaskDefinition.GetSourcePaths()),
	m_files(m_tTaskDefinition.GetSourcePaths()),
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

	m_arrSourcePathsInfo.SetCount(m_tTaskDefinition.GetSourcePathCount());
	m_files.Clear();
}

void CTask::OnRegisterTask(TTasksGlobalStats& rtGlobalStats)
{
	m_localStats.ConnectGlobalStats(rtGlobalStats);
}

void CTask::OnUnregisterTask()
{
	m_localStats.DisconnectGlobalStats();
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

void CTask::SetBufferSizes(const TBufferSizes& bsSizes)
{
	m_tTaskDefinition.GetConfiguration().DelayNotifications();
	SetTaskPropValue<eTO_DefaultBufferSize>(m_tTaskDefinition.GetConfiguration(), bsSizes.GetDefaultSize());
	SetTaskPropValue<eTO_OneDiskBufferSize>(m_tTaskDefinition.GetConfiguration(), bsSizes.GetOneDiskSize());
	SetTaskPropValue<eTO_TwoDisksBufferSize>(m_tTaskDefinition.GetConfiguration(), bsSizes.GetTwoDisksSize());
	SetTaskPropValue<eTO_CDBufferSize>(m_tTaskDefinition.GetConfiguration(), bsSizes.GetCDSize());
	SetTaskPropValue<eTO_LANBufferSize>(m_tTaskDefinition.GetConfiguration(), bsSizes.GetLANSize());
	SetTaskPropValue<eTO_UseOnlyDefaultBuffer>(m_tTaskDefinition.GetConfiguration(), bsSizes.IsOnlyDefault());
	m_tTaskDefinition.GetConfiguration().ResumeNotifications();
}

void CTask::GetBufferSizes(TBufferSizes& bsSizes)
{
	bsSizes.SetDefaultSize(GetTaskPropValue<eTO_DefaultBufferSize>(m_tTaskDefinition.GetConfiguration()));
	bsSizes.SetOneDiskSize(GetTaskPropValue<eTO_OneDiskBufferSize>(m_tTaskDefinition.GetConfiguration()));
	bsSizes.SetTwoDisksSize(GetTaskPropValue<eTO_TwoDisksBufferSize>(m_tTaskDefinition.GetConfiguration()));
	bsSizes.SetCDSize(GetTaskPropValue<eTO_CDBufferSize>(m_tTaskDefinition.GetConfiguration()));
	bsSizes.SetLANSize(GetTaskPropValue<eTO_LANBufferSize>(m_tTaskDefinition.GetConfiguration()));
	bsSizes.SetOnlyDefault(GetTaskPropValue<eTO_UseOnlyDefaultBuffer>(m_tTaskDefinition.GetConfiguration()));
}

int CTask::GetCurrentBufferIndex()
{
	return m_localStats.GetCurrentBufferIndex();
}

// thread
void CTask::SetPriority(int nPriority)
{
	SetTaskPropValue<eTO_ThreadPriority>(m_tTaskDefinition.GetConfiguration(), nPriority);
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

void CTask::Load(const TSmartPath& strPath)
{
	using Serializers::Serialize;

	boost::unique_lock<boost::shared_mutex> lock(m_lock);

	////////////////////////////////
	// First load task description
	m_tTaskDefinition.Load(strPath);
	m_strFilePath = strPath;

	// update members according to the task definition
	// make sure to resize paths info array size to match source paths count
	m_arrSourcePathsInfo.SetCount(m_tTaskDefinition.GetSourcePathCount());
	GetTaskPropValue<eTO_Filters>(m_tTaskDefinition.GetConfiguration(), m_afFilters);

	////////////////////////////////
	// now rarely changing task progress data
	TSmartPath pathRarelyChangingPath = GetRelatedPathNL(ePathType_TaskRarelyChangingState);
	TReadBinarySerializer readSerializer;
	readSerializer.Init(pathRarelyChangingPath);

	m_arrSourcePathsInfo.Serialize(readSerializer, true);
	m_files.Serialize(readSerializer, false);

	CalculateTotalSizeNL();

	///////////////////////////////////
	// and often changing data
	TSmartPath pathOftenChangingPath = GetRelatedPathNL(ePathType_TaskOftenChangingState);
	readSerializer.Init(pathOftenChangingPath);

	Serialize(readSerializer, m_tTaskBasicProgressInfo);

	CalculateProcessedSizeNL();

	// load task state, convert "waiting" state to "processing"
	int iState = eTaskState_None;
	Serialize(readSerializer, iState);
	if(iState >= eTaskState_None && iState < eTaskState_Max)
	{
		if(iState == eTaskState_Waiting)
			iState = eTaskState_Processing;
		m_eCurrentState = (ETaskCurrentState)iState;
	}
	else
	{
		BOOST_ASSERT(false);
		THROW_CORE_EXCEPTION(eErr_InvalidSerializationData);
	}

	time_t timeElapsed = 0;
	Serialize(readSerializer, timeElapsed);
	m_localStats.SetTimeElapsed(timeElapsed);

	m_arrSourcePathsInfo.Serialize(readSerializer, false);
	m_files.Serialize(readSerializer, true);
}

void CTask::Store()
{
	using Serializers::Serialize;

	boost::upgrade_lock<boost::shared_mutex> lock(m_lock);

	BOOST_ASSERT(!m_strTaskDirectory.IsEmpty());
	if(m_strTaskDirectory.IsEmpty())
		THROW_CORE_EXCEPTION(eErr_MissingTaskSerializationPath);

	// generate file path if not available yet
	if(m_strFilePath.IsEmpty())
	{
		boost::upgrade_to_unique_lock<boost::shared_mutex> upgraded_lock(lock);
		m_strFilePath = m_strTaskDirectory + PathFromWString(m_tTaskDefinition.GetTaskUniqueID() + _T(".cht"));
	}

	// store task definition only if changed
	m_tTaskDefinition.Store(GetRelatedPathNL(ePathType_TaskDefinition), true);

	// rarely changing data
	if(m_bRareStateModified)
	{
		TWriteBinarySerializer writeSerializer;
		writeSerializer.Init(GetRelatedPathNL(ePathType_TaskRarelyChangingState));

		m_arrSourcePathsInfo.Serialize(writeSerializer, true);

		ESubOperationType eSubOperation = m_tTaskDefinition.GetOperationPlan().GetSubOperationAt(m_tTaskBasicProgressInfo.GetSubOperationIndex());
		if(eSubOperation != eSubOperation_Scanning)
			m_files.Serialize(writeSerializer, false);
		else
		{
			size_t stFakeSize(0);
			Serialize(writeSerializer, stFakeSize);
		}
	}

	if(m_bOftenStateModified)
	{
		TWriteBinarySerializer writeSerializer;
		writeSerializer.Init(GetRelatedPathNL(ePathType_TaskOftenChangingState));

		Serialize(writeSerializer, m_tTaskBasicProgressInfo);

		// store current state (convert from waiting to processing state before storing)
		int iState = m_eCurrentState;
		if(iState == eTaskState_Waiting)
			iState = eTaskState_Processing;

		Serialize(writeSerializer, iState);

		time_t timeElapsed = m_localStats.GetTimeElapsed();
		Serialize(writeSerializer, timeElapsed);

		m_arrSourcePathsInfo.Serialize(writeSerializer, false);

		ESubOperationType eSubOperation = m_tTaskDefinition.GetOperationPlan().GetSubOperationAt(m_tTaskBasicProgressInfo.GetSubOperationIndex());
		if(eSubOperation != eSubOperation_Scanning)
			m_files.Serialize(writeSerializer, true);
		else
		{
			size_t stFakeSize(0);
			Serialize(writeSerializer, stFakeSize);
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
		pData->m_strPath = m_files.GetAt(stCurrentIndex)->GetFullFilePath().GetFileName().ToString();
	else
	{
		if(m_files.GetSize() > 0)
			pData->m_strPath = m_files.GetAt(0)->GetFullFilePath().GetFileName().ToString();
		else
		{
			if(m_tTaskDefinition.GetSourcePathCount() > 0)
				pData->m_strPath = m_tTaskDefinition.GetSourcePathAt(0).GetFileName().ToString();
			else
				pData->m_strPath.Clear();
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
		pData->m_strFileName = m_files.GetAt(stCurrentIndex)->GetFullFilePath().GetFileName().ToString();
	}
	else
	{
		if(m_files.GetSize() > 0)
		{
			pData->m_strFullFilePath = m_files.GetAt(0)->GetFullFilePath().ToString();
			pData->m_strFileName = m_files.GetAt(0)->GetFullFilePath().GetFileName().ToString();
		}
		else
		{
			if(m_tTaskDefinition.GetSourcePathCount() > 0)
			{
				pData->m_strFullFilePath = m_tTaskDefinition.GetSourcePathAt(0).ToString();
				pData->m_strFileName = m_tTaskDefinition.GetSourcePathAt(0).GetFileName().ToString();
			}
			else
			{
				pData->m_strFullFilePath.Clear();
				pData->m_strFileName.Clear();
			}
		}
	}

	pData->m_nPriority = GetTaskPropValue<eTO_ThreadPriority>(m_tTaskDefinition.GetConfiguration());
	pData->m_pathDstPath = m_tTaskDefinition.GetDestinationPath();
	pData->m_pafFilters = &m_afFilters;
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
		pData->m_iCurrentBufferIndex = m_localStats.GetCurrentBufferIndex();
	else
		pData->m_iCurrentBufferIndex = TBufferSizes::eBuffer_Default;

	switch(pData->m_iCurrentBufferIndex)
	{
	case TBufferSizes::eBuffer_Default:
		pData->m_iCurrentBufferSize = GetTaskPropValue<eTO_DefaultBufferSize>(m_tTaskDefinition.GetConfiguration());
		break;
	case TBufferSizes::eBuffer_OneDisk:
		pData->m_iCurrentBufferSize = GetTaskPropValue<eTO_OneDiskBufferSize>(m_tTaskDefinition.GetConfiguration());
		break;
	case TBufferSizes::eBuffer_TwoDisks:
		pData->m_iCurrentBufferSize = GetTaskPropValue<eTO_TwoDisksBufferSize>(m_tTaskDefinition.GetConfiguration());
		break;
	case TBufferSizes::eBuffer_CD:
		pData->m_iCurrentBufferSize = GetTaskPropValue<eTO_CDBufferSize>(m_tTaskDefinition.GetConfiguration());
		break;
	case TBufferSizes::eBuffer_LAN:
		pData->m_iCurrentBufferSize = GetTaskPropValue<eTO_LANBufferSize>(m_tTaskDefinition.GetConfiguration());
		break;
	default:
		THROW_CORE_EXCEPTION(eErr_UnhandledCase);
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
	TPathContainer vFilesToRemove;

	// separate scope for shared locking
	{
		boost::shared_lock<boost::shared_mutex> lock(m_lock);

		vFilesToRemove.Add(GetRelatedPath(ePathType_TaskDefinition));
		vFilesToRemove.Add(GetRelatedPath(ePathType_TaskRarelyChangingState));
		vFilesToRemove.Add(GetRelatedPath(ePathType_TaskOftenChangingState));
		vFilesToRemove.Add(GetRelatedPath(ePathType_TaskLogFile));
	}

	for(size_t stIndex = 0; stIndex < vFilesToRemove.GetCount(); ++stIndex)
	{
		DeleteFile(vFilesToRemove.GetAt(stIndex).ToString());
	}
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

void CTask::SetTaskDirectory(const TSmartPath& strDir)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_strTaskDirectory = strDir;
}

TSmartPath CTask::GetTaskDirectory() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_strTaskDirectory;
}

void CTask::SetTaskFilePath(const TSmartPath& strFilePath)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_strFilePath = strFilePath;
}

TSmartPath CTask::GetTaskFilePath() const
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

void CTask::CalculateTotalSizeNL()
{
	m_localStats.SetTotalSize(m_files.CalculateTotalSize());
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

TSubTaskBase::ESubOperationResult CTask::CheckForWaitState()
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
			return TSubTaskBase::eSubResult_KillRequest;
		}
	}

	return TSubTaskBase::eSubResult_Continue;
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
		TSubTaskBase::ESubOperationResult eResult = TSubTaskBase::eSubResult_Continue;

		// initialize log file
		TSmartPath pathLogFile = GetRelatedPath(ePathType_TaskLogFile);

		m_log.init(pathLogFile.ToString(), 262144, icpf::log_file::level_debug, false, false);

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

		// prepare context for subtasks
		TSubTaskContext tSubTaskContext(m_tTaskDefinition, m_arrSourcePathsInfo, m_files, m_localStats, m_tTaskBasicProgressInfo, m_cfgTracker, m_log, m_piFeedbackHandler, m_workerThread, m_fsLocal);

		for(; stSubOperationIndex < m_tTaskDefinition.GetOperationPlan().GetSubOperationsCount() && eResult == TSubTaskBase::eSubResult_Continue; ++stSubOperationIndex)
		{
			// set current sub-operation index to allow resuming
			m_tTaskBasicProgressInfo.SetSubOperationIndex(stSubOperationIndex);

			ESubOperationType eSubOperation = m_tTaskDefinition.GetOperationPlan().GetSubOperationAt(stSubOperationIndex);
			switch(eSubOperation)
			{
			case eSubOperation_Scanning:
				{
					// start searching
					TSubTaskScanDirectories tSubTaskScanDir(tSubTaskContext);
					eResult = tSubTaskScanDir.Exec();

					// if we didn't wait for permission to start earlier, then ask now (but only in case this is the first search)
					if(eResult == TSubTaskBase::eSubResult_Continue && bReadTasksSize && stSubOperationIndex == 0)
					{
						m_localStats.DisableTimeTracking();

						eResult = CheckForWaitState();

						m_localStats.EnableTimeTracking();
					}

					break;
				}

			case eSubOperation_Copying:
				{
					TSubTaskCopyMove tSubTaskCopyMove(tSubTaskContext);

					eResult = tSubTaskCopyMove.Exec();
					break;
				}

			case eSubOperation_Deleting:
				{
					TSubTaskDelete tSubTaskDelete(tSubTaskContext);
					eResult = tSubTaskDelete.Exec();
					break;
				}

			default:
				BOOST_ASSERT(false);
				THROW_CORE_EXCEPTION(eErr_UnhandledCase);
			}
		}

		// change status to finished
		if(eResult == TSubTaskBase::eSubResult_Continue)
			SetTaskState(eTaskState_Finished);

		// refresh time
		m_localStats.DisableTimeTracking();

		// finishing processing
		// change task status
		switch(eResult)
		{
		case TSubTaskBase::eSubResult_Error:
			m_piFeedbackHandler->RequestFeedback(IFeedbackHandler::eFT_OperationError, NULL);
			SetTaskState(eTaskState_Error);
			break;

		case TSubTaskBase::eSubResult_CancelRequest:
			SetTaskState(eTaskState_Cancelled);
			break;

		case TSubTaskBase::eSubResult_PauseRequest:
			SetTaskState(eTaskState_Paused);
			break;

		case TSubTaskBase::eSubResult_KillRequest:
			// the only operation 
			if(GetTaskState() == eTaskState_Waiting)
				SetTaskState(eTaskState_Processing);
			break;

		case TSubTaskBase::eSubResult_Continue:
			m_piFeedbackHandler->RequestFeedback(IFeedbackHandler::eFT_OperationFinished, NULL);
			SetTaskState(eTaskState_Finished);
			break;

		default:
			BOOST_ASSERT(false);
			THROW_CORE_EXCEPTION(eErr_UnhandledCase);
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
		m_log.loge(_T("Caught exception in ThrdProc"));

		// let others know some error happened
		m_piFeedbackHandler->RequestFeedback(IFeedbackHandler::eFT_OperationError, NULL);
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

	TString strFormat = _T("\r\n# COPYING THREAD STARTED #\r\nBegan processing data (dd:mm:yyyy) %day.%month.%year at %hour:%minute.%second");
	strFormat.Replace(_t("%year"), boost::lexical_cast<std::wstring>(tm.GetYear()).c_str());
	strFormat.Replace(_t("%month"), boost::lexical_cast<std::wstring>(tm.GetMonth()).c_str());
	strFormat.Replace(_t("%day"), boost::lexical_cast<std::wstring>(tm.GetDay()).c_str());
	strFormat.Replace(_t("%hour"), boost::lexical_cast<std::wstring>(tm.GetHour()).c_str());
	strFormat.Replace(_t("%minute"), boost::lexical_cast<std::wstring>(tm.GetMinute()).c_str());
	strFormat.Replace(_t("%second"), boost::lexical_cast<std::wstring>(tm.GetSecond()).c_str());
	m_log.logi(strFormat);
}

void CTask::OnEndOperation()
{
	CTime tm=CTime::GetCurrentTime();

	TString strFormat = _T("Finished processing data (dd:mm:yyyy) %day.%month.%year at %hour:%minute.%second");
	strFormat.Replace(_t("%year"), boost::lexical_cast<std::wstring>(tm.GetYear()).c_str());
	strFormat.Replace(_t("%month"), boost::lexical_cast<std::wstring>(tm.GetMonth()).c_str());
	strFormat.Replace(_t("%day"), boost::lexical_cast<std::wstring>(tm.GetDay()).c_str());
	strFormat.Replace(_t("%hour"), boost::lexical_cast<std::wstring>(tm.GetHour()).c_str());
	strFormat.Replace(_t("%minute"), boost::lexical_cast<std::wstring>(tm.GetMinute()).c_str());
	strFormat.Replace(_t("%second"), boost::lexical_cast<std::wstring>(tm.GetSecond()).c_str());
	m_log.logi(strFormat);
}

void CTask::RequestStopThread()
{
	m_workerThread.SignalThreadToStop();
}

TSmartPath CTask::GetRelatedPath(EPathType ePathType)
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);

	return GetRelatedPathNL(ePathType);
}

TSmartPath CTask::GetRelatedPathNL(EPathType ePathType)
{
	BOOST_ASSERT(!m_strTaskDirectory.IsEmpty() || !m_strFilePath.IsEmpty());
	if(m_strTaskDirectory.IsEmpty() && m_strFilePath.IsEmpty())
		THROW_CORE_EXCEPTION(eErr_MissingTaskSerializationPath);

	// in all cases we would like to have task definition path defined
	TSmartPath strFilePath = m_strFilePath;
	if(strFilePath.IsEmpty())
		strFilePath = m_strTaskDirectory + PathFromWString(m_tTaskDefinition.GetTaskUniqueID() + _T(".cht"));

	switch(ePathType)
	{
	case ePathType_TaskDefinition:
		return strFilePath;

	case ePathType_TaskRarelyChangingState:
		return strFilePath.AppendCopy(PathFromString(_T(".rstate")), false);

	case ePathType_TaskOftenChangingState:
		return strFilePath.AppendCopy(PathFromString(_T(".ostate")), false);

	case ePathType_TaskLogFile:
		return strFilePath.AppendCopy(PathFromString(_T(".log")), false);

	default:
		THROW_CORE_EXCEPTION(eErr_UnhandledCase);
	}
}

void CTask::OnCfgOptionChanged(const TStringSet& rsetChanges, void* pParam)
{
	CTask* pTask = (CTask*)pParam;
	if(!pTask)
		THROW_CORE_EXCEPTION(eErr_InvalidArgument);

	if(rsetChanges.HasValue(TaskPropData<eTO_ThreadPriority>::GetPropertyName()))
	{
		pTask->m_workerThread.ChangePriority(GetTaskPropValue<eTO_ThreadPriority>(pTask->GetTaskDefinition().GetConfiguration()));
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

void CTaskArray::Create(IFeedbackHandlerFactory* piFeedbackHandlerFactory)
{
	BOOST_ASSERT(piFeedbackHandlerFactory);

	m_piFeedbackFactory = piFeedbackHandlerFactory;
}

CTaskPtr CTaskArray::CreateTask(const TTaskDefinition& tTaskDefinition)
{
	CTaskPtr spTask = CreateEmptyTask();
	if(spTask)
	{
		spTask->SetTaskDefinition(tTaskDefinition);
		Add(spTask);
		spTask->Store();
	}

	return spTask;
}

CTaskPtr CTaskArray::ImportTask(const TSmartPath& strTaskPath)
{
	// load task definition from the new location
	TTaskDefinition tTaskDefinition;
	tTaskDefinition.Load(strTaskPath);

	return CreateTask(tTaskDefinition);
}

CTaskPtr CTaskArray::CreateEmptyTask()
{
	BOOST_ASSERT(m_piFeedbackFactory);
	if(!m_piFeedbackFactory)
		return CTaskPtr();

	IFeedbackHandler* piHandler = m_piFeedbackFactory->Create();
	if(!piHandler)
		return CTaskPtr();

	BOOST_ASSERT(m_stNextSessionUniqueID != NO_TASK_SESSION_UNIQUE_ID);
	CTaskPtr spTask(new CTask(piHandler, m_stNextSessionUniqueID++));

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
		THROW_CORE_EXCEPTION(eErr_InvalidArgument);

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
		THROW_CORE_EXCEPTION(eErr_InvalidArgument);

	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	// here we know load succeeded
	spNewTask->SetTaskDirectory(m_pathTasksDir);

	m_vTasks.push_back(spNewTask);

	spNewTask->OnRegisterTask(m_globalStats);

	return m_vTasks.size() - 1;
}

void CTaskArray::RemoveAt(size_t stIndex, size_t stCount)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);

	_ASSERTE(stIndex >= m_vTasks.size() || stIndex + stCount > m_vTasks.size());
	if(stIndex >= m_vTasks.size() || stIndex + stCount > m_vTasks.size())
		THROW_CORE_EXCEPTION(eErr_InvalidArgument);

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
	if(m_pathTasksDir.IsEmpty())
		THROW_CORE_EXCEPTION(eErr_MissingTaskSerializationPath);

	CTaskPtr spTask;
	TSmartPath pathFound;
	WIN32_FIND_DATA wfd;
	bool bExceptionEncountered = false;

	const size_t stMaxMsgSize = 4096;
	boost::shared_array<wchar_t> spMsgBuffer(new wchar_t[stMaxMsgSize]);
	spMsgBuffer[0] = _T('\0');

	// find all CH Task files
	TSmartPath pathToFind = m_pathTasksDir + PathFromString(_T("*.cht"));

	HANDLE hFind = ::FindFirstFile(pathToFind.ToString(), &wfd);
	BOOL bContinue = TRUE;
	while(hFind != INVALID_HANDLE_VALUE && bContinue)
	{
		pathFound = m_pathTasksDir + PathFromString(wfd.cFileName);
		// load data
		spTask = CreateEmptyTask();
		try
		{
			spTask->Load(pathFound);

			// add read task to array
			Add(spTask);
		}
		catch(icpf::exception& e)
		{
			e.get_info(spMsgBuffer.get(), stMaxMsgSize);
			bExceptionEncountered = true;
		}
		catch(std::exception& e)
		{
			_tcsncpy_s(spMsgBuffer.get(), stMaxMsgSize, CA2CT(e.what()), _TRUNCATE);
			bExceptionEncountered = true;
		}
		
		if(bExceptionEncountered)
		{
			TString strFmt = _T("Cannot load task data: %path (reason: %reason)");
			strFmt.Replace(_T("%path"), pathFound.ToString());
			strFmt.Replace(_T("%reason"), spMsgBuffer.get());

			LOG_ERROR(strFmt);

			bExceptionEncountered = false;
		}
		bContinue = ::FindNextFile(hFind, &wfd);
	}

	::FindClose(hFind);
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

void CTaskArray::SetTasksDir(const TSmartPath& pathDir)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_pathTasksDir = pathDir;
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

END_CHCORE_NAMESPACE
