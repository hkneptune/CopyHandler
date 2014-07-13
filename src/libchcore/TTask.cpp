/***************************************************************************
*   Copyright (C) 2001-2014 by Jozef Starosczyk                           *
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

#include <fstream>
#include "TSubTaskScanDirectory.h"
#include "TSubTaskCopyMove.h"
#include "TSubTaskDelete.h"
#include <boost/lexical_cast.hpp>
#include "../libicpf/exception.h"
#include <atlconv.h>
#include "DataBuffer.h"
#include "TFileInfo.h"
#include "TSubTaskArray.h"
#include "TTaskStatsSnapshot.h"
#include "TCoreException.h"
#include "ErrorCodes.h"
#include "ISerializerRowData.h"
#include "TStringSet.h"
#include "SerializerTrace.h"

BEGIN_CHCORE_NAMESPACE

////////////////////////////////////////////////////////////////////////////
// TTask members

TTask::TTask(const ISerializerPtr& spSerializer, const IFeedbackHandlerPtr& spFeedbackHandler) :
	m_log(),
	m_spFeedbackHandler(spFeedbackHandler),
	m_spSrcPaths(new TBasePathDataContainer),
	m_files(),
	m_bForce(false),
	m_bContinue(false),
	m_tSubTaskContext(m_tConfiguration, m_spSrcPaths, m_afFilters, m_files,
		m_cfgTracker, m_log, spFeedbackHandler, m_workerThread, m_fsLocal),
	m_tSubTasksArray(m_tSubTaskContext),
	m_spSerializer(spSerializer)
{
	if(!spFeedbackHandler || !spSerializer)
		THROW_CORE_EXCEPTION(eErr_InvalidPointer);
}

TTask::~TTask()
{
	KillThread();
}

void TTask::SetTaskDefinition(const TTaskDefinition& rTaskDefinition)
{
	m_tBaseData.SetDestinationPath(rTaskDefinition.GetDestinationPath());
	m_tConfiguration = rTaskDefinition.GetConfiguration();
	*m_spSrcPaths = rTaskDefinition.GetSourcePaths();
	m_afFilters = rTaskDefinition.GetFilters();
	m_tBaseData.SetTaskName(rTaskDefinition.GetTaskName());

	m_tSubTasksArray.Init(rTaskDefinition.GetOperationPlan());
	m_files.Clear();
	m_tSubTaskContext.SetOperationType(m_tSubTasksArray.GetOperationType());
	m_tSubTaskContext.SetDestinationPath(m_tBaseData.GetDestinationPath());
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
	m_tBaseData.SetCurrentState(eTaskState);
}

ETaskCurrentState TTask::GetTaskState() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_tBaseData.GetCurrentState();
}

void TTask::SetBufferSizes(const TBufferSizes& bsSizes)
{
	m_tConfiguration.DelayNotifications();
	SetTaskPropValue<eTO_DefaultBufferSize>(m_tConfiguration, bsSizes.GetDefaultSize());
	SetTaskPropValue<eTO_OneDiskBufferSize>(m_tConfiguration, bsSizes.GetOneDiskSize());
	SetTaskPropValue<eTO_TwoDisksBufferSize>(m_tConfiguration, bsSizes.GetTwoDisksSize());
	SetTaskPropValue<eTO_CDBufferSize>(m_tConfiguration, bsSizes.GetCDSize());
	SetTaskPropValue<eTO_LANBufferSize>(m_tConfiguration, bsSizes.GetLANSize());
	SetTaskPropValue<eTO_UseOnlyDefaultBuffer>(m_tConfiguration, bsSizes.IsOnlyDefault());
	m_tConfiguration.ResumeNotifications();
}

void TTask::GetBufferSizes(TBufferSizes& bsSizes)
{
	bsSizes.SetDefaultSize(GetTaskPropValue<eTO_DefaultBufferSize>(m_tConfiguration));
	bsSizes.SetOneDiskSize(GetTaskPropValue<eTO_OneDiskBufferSize>(m_tConfiguration));
	bsSizes.SetTwoDisksSize(GetTaskPropValue<eTO_TwoDisksBufferSize>(m_tConfiguration));
	bsSizes.SetCDSize(GetTaskPropValue<eTO_CDBufferSize>(m_tConfiguration));
	bsSizes.SetLANSize(GetTaskPropValue<eTO_LANBufferSize>(m_tConfiguration));
	bsSizes.SetOnlyDefault(GetTaskPropValue<eTO_UseOnlyDefaultBuffer>(m_tConfiguration));
}

// thread
void TTask::SetPriority(int nPriority)
{
	SetTaskPropValue<eTO_ThreadPriority>(m_tConfiguration, nPriority);
}

void TTask::Load()
{
	using namespace chcore;

	bool bLogPathLoaded = false;
	bool bLoadFailed = false;
	const size_t stMaxSize = 1024;
	wchar_t szErr[stMaxSize];

	try
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);

		ISerializerContainerPtr spContainer = m_spSerializer->GetContainer(_T("task"));
		m_tBaseData.Load(spContainer);

		bLogPathLoaded = true;

		spContainer = m_spSerializer->GetContainer(_T("base_paths"));
		m_spSrcPaths->Load(spContainer);

		spContainer = m_spSerializer->GetContainer(_T("scanned_files"));
		m_files.Load(spContainer, m_spSrcPaths);

		spContainer = m_spSerializer->GetContainer(_T("task_config"));
		m_tConfiguration.Load(spContainer);

		spContainer = m_spSerializer->GetContainer(_T("filters"));
		m_afFilters.Load(spContainer);

		spContainer = m_spSerializer->GetContainer(_T("local_stats"));
		m_tLocalStats.Load(spContainer);

		m_tSubTasksArray.Load(m_spSerializer);

		// ensure copy-based context entries are properly updated after loading
		m_tSubTaskContext.SetDestinationPath(m_tBaseData.GetDestinationPath());
		m_tSubTaskContext.SetOperationType(m_tSubTasksArray.GetOperationType());
	}
	catch(const chcore::TBaseException& e)
	{
		SetTaskState(eTaskState_LoadError);
		bLoadFailed = true;

		_tcscpy_s(szErr, stMaxSize, _T("Task load error: "));
		size_t stLen = _tcslen(szErr);

		e.GetDetailedErrorInfo(szErr + stLen, stMaxSize - stLen);
	}
	catch(const std::exception& e)
	{
		SetTaskState(eTaskState_LoadError);
		bLoadFailed = true;
		_snwprintf_s(szErr, stMaxSize, _TRUNCATE, _T("Task load error. %hs"), e.what());
	}

	if(bLoadFailed)
	{
		try
		{
			if(bLogPathLoaded)
			{
				m_log.init(m_tBaseData.GetLogPath().ToString(), 262144, icpf::log_file::level_debug, false, false);
				m_log.loge(szErr);
			}
		}
		catch(const std::exception&)
		{
		}
	}
}

void TTask::Store()
{
	if(GetTaskState() == eTaskState_LoadError)
	{
		DBTRACE0(_T("Task::Store() - not storing task as it was not loaded correctly\n"));
		return;
	}

	TSimpleTimer timer(true);
	DBTRACE0(_T("###### Task::Store() - starting\n"));

	using namespace chcore;

	{
		boost::shared_lock<boost::shared_mutex> lock(m_lock);

		ISerializerContainerPtr spContainer = m_spSerializer->GetContainer(_T("task"));
		m_tBaseData.Store(spContainer);

		// base paths
		spContainer = m_spSerializer->GetContainer(_T("base_paths"));
		m_spSrcPaths->Store(spContainer);

		spContainer = m_spSerializer->GetContainer(_T("scanned_files"));
		m_files.Store(spContainer);

		spContainer = m_spSerializer->GetContainer(_T("task_config"));
		m_tConfiguration.Store(spContainer);

		spContainer = m_spSerializer->GetContainer(_T("filters"));
		m_afFilters.Store(spContainer);

		spContainer = m_spSerializer->GetContainer(_T("local_stats"));
		m_tLocalStats.Store(spContainer);

		m_tSubTasksArray.Store(m_spSerializer);
	}

	unsigned long long ullGatherTime = timer.Checkpoint(); ullGatherTime;

	m_spSerializer->Flush();

	unsigned long long ullFlushTime = timer.Stop(); ullFlushTime;
	DBTRACE2(_T("###### Task::Store() - finished - gather: %I64u ms, flush: %I64u ms\n"), ullGatherTime, ullFlushTime);
}

void TTask::KillThread()
{
	m_workerThread.StopThread();
}

void TTask::BeginProcessing()
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	if(m_tBaseData.GetCurrentState() != eTaskState_LoadError)
		m_workerThread.StartThread(DelegateThreadProc, this, GetTaskPropValue<eTO_ThreadPriority>(m_tConfiguration));
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
	switch(GetTaskState())
	{
	case eTaskState_Paused:
	case eTaskState_Finished:
	case eTaskState_Cancelled:
	case eTaskState_LoadError:
		return false;

	default:
		BeginProcessing();
		return true;
	}
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
	}
}

void TTask::CancelProcessing()
{
	// change to ST_CANCELLED
	if(GetTaskState() != eTaskState_Finished)
	{
		KillThread();
		SetTaskState(eTaskState_Cancelled);
	}
}

void TTask::GetStatsSnapshot(TTaskStatsSnapshotPtr& spSnapshot)
{
	if(!spSnapshot)
		THROW_CORE_EXCEPTION(eErr_InvalidArgument);

	spSnapshot->Clear();

	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	m_tSubTasksArray.GetStatsSnapshot(spSnapshot->GetSubTasksStats());

	m_tLocalStats.GetSnapshot(spSnapshot);

	spSnapshot->SetTaskName(m_tBaseData.GetTaskName());
	spSnapshot->SetThreadPriority(GetTaskPropValue<eTO_ThreadPriority>(m_tConfiguration));
	spSnapshot->SetDestinationPath(m_tBaseData.GetDestinationPath().ToString());
	spSnapshot->SetFilters(m_afFilters);
	spSnapshot->SetTaskState(m_tBaseData.GetCurrentState());
	spSnapshot->SetOperationType(m_tSubTasksArray.GetOperationType());

	spSnapshot->SetIgnoreDirectories(GetTaskPropValue<eTO_IgnoreDirectories>(m_tConfiguration));
	spSnapshot->SetCreateEmptyFiles(GetTaskPropValue<eTO_CreateEmptyFiles>(m_tConfiguration));

	TSubTaskStatsSnapshotPtr spCurrentSubTask = spSnapshot->GetSubTasksStats().GetCurrentSubTaskSnapshot();

	int iCurrentBufferIndex = spCurrentSubTask ? spCurrentSubTask->GetCurrentBufferIndex() : TBufferSizes::eBuffer_Default;
	switch(iCurrentBufferIndex)
	{
	case TBufferSizes::eBuffer_Default:
		spSnapshot->SetCurrentBufferSize(GetTaskPropValue<eTO_DefaultBufferSize>(m_tConfiguration));
		break;
	case TBufferSizes::eBuffer_OneDisk:
		spSnapshot->SetCurrentBufferSize(GetTaskPropValue<eTO_OneDiskBufferSize>(m_tConfiguration));
		break;
	case TBufferSizes::eBuffer_TwoDisks:
		spSnapshot->SetCurrentBufferSize(GetTaskPropValue<eTO_TwoDisksBufferSize>(m_tConfiguration));
		break;
	case TBufferSizes::eBuffer_CD:
		spSnapshot->SetCurrentBufferSize(GetTaskPropValue<eTO_CDBufferSize>(m_tConfiguration));
		break;
	case TBufferSizes::eBuffer_LAN:
		spSnapshot->SetCurrentBufferSize(GetTaskPropValue<eTO_LANBufferSize>(m_tConfiguration));
		break;
	default:
		THROW_CORE_EXCEPTION(eErr_UnhandledCase);
		//BOOST_ASSERT(false);		// assertions are dangerous here, because we're inside critical section
		// (and there could be conflict with Get(Mini)Snapshot called OnTimer in several places.
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
		m_log.init(m_tBaseData.GetLogPath().ToString(), 262144, icpf::log_file::level_debug, false, false);

		// start operation
		OnBeginOperation();

		// enable configuration changes tracking
		m_tConfiguration.ConnectToNotifier(TTaskConfigTracker::NotificationProc, &m_cfgTracker);
		m_tConfiguration.ConnectToNotifier(TTask::OnCfgOptionChanged, this);

		// set thread options
		HANDLE hThread = GetCurrentThread();
		::SetThreadPriorityBoost(hThread, GetTaskPropValue<eTO_DisablePriorityBoost>(m_tConfiguration));

		// determine when to scan directories
		bool bReadTasksSize = GetTaskPropValue<eTO_ScanDirectoriesBeforeBlocking>(m_tConfiguration);

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
			m_spFeedbackHandler->RequestFeedback(IFeedbackHandler::eFT_OperationError, NULL);
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
			m_spFeedbackHandler->RequestFeedback(IFeedbackHandler::eFT_OperationFinished, NULL);
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
		Store();

		// reset flags
		SetContinueFlag(false);
		SetForceFlag(false);

		m_tConfiguration.DisconnectFromNotifier(TTaskConfigTracker::NotificationProc);
		m_tConfiguration.DisconnectFromNotifier(TTask::OnCfgOptionChanged);

		// and the real end
		OnEndOperation();

		return 0;
	}
	catch(...)
	{
	}

	m_tConfiguration.DisconnectFromNotifier(TTaskConfigTracker::NotificationProc);
	m_tConfiguration.DisconnectFromNotifier(TTask::OnCfgOptionChanged);

	// log
	m_log.loge(_T("Caught exception in ThrdProc"));

	// stop tracking time because of a possible blocking feedback dialogs
	tProcessingGuard.PauseTimeTracking();

	// let others know some error happened
	m_spFeedbackHandler->RequestFeedback(IFeedbackHandler::eFT_OperationError, NULL);
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
	m_log.logi(strFormat.c_str());
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
	m_log.logi(strFormat.c_str());
}

void TTask::RequestStopThread()
{
	m_workerThread.SignalThreadToStop();
}

void TTask::OnCfgOptionChanged(const TStringSet& rsetChanges, void* pParam)
{
	TTask* pTask = (TTask*)pParam;
	if(!pTask)
		THROW_CORE_EXCEPTION(eErr_InvalidArgument);

	if(rsetChanges.HasValue(TaskPropData<eTO_ThreadPriority>::GetPropertyName()))
	{
		pTask->m_workerThread.ChangePriority(GetTaskPropValue<eTO_ThreadPriority>(pTask->m_tConfiguration));
	}
}

bool TTask::IsRunning() const
{
	return m_tLocalStats.IsRunning();
}

chcore::TSmartPath TTask::GetLogPath() const
{
	return m_tBaseData.GetLogPath();
}

void TTask::SetLogPath(const TSmartPath& pathLog)
{
	m_tBaseData.SetLogPath(pathLog);
}

chcore::ISerializerPtr TTask::GetSerializer() const
{
	return m_spSerializer;
}

END_CHCORE_NAMESPACE
