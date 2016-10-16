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
#include <atlconv.h>
#include "TFileInfo.h"
#include "TSubTaskArray.h"
#include "TTaskStatsSnapshot.h"
#include "TCoreException.h"
#include "ErrorCodes.h"
#include "ISerializerRowData.h"
#include "TStringSet.h"
#include "SerializerTrace.h"
#include "TScopedRunningTimeTracker.h"
#include "TScopedRunningTimeTrackerPause.h"
#include "TFeedbackHandlerWrapper.h"
#include "TTaskConfigBufferSizes.h"
#include <wchar.h>
#include "TLocalFilesystem.h"
#include "TTaskConfigVerifier.h"
#include "../liblogger/TAsyncMultiLogger.h"

namespace chcore
{
	////////////////////////////////////////////////////////////////////////////
	// TTask members

	TTask::TTask(const ISerializerPtr& spSerializer, const IFeedbackHandlerPtr& spFeedbackHandler,
		const TTaskDefinition& rTaskDefinition, const TSmartPath& rLogPath, const logger::TMultiLoggerConfigPtr& spLoggerConfig) :
		m_spLog(std::make_unique<logger::TLogger>(logger::TAsyncMultiLogger::GetInstance()->CreateLoggerData(rLogPath.ToString(), spLoggerConfig), L"Task")),
		m_spInternalFeedbackHandler(spFeedbackHandler),
		m_spSrcPaths(new TBasePathDataContainer),
		m_bForce(false),
		m_bContinue(false),
		m_tSubTaskContext(m_tConfiguration, m_spSrcPaths, m_afFilters,
			m_cfgTracker, m_spLog->GetLogFileData(), m_workerThread,
			std::make_shared<TLocalFilesystem>()),
		m_tSubTasksArray(m_tSubTaskContext),
		m_spSerializer(spSerializer)
	{
		if(!spFeedbackHandler)
			throw TCoreException(eErr_InvalidArgument, L"spFeedbackHandler", LOCATION);
		if(!spSerializer)
			throw TCoreException(eErr_InvalidArgument, L"spSerializer", LOCATION);

		m_tBaseData.SetLogPath(rLogPath);
		SetTaskDefinition(rTaskDefinition);
	}

	TTask::TTask(const ISerializerPtr& spSerializer, const IFeedbackHandlerPtr& spFeedbackHandler, const TTaskBaseData& rBaseTaskData, const logger::TMultiLoggerConfigPtr& spLoggerConfig) :
		m_spLog(std::make_unique<logger::TLogger>(logger::TAsyncMultiLogger::GetInstance()->CreateLoggerData(rBaseTaskData.GetLogPath().ToString(), spLoggerConfig), L"Task")),
		m_spInternalFeedbackHandler(spFeedbackHandler),
		m_spSrcPaths(new TBasePathDataContainer),
		m_bForce(false),
		m_bContinue(false),
		m_tSubTaskContext(m_tConfiguration, m_spSrcPaths, m_afFilters,
			m_cfgTracker, m_spLog->GetLogFileData(), m_workerThread,
			std::make_shared<TLocalFilesystem>()),
		m_tSubTasksArray(m_tSubTaskContext),
		m_spSerializer(spSerializer)
	{
		if(!spFeedbackHandler)
			throw TCoreException(eErr_InvalidArgument, L"spFeedbackHandler", LOCATION);
		if(!spSerializer)
			throw TCoreException(eErr_InvalidArgument, L"spSerializer", LOCATION);
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

		SetTaskPropBufferSizes(m_tConfiguration, bsSizes);
		m_tConfiguration.ResumeNotifications();
	}

	void TTask::GetBufferSizes(TBufferSizes& bsSizes)
	{
		bsSizes = GetTaskPropBufferSizes(m_tConfiguration);
	}

	// thread
	void TTask::SetPriority(int nPriority)
	{
		SetTaskPropValue<eTO_ThreadPriority>(m_tConfiguration, nPriority);
	}

	void TTask::Load(const TTaskBaseData& rBaseData)
	{
		bool bLogPathLoaded = false;
		bool bLoadFailed = false;
		const size_t stMaxSize = 1024;
		wchar_t szErr[stMaxSize];

		try
		{
			boost::unique_lock<boost::shared_mutex> lock(m_lock);

			ISerializerContainerPtr spContainer = m_spSerializer->GetContainer(_T("task"));
			m_tBaseData = rBaseData;// .Load(spContainer);

			bLogPathLoaded = true;

			spContainer = m_spSerializer->GetContainer(_T("base_paths"));
			m_spSrcPaths->Load(spContainer);

			spContainer = m_spSerializer->GetContainer(_T("scanned_files"));
			m_tSubTaskContext.GetFilesCache().Load(spContainer, m_spSrcPaths);

			spContainer = m_spSerializer->GetContainer(_T("task_config"));
			m_tConfiguration.Load(spContainer);

			spContainer = m_spSerializer->GetContainer(_T("filters"));
			m_afFilters.Load(spContainer);

			spContainer = m_spSerializer->GetContainer(_T("local_stats"));
			m_tLocalStats.Load(spContainer);

			spContainer = m_spSerializer->GetContainer(_T("feedback"));
			m_spInternalFeedbackHandler->Load(spContainer);

			// ensure copy-based context entries are properly updated after loading
			m_tSubTaskContext.SetDestinationPath(m_tBaseData.GetDestinationPath());
			m_tSubTaskContext.SetOperationType(m_tSubTasksArray.GetOperationType());

			m_tSubTasksArray.Load(m_spSerializer);
		}
		catch (const TBaseException& e)
		{
			SetTaskState(eTaskState_LoadError);
			bLoadFailed = true;

			_tcscpy_s(szErr, stMaxSize, _T("Task load error: "));
			size_t stLen = _tcslen(szErr);

			e.GetDetailedErrorInfo(szErr + stLen, stMaxSize - stLen);
		}
		catch (const std::exception& e)
		{
			SetTaskState(eTaskState_LoadError);
			bLoadFailed = true;
			_snwprintf_s(szErr, stMaxSize, _TRUNCATE, _T("Task load error. %hs"), e.what());
		}

		if (bLoadFailed)
		{
			try
			{
				if (bLogPathLoaded)
				{
					LOG_ERROR(m_spLog) << szErr;
				}
			}
			catch (const std::exception&)
			{
			}
		}
	}

	TTaskPtr TTask::Load(const ISerializerPtr& spSerializer, const IFeedbackHandlerPtr& spFeedbackHandler, const logger::TMultiLoggerConfigPtr& spLoggerConfig)
	{
		TTaskBaseData tBaseData;
		ISerializerContainerPtr spContainer = spSerializer->GetContainer(_T("task"));
		tBaseData.Load(spContainer);

		TTaskPtr spTask = std::shared_ptr<TTask>(new TTask(spSerializer, spFeedbackHandler, tBaseData, spLoggerConfig));
		spTask->Load(tBaseData);

		return spTask;
	}

	void TTask::Store(bool bForce)
	{
		if (GetTaskState() == eTaskState_LoadError)
		{
			DBTRACE0(_T("Task::Store() - not storing task as it was not loaded correctly\n"));
			return;
		}

		DBTRACE0(_T("###### Task::Store() - starting\n"));

		// ensure we're only running one serialization of this task at a time;
		// (this is usually called from the gui thread (on timer) and at specific points
		// of task processing; there were times where 
		if(!bForce)
		{
			if (!m_mutexSerializer.try_lock())
			{
				DBTRACE0(_T("###### Task::Store() - serialization already running. Skipping.\n"));
				return;
			}
		}
		else
		{
			if (!m_mutexSerializer.try_lock())
			{
				DBTRACE0(_T("###### Task::Store() - waiting for serialization mutex...\n"));
				m_mutexSerializer.lock();
			}
		}

		std::unique_lock<std::mutex> locke(m_mutexSerializer, std::adopt_lock);

		TSimpleTimer timer(true);

		using namespace chcore;

		{
			boost::shared_lock<boost::shared_mutex> lock(m_lock);

			ISerializerContainerPtr spContainer = m_spSerializer->GetContainer(_T("task"));
			m_tBaseData.Store(spContainer);

			// base paths
			spContainer = m_spSerializer->GetContainer(_T("base_paths"));
			m_spSrcPaths->Store(spContainer);

			spContainer = m_spSerializer->GetContainer(_T("scanned_files"));
			m_tSubTaskContext.GetFilesCache().Store(spContainer);

			spContainer = m_spSerializer->GetContainer(_T("task_config"));
			m_tConfiguration.Store(spContainer);

			spContainer = m_spSerializer->GetContainer(_T("filters"));
			m_afFilters.Store(spContainer);

			spContainer = m_spSerializer->GetContainer(_T("local_stats"));
			m_tLocalStats.Store(spContainer);

			spContainer = m_spSerializer->GetContainer(_T("feedback"));
			m_spInternalFeedbackHandler->Store(spContainer);

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
		LOG_INFO(m_spLog) << _T("Requested task to begin processing");

		boost::unique_lock<boost::shared_mutex> lock(m_lock);
		if (m_tBaseData.GetCurrentState() != eTaskState_LoadError)
			m_workerThread.StartThread(DelegateThreadProc, this, GetTaskPropValue<eTO_ThreadPriority>(m_tConfiguration));
	}

	void TTask::ResumeProcessing()
	{
		// the same as retry but less demanding
		if (GetTaskState() == eTaskState_Paused)
		{
			LOG_INFO(m_spLog) << _T("Requested task resume");
			SetTaskState(eTaskState_Processing);
			BeginProcessing();
		}
	}

	bool TTask::RetryProcessing()
	{
		// retry used to auto-resume, after loading
		ETaskCurrentState eState = GetTaskState();
		switch (eState)
		{
		case eTaskState_Paused:
		case eTaskState_Finished:
		case eTaskState_Cancelled:
		case eTaskState_LoadError:
			return false;

		case eTaskState_Processing:
		case eTaskState_Waiting:
			{
				if(IsRunning())
					return false;
				//else go to default clause
			}

		default:
			BeginProcessing();
			return true;
		}
	}

	void TTask::RestartProcessing()
	{
		LOG_INFO(m_spLog) << _T("Requested task restart");
		KillThread();

		SetTaskState(eTaskState_None);

		m_spInternalFeedbackHandler->RestoreDefaults();
		m_tSubTasksArray.ResetProgressAndStats();
		m_tLocalStats.Clear();
		m_spSrcPaths->ResetProcessingFlags();
		m_tSubTaskContext.GetFilesCache().Clear();

		Store(true);

		BeginProcessing();
	}

	void TTask::RestoreFeedbackDefaults()
	{
		m_spInternalFeedbackHandler->RestoreDefaults();
	}

	void TTask::PauseProcessing()
	{
		if (GetTaskState() != eTaskState_Finished && GetTaskState() != eTaskState_Cancelled)
		{
			LOG_INFO(m_spLog) << _T("Requested task pause");
			KillThread();
			SetTaskState(eTaskState_Paused);
		}
	}

	void TTask::CancelProcessing()
	{
		// change to ST_CANCELLED
		if (GetTaskState() != eTaskState_Finished)
		{
			LOG_INFO(m_spLog) << _T("Requested task cancel");
			KillThread();
			SetTaskState(eTaskState_Cancelled);
		}
	}

	void TTask::GetStatsSnapshot(TTaskStatsSnapshotPtr& spSnapshot)
	{
		if (!spSnapshot)
			throw TCoreException(eErr_InvalidArgument, L"spSnapshot", LOCATION);

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
		spSnapshot->SetBufferCount(GetTaskPropValue<eTO_BufferQueueDepth>(m_tConfiguration));

		TSubTaskStatsSnapshotPtr spCurrentSubTask = spSnapshot->GetSubTasksStats().GetCurrentSubTaskSnapshot();
		if(spCurrentSubTask)
			spSnapshot->SetSourcePath(spCurrentSubTask->GetCurrentPath());
		else if(m_spSrcPaths->GetCount() > 0)
		{
			TBasePathDataPtr spBasePath = m_spSrcPaths->GetAt(0);
			if(spBasePath)
				spSnapshot->SetSourcePath(spBasePath->GetSrcPath().ToString());
		}

		int iCurrentBufferIndex = spCurrentSubTask ? spCurrentSubTask->GetCurrentBufferIndex() : TBufferSizes::eBuffer_Default;
		switch (iCurrentBufferIndex)
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
			throw TCoreException(eErr_UnhandledCase, L"Unknown buffer index", LOCATION);
			//BOOST_ASSERT(false);		// assertions are dangerous here, because we're inside critical section
			// (and there could be conflict with Get(Mini)Snapshot called OnTimer in several places.
		}
	}

	bool TTask::CanBegin()
	{
		bool bRet = true;
		boost::unique_lock<boost::shared_mutex> lock(m_lock);

		if (GetContinueFlagNL() || GetForceFlagNL())
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
		m_bForce = bFlag;
	}

	bool TTask::GetForceFlag()
	{
		boost::shared_lock<boost::shared_mutex> lock(m_lock);
		return m_bForce;
	}

	void TTask::SetContinueFlag(bool bFlag)
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);
		m_bContinue = bFlag;
	}

	bool TTask::GetContinueFlag()
	{
		boost::shared_lock<boost::shared_mutex> lock(m_lock);
		return m_bContinue;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TTask::SetForceFlagNL(bool bFlag)
	{
		m_bForce = bFlag;
	}

	bool TTask::GetForceFlagNL()
	{
		return m_bForce;
	}

	void TTask::SetContinueFlagNL(bool bFlag)
	{
		m_bContinue = bFlag;
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
		while (!bContinue)
		{
			if (CanBegin())
			{
				SetTaskState(eTaskState_Processing);
				bContinue = true;

				LOG_INFO(m_spLog) << _T("Finished waiting for begin permission");

				//			return; // skips sleep and kill flag checking
			}
			else
				Sleep(50);	// not to make it too hard for processor

			if (m_workerThread.KillRequested())
			{
				// log
				LOG_INFO(m_spLog) << _T("Kill request while waiting for begin permission (wait state)");
				return TSubTaskBase::eSubResult_KillRequest;
			}
		}

		return TSubTaskBase::eSubResult_Continue;
	}

	DWORD WINAPI TTask::DelegateThreadProc(LPVOID pParam)
	{
		BOOST_ASSERT(pParam);
		if (!pParam)
			return 1;

		TTask* pTask = (TTask*)pParam;
		return pTask->ThrdProc();
	}

	DWORD TTask::ThrdProc()
	{
		// start tracking time for this thread
		TScopedRunningTimeTracker tProcessingGuard(m_tLocalStats);
		TFeedbackHandlerWrapperPtr spFeedbackHandler(std::make_shared<TFeedbackHandlerWrapper>(m_spInternalFeedbackHandler, tProcessingGuard));

		const size_t ExceptionBufferSize = 2048;
		std::unique_ptr<wchar_t[]> upExceptionInfoBuffer(new wchar_t[ExceptionBufferSize]);
		try
		{
			// start operation
			OnBeginOperation();

			// enable configuration changes tracking
			m_tConfiguration.ConnectToNotifier(TTaskConfigTracker::NotificationProc, &m_cfgTracker);
			m_tConfiguration.ConnectToNotifier(TTask::OnCfgOptionChanged, this);

			// verify configuration is valid
			TTaskConfigVerifier::VerifyAndUpdate(m_tConfiguration, m_spLog);

			// set thread options
			HANDLE hThread = GetCurrentThread();
			::SetThreadPriorityBoost(hThread, GetTaskPropValue<eTO_DisablePriorityBoost>(m_tConfiguration));

			// initialize subtask array
			m_tSubTasksArray.InitBeforeExec();

			// exec the estimation subtasks
			TSubTaskBase::ESubOperationResult eResult = m_tSubTasksArray.Execute(spFeedbackHandler, true);

			// go into wait state only in case the preprocessing did not finish the operation already
			// (only fast move can do that right now)
			if (eResult == TSubTaskBase::eSubResult_Continue && !m_tSubTasksArray.AreAllBasePathsProcessed())
			{
				TScopedRunningTimeTrackerPause scopedPause(tProcessingGuard);

				eResult = CheckForWaitState();	// operation limiting
			}
			if (eResult == TSubTaskBase::eSubResult_Continue)
				eResult = m_tSubTasksArray.Execute(spFeedbackHandler, false);

			// change status to finished
			if (eResult == TSubTaskBase::eSubResult_Continue)
				SetTaskState(eTaskState_Finished);

			// finishing processing
			// change task status
			switch (eResult)
			{
			case TSubTaskBase::eSubResult_Error:
				spFeedbackHandler->OperationError();
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
				if (GetTaskState() == eTaskState_Waiting)
					SetTaskState(eTaskState_Processing);
				break;

			case TSubTaskBase::eSubResult_Continue:
				spFeedbackHandler->OperationFinished();
				SetTaskState(eTaskState_Finished);
				break;

			default:
				BOOST_ASSERT(false);
				throw TCoreException(eErr_UnhandledCase, L"Unknown feedback result", LOCATION);
			}

			// if the files cache is not completely read - clean it up
			if (!m_tSubTaskContext.GetFilesCache().IsComplete())
				m_tSubTaskContext.GetFilesCache().Clear();		// scanning for files did not finish processing, so the content of the files cache are useless

			// save progress before killed
			Store(true);

			// reset flags
			SetContinueFlag(false);
			SetForceFlag(false);

			m_tConfiguration.DisconnectFromNotifier(TTaskConfigTracker::NotificationProc);
			m_tConfiguration.DisconnectFromNotifier(TTask::OnCfgOptionChanged);

			// and the real end
			OnEndOperation();

			return 0;
		}
		catch(const TBaseException& e)
		{
			e.GetDetailedErrorInfo(upExceptionInfoBuffer.get(), ExceptionBufferSize);
		}
		catch(const std::exception& e)
		{
			swprintf_s(upExceptionInfoBuffer.get(), ExceptionBufferSize, L"%S", e.what());
		}
		catch (...)
		{
			swprintf_s(upExceptionInfoBuffer.get(), ExceptionBufferSize, L"Unspecified error occurred");
		}

		m_tConfiguration.DisconnectFromNotifier(TTaskConfigTracker::NotificationProc);
		m_tConfiguration.DisconnectFromNotifier(TTask::OnCfgOptionChanged);

		// log
		TString strMsg = TString(L"Caught exception in ThrdProc: ") + upExceptionInfoBuffer.get();
		LOG_ERROR(m_spLog) << strMsg.c_str();

		// let others know some error happened
		spFeedbackHandler->OperationError();
		SetTaskState(eTaskState_Error);

		SetContinueFlag(false);
		SetForceFlag(false);

		OnEndOperation();

		return 1;
	}

	void TTask::OnBeginOperation()
	{
		LOG_INFO(m_spLog) << _T("Processing thread started");
	}

	void TTask::OnEndOperation()
	{
		LOG_INFO(m_spLog) << _T("Finished processing data");
	}

	void TTask::RequestStopThread()
	{
		m_workerThread.SignalThreadToStop();
	}

	void TTask::OnCfgOptionChanged(const TStringSet& rsetChanges, void* pParam)
	{
		TTask* pTask = (TTask*)pParam;
		if (!pTask)
			throw TCoreException(eErr_InvalidArgument, L"pParam is null, task pointer not provided", LOCATION);

		if (rsetChanges.HasValue(TaskPropData<eTO_ThreadPriority>::GetPropertyName()))
		{
			pTask->m_workerThread.ChangePriority(GetTaskPropValue<eTO_ThreadPriority>(pTask->m_tConfiguration));
		}
	}

	bool TTask::IsRunning() const
	{
		return m_tLocalStats.IsRunning();
	}

	TSmartPath TTask::GetLogPath() const
	{
		return m_tBaseData.GetLogPath();
	}

	TString TTask::GetTaskName() const
	{
		return m_tBaseData.GetTaskName();
	}

	void TTask::SetLogPath(const TSmartPath& pathLog)
	{
		m_tBaseData.SetLogPath(pathLog);
	}

	ISerializerPtr TTask::GetSerializer() const
	{
		return m_spSerializer;
	}
}
