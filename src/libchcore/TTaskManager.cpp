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
#include "stdafx.h"
#include "TTaskManager.h"
#include "TTask.h"

#include <boost/smart_ptr/shared_array.hpp>
#include "../libicpf/exception.h"
#include "TLogger.h"
#include <boost/numeric/conversion/cast.hpp>
#include "TTaskStatsSnapshot.h"
#include "TTaskManagerStatsSnapshot.h"
#include "TCoreException.h"
#include "ErrorCodes.h"
#include "TTaskInfo.h"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/lexical_cast.hpp>

BEGIN_CHCORE_NAMESPACE

////////////////////////////////////////////////////////////////////////////////
// TTaskManager members
TTaskManager::TTaskManager(const ITaskManagerSerializerPtr& spSerializer) :
	m_piFeedbackFactory(NULL),
	m_stNextTaskID(NoTaskID + 1),
	m_spSerializer(spSerializer)
{
}

TTaskManager::~TTaskManager()
{
	// NOTE: do not delete the feedback factory, since we are not responsible for releasing it
}

void TTaskManager::Create(IFeedbackHandlerFactory* piFeedbackHandlerFactory)
{
	BOOST_ASSERT(piFeedbackHandlerFactory);

	m_piFeedbackFactory = piFeedbackHandlerFactory;
}

TTaskPtr TTaskManager::CreateTask(const TTaskDefinition& tTaskDefinition)
{
	TString strUuid = GetUuid();
	TSmartPath pathTaskSerializer = CreateTaskSerializePath(strUuid);
	TSmartPath pathTaskLog = CreateTaskLogPath(strUuid);

	IFeedbackHandler* piHandler = CreateNewFeedbackHandler();
	ITaskSerializerPtr spSerializer = m_spSerializer->CreateTaskSerializer(pathTaskSerializer);

	TTaskPtr spTask(new TTask(spSerializer, piHandler));
	spTask->SetLogPath(pathTaskLog);
	spTask->SetTaskDefinition(tTaskDefinition);

	Add(spTask);

	spTask->Store();

	return spTask;
}

TTaskPtr TTaskManager::ImportTask(const TSmartPath& strTaskPath)
{
	// load task definition from the new location
	TTaskDefinition tTaskDefinition;
	tTaskDefinition.Load(strTaskPath);

	return CreateTask(tTaskDefinition);
}

size_t TTaskManager::GetSize() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_tTasks.GetCount();
}

TTaskPtr TTaskManager::GetAt(size_t nIndex) const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	const TTaskInfoEntry& rInfo = m_tTasks.GetAt(nIndex);
	return rInfo.GetTask();
}

TTaskPtr TTaskManager::GetTaskByTaskID(taskid_t tTaskID) const
{
	if(tTaskID == NoTaskID)
		return TTaskPtr();

	TTaskInfoEntry tEntry;

	boost::shared_lock<boost::shared_mutex> lock(m_lock);

	if(!m_tTasks.GetByTaskID(tTaskID, tEntry))
		return TTaskPtr();

	return tEntry.GetTask();
}

void TTaskManager::Add(const TTaskPtr& spNewTask)
{
	if(!spNewTask)
		THROW_CORE_EXCEPTION(eErr_InvalidArgument);

	boost::unique_lock<boost::shared_mutex> lock(m_lock);

	int iOrder = 1;
	if(!m_tTasks.IsEmpty())
	{
		const TTaskInfoEntry& rEntry = m_tTasks.GetAt(m_tTasks.GetCount() - 1);
		iOrder = rEntry.GetOrder() + 1;
	}

	m_tTasks.Add(m_stNextTaskID++, spNewTask->GetSerializerPath(), iOrder, spNewTask);

	spNewTask->OnRegisterTask();
}

void TTaskManager::ClearBeforeExit()
{
	StopAllTasks();

	// ensure everything is stored so that we can resume processing in the future
	Store();

	// now remove all tasks without serializing anymore (prevent accidental
	// serialization)
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);
		m_tTasks.Clear();
		m_tTasks.ClearModifications();
	}
}

void TTaskManager::RemoveAllFinished()
{
	std::vector<TSmartPath> vTasksToRemove;

	// separate scope for locking
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);

		size_t stIndex = m_tTasks.GetCount();
		while(stIndex--)
		{
			TTaskInfoEntry& rEntry = m_tTasks.GetAt(stIndex);
			TTaskPtr spTask = rEntry.GetTask();

			// delete only when the thread is finished
			if((spTask->GetTaskState() == eTaskState_Finished || spTask->GetTaskState() == eTaskState_Cancelled))
			{
				spTask->KillThread();

				spTask->OnUnregisterTask();

				vTasksToRemove.push_back(rEntry.GetTaskPath());
				m_tTasks.RemoveAt(stIndex);
			}
		}
	}

	BOOST_FOREACH(TSmartPath& spTaskPath, vTasksToRemove)
	{
		// delete associated files
		DeleteFile(spTaskPath.ToString());
	}
}

void TTaskManager::RemoveFinished(const TTaskPtr& spSelTask)
{
	std::vector<TSmartPath> vTasksToRemove;

	// separate scope for locking
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);

		size_t stIndex = m_tTasks.GetCount();
		while(stIndex--)
		{
			TTaskInfoEntry& rEntry = m_tTasks.GetAt(stIndex);
			TTaskPtr spTask = rEntry.GetTask();

			// delete only when the thread is finished
			if(spTask == spSelTask && (spTask->GetTaskState() == eTaskState_Finished || spTask->GetTaskState() == eTaskState_Cancelled))
			{
				spTask->KillThread();

				spTask->OnUnregisterTask();

				vTasksToRemove.push_back(rEntry.GetTaskPath());
				m_tTasks.RemoveAt(stIndex);
				break;
			}
		}
	}

	BOOST_FOREACH(TSmartPath& spTaskPath, vTasksToRemove)
	{
		// delete associated files
		DeleteFile(spTaskPath.ToString());
	}
}

void TTaskManager::StopAllTasks()
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);

	StopAllTasksNL();
}

void TTaskManager::ResumeWaitingTasks(size_t stMaxRunningTasks)
{
	size_t stRunningCount = GetCountOfRunningTasks();

	boost::shared_lock<boost::shared_mutex> lock(m_lock);

	size_t stTasksToRun = stMaxRunningTasks == 0 ? std::numeric_limits<size_t>::max() : stMaxRunningTasks;
	stTasksToRun -= stRunningCount;

	if(stTasksToRun > 0)
	{
		for(size_t stIndex = 0; stIndex < m_tTasks.GetCount(); ++stIndex)
		{
			TTaskInfoEntry& rEntry = m_tTasks.GetAt(stIndex);
			TTaskPtr spTask = rEntry.GetTask();

			// turn on some thread - find something with wait state
			if(spTask->GetTaskState() == eTaskState_Waiting)
			{
				spTask->SetContinueFlagNL(true);
				if(--stTasksToRun == 0)
					break;
			}
		}
	}
}

void TTaskManager::TasksBeginProcessing()
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	for(size_t stIndex = 0; stIndex < m_tTasks.GetCount(); ++stIndex)
	{
		TTaskInfoEntry& rEntry = m_tTasks.GetAt(stIndex);
		TTaskPtr spTask = rEntry.GetTask();
		spTask->BeginProcessing();
	}
}

void TTaskManager::TasksPauseProcessing()
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	for(size_t stIndex = 0; stIndex < m_tTasks.GetCount(); ++stIndex)
	{
		TTaskInfoEntry& rEntry = m_tTasks.GetAt(stIndex);
		TTaskPtr spTask = rEntry.GetTask();
		spTask->PauseProcessing();
	}
}

void TTaskManager::TasksResumeProcessing()
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	for(size_t stIndex = 0; stIndex < m_tTasks.GetCount(); ++stIndex)
	{
		TTaskInfoEntry& rEntry = m_tTasks.GetAt(stIndex);
		TTaskPtr spTask = rEntry.GetTask();
		spTask->ResumeProcessing();
	}
}

void TTaskManager::TasksRestartProcessing()
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	for(size_t stIndex = 0; stIndex < m_tTasks.GetCount(); ++stIndex)
	{
		TTaskInfoEntry& rEntry = m_tTasks.GetAt(stIndex);
		TTaskPtr spTask = rEntry.GetTask();
		spTask->RestartProcessing();
	}
}

bool TTaskManager::TasksRetryProcessing()
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	bool bChanged=false;
	for(size_t stIndex = 0; stIndex < m_tTasks.GetCount(); ++stIndex)
	{
		TTaskInfoEntry& rEntry = m_tTasks.GetAt(stIndex);
		TTaskPtr spTask = rEntry.GetTask();
		if(spTask->RetryProcessing())
			bChanged = true;
	}

	return bChanged;
}

void TTaskManager::TasksCancelProcessing()
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	for(size_t stIndex = 0; stIndex < m_tTasks.GetCount(); ++stIndex)
	{
		TTaskInfoEntry& rEntry = m_tTasks.GetAt(stIndex);
		TTaskPtr spTask = rEntry.GetTask();
		spTask->CancelProcessing();
	}
}

bool TTaskManager::AreAllFinished()
{
	bool bFlag=true;

	if(GetCountOfRunningTasks() != 0)
		bFlag = false;
	else
	{
		boost::shared_lock<boost::shared_mutex> lock(m_lock);
		for(size_t stIndex = 0; stIndex < m_tTasks.GetCount(); ++stIndex)
		{
			TTaskInfoEntry& rEntry = m_tTasks.GetAt(stIndex);
			TTaskPtr spTask = rEntry.GetTask();

			ETaskCurrentState eState = spTask->GetTaskState();
			bFlag = (eState == eTaskState_Finished || eState == eTaskState_Cancelled || eState == eTaskState_Paused || eState == eTaskState_Error);

			if(!bFlag)
				break;
		}
	}

	return bFlag;
}

void TTaskManager::GetStatsSnapshot(TTaskManagerStatsSnapshotPtr& spSnapshot) const
{
	if(!spSnapshot)
		THROW_CORE_EXCEPTION(eErr_InvalidArgument);

	spSnapshot->Clear();

	boost::shared_lock<boost::shared_mutex> lock(m_lock);

	size_t stRunningTasks = 0;
	for(size_t stIndex = 0; stIndex < m_tTasks.GetCount(); ++stIndex)
	{
		const TTaskInfoEntry& rEntry = m_tTasks.GetAt(stIndex);
		TTaskPtr spTask = rEntry.GetTask();
		TTaskStatsSnapshotPtr spStats(new TTaskStatsSnapshot);
		spTask->GetStatsSnapshot(spStats);
		spStats->SetTaskID(rEntry.GetTaskID());

		if(spStats->IsTaskRunning() && spStats->GetTaskState())
			++stRunningTasks;

		spSnapshot->AddTaskStats(spStats);
	}

	spSnapshot->SetRunningTasks(stRunningTasks);
}

size_t TTaskManager::GetCountOfRunningTasks() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);

	TTaskStatsSnapshot tTaskStats;

	size_t stRunningTasks = 0;

	for(size_t stIndex = 0; stIndex < m_tTasks.GetCount(); ++stIndex)
	{
		const TTaskInfoEntry& rEntry = m_tTasks.GetAt(stIndex);
		TTaskPtr spTask = rEntry.GetTask();
		if(spTask->IsRunning() && spTask->GetTaskState() == eTaskState_Processing)
			++stRunningTasks;
	}

	return stRunningTasks;
}

void TTaskManager::StopAllTasksNL()
{
	// kill all unfinished tasks - send kill request
	for(size_t stIndex = 0; stIndex < m_tTasks.GetCount(); ++stIndex)
	{
		TTaskInfoEntry& rEntry = m_tTasks.GetAt(stIndex);
		TTaskPtr spTask = rEntry.GetTask();
		spTask->RequestStopThread();
	}

	// wait for finishing
	for(size_t stIndex = 0; stIndex < m_tTasks.GetCount(); ++stIndex)
	{
		TTaskInfoEntry& rEntry = m_tTasks.GetAt(stIndex);
		TTaskPtr spTask = rEntry.GetTask();
		spTask->KillThread();
	}
}

IFeedbackHandler* TTaskManager::CreateNewFeedbackHandler()
{
	BOOST_ASSERT(m_piFeedbackFactory);
	if(!m_piFeedbackFactory)
		return NULL;

	IFeedbackHandler* piHandler = m_piFeedbackFactory->Create();

	return piHandler;
}

void TTaskManager::Store()
{
	// store this container information
	TTaskInfoContainer tDataDiff;
	{
		boost::shared_lock<boost::shared_mutex> lock(m_lock);
		m_tTasks.GetDiffAndResetModifications(tDataDiff);
	}

	try
	{
		m_spSerializer->Store(tDataDiff);
	}
	catch(const std::exception&)
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);
		m_tTasks.RestoreModifications(tDataDiff);

		throw;
	}

	// trigger storing tasks
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	for(size_t stIndex = 0; stIndex < m_tTasks.GetCount(); ++stIndex)
	{
		TTaskInfoEntry& rEntry = m_tTasks.GetAt(stIndex);
		TTaskPtr spTask = rEntry.GetTask();

		spTask->Store();
	}

}

void TTaskManager::Load()
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);

	if(!m_tTasks.IsEmpty())
		THROW_CORE_EXCEPTION(eErr_InternalProblem);

	m_spSerializer->Load(m_tTasks);

	// clear all modifications of freshly loaded tasks (in case serializer does
	// not reset the modification state)
	m_tTasks.ClearModifications();

	// load tasks
	for(size_t stIndex = 0; stIndex < m_tTasks.GetCount(); ++stIndex)
	{
		TTaskInfoEntry& rEntry = m_tTasks.GetAt(stIndex);

		if(!rEntry.GetTask())
		{
			IFeedbackHandler* piHandler = CreateNewFeedbackHandler();
			ITaskSerializerPtr spSerializer = m_spSerializer->CreateTaskSerializer(rEntry.GetTaskPath());

			TTaskPtr spTask(new TTask(spSerializer, piHandler));
			spTask->Load();

			rEntry.SetTask(spTask);
		}
	}
}

TString TTaskManager::GetUuid()
{
	boost::uuids::random_generator gen;
	boost::uuids::uuid u = gen();
	return boost::lexical_cast<std::wstring>(u).c_str();
}

TSmartPath TTaskManager::CreateTaskLogPath(const TString& strUuid) const
{
	TSmartPath pathLog = m_pathLogDir + PathFromString(TString(_T("Task-")) + strUuid + _T(".log"));
	return pathLog;
}

chcore::TSmartPath TTaskManager::CreateTaskSerializePath(const TString& strUuid) const
{
	TSmartPath pathLog = m_pathLogDir + PathFromString(TString(_T("Task-")) + strUuid + _T(".sqlite"));
	return pathLog;
}

END_CHCORE_NAMESPACE
