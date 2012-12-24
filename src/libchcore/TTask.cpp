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
#include "TTask.h"

#pragma warning(push)
#pragma warning(disable: 4996 4244 4310)
	#include <boost/serialization/serialization.hpp>
	#include <boost/archive/binary_oarchive.hpp>
	#include <boost/archive/binary_iarchive.hpp>
#pragma warning(pop)

#include <fstream>
#include "TSubTaskScanDirectory.h"
#include "TSubTaskCopyMove.h"
#include "TSubTaskDelete.h"
#include "TBinarySerializer.h"
#include "SerializationHelpers.h"
#include <boost/lexical_cast.hpp>
#include "../libicpf/exception.h"
#include <atlconv.h>
#include "DataBuffer.h"
#include "TFileInfo.h"
#include "TSubTaskArray.h"
#include "TTaskStatsSnapshot.h"

BEGIN_CHCORE_NAMESPACE

////////////////////////////////////////////////////////////////////////////
// TTask members

TTask::TTask(IFeedbackHandler* piFeedbackHandler, size_t stSessionUniqueID) :
	m_log(),
	m_piFeedbackHandler(piFeedbackHandler),
	m_arrSourcePathsInfo(m_tTaskDefinition.GetSourcePaths()),
	m_files(m_tTaskDefinition.GetSourcePaths()),
	m_bForce(false),
	m_bContinue(false),
	m_bRareStateModified(false),
	m_bOftenStateModified(false),
	m_stSessionUniqueID(stSessionUniqueID),
	m_eCurrentState(eTaskState_None),
	m_tSubTaskContext(m_tTaskDefinition, m_arrSourcePathsInfo, m_files, m_cfgTracker, m_log, piFeedbackHandler, m_workerThread, m_fsLocal),
	m_tSubTasksArray(m_tLocalStats)
{
	BOOST_ASSERT(piFeedbackHandler);
}

TTask::~TTask()
{
	KillThread();
	if(m_piFeedbackHandler)
		m_piFeedbackHandler->Delete();
}

void TTask::SetTaskDefinition(const TTaskDefinition& rTaskDefinition)
{
	m_tTaskDefinition = rTaskDefinition;

	m_tSubTasksArray.Init(m_tTaskDefinition.GetOperationPlan(), m_tSubTaskContext);
	m_arrSourcePathsInfo.SetCount(m_tTaskDefinition.GetSourcePathCount());
	m_files.Clear();
}

void TTask::OnRegisterTask()
{
}

void TTask::OnUnregisterTask()
{
}

void TTask::SetTaskState(ETaskCurrentState eTaskState)
{
	// NOTE: we could check some transition rules here
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_eCurrentState = eTaskState;
}

ETaskCurrentState TTask::GetTaskState() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_eCurrentState;
}

void TTask::SetBufferSizes(const TBufferSizes& bsSizes)
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

void TTask::GetBufferSizes(TBufferSizes& bsSizes)
{
	bsSizes.SetDefaultSize(GetTaskPropValue<eTO_DefaultBufferSize>(m_tTaskDefinition.GetConfiguration()));
	bsSizes.SetOneDiskSize(GetTaskPropValue<eTO_OneDiskBufferSize>(m_tTaskDefinition.GetConfiguration()));
	bsSizes.SetTwoDisksSize(GetTaskPropValue<eTO_TwoDisksBufferSize>(m_tTaskDefinition.GetConfiguration()));
	bsSizes.SetCDSize(GetTaskPropValue<eTO_CDBufferSize>(m_tTaskDefinition.GetConfiguration()));
	bsSizes.SetLANSize(GetTaskPropValue<eTO_LANBufferSize>(m_tTaskDefinition.GetConfiguration()));
	bsSizes.SetOnlyDefault(GetTaskPropValue<eTO_UseOnlyDefaultBuffer>(m_tTaskDefinition.GetConfiguration()));
}

// thread
void TTask::SetPriority(int nPriority)
{
	SetTaskPropValue<eTO_ThreadPriority>(m_tTaskDefinition.GetConfiguration(), nPriority);
}

void TTask::Load(const TSmartPath& strPath)
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

	///////////////////////////////////
	// and often changing data
	TSmartPath pathOftenChangingPath = GetRelatedPathNL(ePathType_TaskOftenChangingState);
	readSerializer.Init(pathOftenChangingPath);

	m_tSubTasksArray.Init(m_tTaskDefinition.GetOperationPlan(), m_tSubTaskContext);
	m_tSubTasksArray.SerializeProgress(readSerializer);

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
//	m_localStats.SetTimeElapsed(timeElapsed);

	m_arrSourcePathsInfo.Serialize(readSerializer, false);
	m_files.Serialize(readSerializer, true);
}

void TTask::Store()
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

		m_files.Serialize(writeSerializer, false);
	}

	if(m_bOftenStateModified)
	{
		TWriteBinarySerializer writeSerializer;
		writeSerializer.Init(GetRelatedPathNL(ePathType_TaskOftenChangingState));

		m_tSubTasksArray.SerializeProgress(writeSerializer);

		// store current state (convert from waiting to processing state before storing)
		int iState = m_eCurrentState;
		if(iState == eTaskState_Waiting)
			iState = eTaskState_Processing;

		Serialize(writeSerializer, iState);

//		time_t timeElapsed = m_localStats.GetTimeElapsed();
//		Serialize(writeSerializer, timeElapsed);

		m_arrSourcePathsInfo.Serialize(writeSerializer, false);

		m_files.Serialize(writeSerializer, true);
	}
}

void TTask::KillThread()
{
	m_workerThread.StopThread();
}

void TTask::BeginProcessing()
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);

	m_bRareStateModified = true;
	m_bOftenStateModified = true;

	m_workerThread.StartThread(DelegateThreadProc, this, GetTaskPropValue<eTO_ThreadPriority>(m_tTaskDefinition.GetConfiguration()));
}

void TTask::ResumeProcessing()
{
	// the same as retry but less demanding
	if(GetTaskState() == eTaskState_Paused)
	{
		SetTaskState(eTaskState_Processing);
		BeginProcessing();
	}
}

bool TTask::RetryProcessing()
{
	// retry used to auto-resume, after loading
	if(GetTaskState() != eTaskState_Paused && GetTaskState() != eTaskState_Finished && GetTaskState() != eTaskState_Cancelled)
	{
		BeginProcessing();
		return true;
	}
	return false;
}

void TTask::RestartProcessing()
{
	KillThread();

	SetTaskState(eTaskState_None);

	m_tSubTasksArray.ResetProgressAndStats();

	BeginProcessing();
}

void TTask::PauseProcessing()
{
	if(GetTaskState() != eTaskState_Finished && GetTaskState() != eTaskState_Cancelled)
	{
		KillThread();
		SetTaskState(eTaskState_Paused);

		m_bOftenStateModified = true;
	}
}

void TTask::CancelProcessing()
{
	// change to ST_CANCELLED
	if(GetTaskState() != eTaskState_Finished)
	{
		KillThread();
		SetTaskState(eTaskState_Cancelled);
		m_bOftenStateModified = true;
	}
}

void TTask::GetMiniSnapshot(TASK_MINI_DISPLAY_DATA *pData)
{
	TTaskStatsSnapshot tStats;
	m_tSubTasksArray.GetTaskStats(tStats);

	pData->m_strPath = tStats.GetCurrentSubTaskStats().GetCurrentPath();
	pData->m_dPercent = tStats.GetTaskProgressInPercent();

	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	pData->m_eTaskState = m_eCurrentState;
}

void TTask::GetSnapshot(TASK_DISPLAY_DATA *pData)
{
	TTaskStatsSnapshot tStats;
	m_tSubTasksArray.GetTaskStats(tStats);

	pData->m_strFullFilePath = tStats.GetCurrentSubTaskStats().GetCurrentPath();
	pData->m_strFileName = chcore::PathFromString(pData->m_strFullFilePath).GetFileName().ToString();
	pData->m_stIndex = tStats.GetCurrentSubTaskStats().GetProcessedCount();
	pData->m_stSize = tStats.GetCurrentSubTaskStats().GetTotalCount();
	pData->m_ullProcessedSize = tStats.GetCurrentSubTaskStats().GetProcessedSize();
	pData->m_ullSizeAll = tStats.GetCurrentSubTaskStats().GetTotalSize();
	pData->m_eSubOperationType = tStats.GetCurrentSubOperationType();
	pData->m_iCurrentBufferIndex = tStats.GetCurrentSubTaskStats().GetCurrentBufferIndex();
	pData->m_dPercent = tStats.GetTaskProgressInPercent();
	pData->m_timeElapsed = tStats.GetTimeElapsed();

	boost::shared_lock<boost::shared_mutex> lock(m_lock);

	pData->m_nPriority = GetTaskPropValue<eTO_ThreadPriority>(m_tTaskDefinition.GetConfiguration());
	pData->m_pathDstPath = m_tTaskDefinition.GetDestinationPath();
	pData->m_pafFilters = &m_afFilters;
	pData->m_eTaskState = m_eCurrentState;
	pData->m_strUniqueName = m_tTaskDefinition.GetTaskUniqueID();
	pData->m_eOperationType = m_tTaskDefinition.GetOperationType();

	pData->m_bIgnoreDirectories = GetTaskPropValue<eTO_IgnoreDirectories>(m_tTaskDefinition.GetConfiguration());
	pData->m_bCreateEmptyFiles = GetTaskPropValue<eTO_CreateEmptyFiles>(m_tTaskDefinition.GetConfiguration());

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
}

void TTask::DeleteProgress()
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

bool TTask::CanBegin()
{
	bool bRet=true;
	boost::unique_lock<boost::shared_mutex> lock(m_lock);

	if(GetContinueFlagNL() || GetForceFlagNL())
	{
		SetForceFlagNL(false);
		SetContinueFlagNL(false);
	}
	else
		bRet = false;

	return bRet;
}

void TTask::SetTaskDirectory(const TSmartPath& strDir)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_strTaskDirectory = strDir;
}

TSmartPath TTask::GetTaskDirectory() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_strTaskDirectory;
}

void TTask::SetTaskFilePath(const TSmartPath& strFilePath)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_strFilePath = strFilePath;
}

TSmartPath TTask::GetTaskFilePath() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_strFilePath;
}

void TTask::SetForceFlag(bool bFlag)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_bForce=bFlag;
}

bool TTask::GetForceFlag()
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_bForce;
}

void TTask::SetContinueFlag(bool bFlag)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_bContinue=bFlag;
}

bool TTask::GetContinueFlag()
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_bContinue;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TTask::SetForceFlagNL(bool bFlag)
{
	m_bForce=bFlag;
}

bool TTask::GetForceFlagNL()
{
	return m_bForce;
}

void TTask::SetContinueFlagNL(bool bFlag)
{
	m_bContinue=bFlag;
}

bool TTask::GetContinueFlagNL()
{
	return m_bContinue;
}

TSubTaskBase::ESubOperationResult TTask::CheckForWaitState()
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

DWORD WINAPI TTask::DelegateThreadProc(LPVOID pParam)
{
	BOOST_ASSERT(pParam);
	if(!pParam)
		return 1;

	TTask* pTask = (TTask*)pParam;
	return pTask->ThrdProc();
}

DWORD TTask::ThrdProc()
{
	// start tracking time for this thread
	TTaskProcessingGuard tProcessingGuard(m_tLocalStats);

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
		m_tTaskDefinition.GetConfiguration().ConnectToNotifier(TTask::OnCfgOptionChanged, this);

		// set thread options
		HANDLE hThread = GetCurrentThread();
		::SetThreadPriorityBoost(hThread, GetTaskPropValue<eTO_DisablePriorityBoost>(m_tTaskDefinition.GetConfiguration()));

		// determine when to scan directories
		bool bReadTasksSize = GetTaskPropValue<eTO_ScanDirectoriesBeforeBlocking>(m_tTaskDefinition.GetConfiguration());

		// prepare context for subtasks
		if(bReadTasksSize)
			eResult = m_tSubTasksArray.Execute(true);
		if(eResult == TSubTaskBase::eSubResult_Continue)
		{
			tProcessingGuard.PauseTimeTracking();
			eResult = CheckForWaitState();	// operation limiting
			tProcessingGuard.UnPauseTimeTracking();
		}
		if(eResult == TSubTaskBase::eSubResult_Continue)
			eResult = m_tSubTasksArray.Execute(false);

		// change status to finished
		if(eResult == TSubTaskBase::eSubResult_Continue)
			SetTaskState(eTaskState_Finished);

		// stop tracking time because of a possible blocking feedback dialogs
		tProcessingGuard.PauseTimeTracking();

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

		// if the files cache is not completely read - clean it up
		if(!m_files.IsComplete())
			m_files.Clear();		// get rid of m_files contents; rare state not modified, since incomplete cache is not being stored

		// save progress before killed
		m_bOftenStateModified = true;
		Store();

		// reset flags
		SetContinueFlag(false);
		SetForceFlag(false);

		m_tTaskDefinition.GetConfiguration().DisconnectFromNotifier(TTaskConfigTracker::NotificationProc);
		m_tTaskDefinition.GetConfiguration().DisconnectFromNotifier(TTask::OnCfgOptionChanged);

		// and the real end
		OnEndOperation();

		return 0;
	}
	catch(...)
	{
	}

	m_tTaskDefinition.GetConfiguration().DisconnectFromNotifier(TTaskConfigTracker::NotificationProc);
	m_tTaskDefinition.GetConfiguration().DisconnectFromNotifier(TTask::OnCfgOptionChanged);

	// log
	m_log.loge(_T("Caught exception in ThrdProc"));

	// stop tracking time because of a possible blocking feedback dialogs
	tProcessingGuard.PauseTimeTracking();

	// let others know some error happened
	m_piFeedbackHandler->RequestFeedback(IFeedbackHandler::eFT_OperationError, NULL);
	SetTaskState(eTaskState_Error);

	SetContinueFlag(false);
	SetForceFlag(false);

	OnEndOperation();

	return 1;
}

void TTask::OnBeginOperation()
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

void TTask::OnEndOperation()
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

void TTask::RequestStopThread()
{
	m_workerThread.SignalThreadToStop();
}

TSmartPath TTask::GetRelatedPath(EPathType ePathType)
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);

	return GetRelatedPathNL(ePathType);
}

void TTask::GetTaskStats(TTaskStatsSnapshot& rSnapshot) const
{
	m_tSubTasksArray.GetTaskStats(rSnapshot);
}

TSmartPath TTask::GetRelatedPathNL(EPathType ePathType)
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

void TTask::OnCfgOptionChanged(const TStringSet& rsetChanges, void* pParam)
{
	TTask* pTask = (TTask*)pParam;
	if(!pTask)
		THROW_CORE_EXCEPTION(eErr_InvalidArgument);

	if(rsetChanges.HasValue(TaskPropData<eTO_ThreadPriority>::GetPropertyName()))
	{
		pTask->m_workerThread.ChangePriority(GetTaskPropValue<eTO_ThreadPriority>(pTask->GetTaskDefinition().GetConfiguration()));
	}
}

END_CHCORE_NAMESPACE
