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

BEGIN_CHCORE_NAMESPACE

////////////////////////////////////////////////////////////////////////////////
// TTaskManager members
TTaskManager::TTaskManager() :
	m_piFeedbackFactory(NULL),
	m_stNextSessionUniqueID(NO_TASK_SESSION_UNIQUE_ID + 1)
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
	TTaskPtr spTask = CreateEmptyTask();
	if(spTask)
	{
		spTask->SetTaskDefinition(tTaskDefinition);
		Add(spTask);
		spTask->Store();
	}

	return spTask;
}

TTaskPtr TTaskManager::ImportTask(const TSmartPath& strTaskPath)
{
	// load task definition from the new location
	TTaskDefinition tTaskDefinition;
	tTaskDefinition.Load(strTaskPath);

	return CreateTask(tTaskDefinition);
}

TTaskPtr TTaskManager::CreateEmptyTask()
{
	BOOST_ASSERT(m_piFeedbackFactory);
	if(!m_piFeedbackFactory)
		return TTaskPtr();

	IFeedbackHandler* piHandler = m_piFeedbackFactory->Create();
	if(!piHandler)
		return TTaskPtr();

	BOOST_ASSERT(m_stNextSessionUniqueID != NO_TASK_SESSION_UNIQUE_ID);
	TTaskPtr spTask(new TTask(piHandler, m_stNextSessionUniqueID++));

	// NO_TASK_SESSION_UNIQUE_ID is a special value so it should not be used to identify tasks
	if(m_stNextSessionUniqueID == NO_TASK_SESSION_UNIQUE_ID)
		++m_stNextSessionUniqueID;

	return spTask;
}

size_t TTaskManager::GetSize() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_vTasks.size();
}

TTaskPtr TTaskManager::GetAt(size_t nIndex) const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);

	_ASSERTE(nIndex >= 0 && nIndex < m_vTasks.size());
	if(nIndex >= m_vTasks.size())
		THROW_CORE_EXCEPTION(eErr_InvalidArgument);

	return m_vTasks.at(nIndex);
}

TTaskPtr TTaskManager::GetTaskBySessionUniqueID(size_t stSessionUniqueID) const
{
	if(stSessionUniqueID == NO_TASK_SESSION_UNIQUE_ID)
		return TTaskPtr();

	TTaskPtr spFoundTask;

	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	BOOST_FOREACH(const TTaskPtr& spTask, m_vTasks)
	{
		if(spTask->GetSessionUniqueID() == stSessionUniqueID)
		{
			spFoundTask = spTask;
			break;
		}
	}

	return spFoundTask;
}

size_t TTaskManager::Add(const TTaskPtr& spNewTask)
{
	if(!spNewTask)
		THROW_CORE_EXCEPTION(eErr_InvalidArgument);

	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	// here we know load succeeded
	spNewTask->SetTaskDirectory(m_pathTasksDir);

	m_vTasks.push_back(spNewTask);

	spNewTask->OnRegisterTask();

	return m_vTasks.size() - 1;
}

void TTaskManager::RemoveAt(size_t stIndex, size_t stCount)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);

	_ASSERTE(stIndex >= m_vTasks.size() || stIndex + stCount > m_vTasks.size());
	if(stIndex >= m_vTasks.size() || stIndex + stCount > m_vTasks.size())
		THROW_CORE_EXCEPTION(eErr_InvalidArgument);

	for(std::vector<TTaskPtr>::iterator iterTask = m_vTasks.begin() + stIndex; iterTask != m_vTasks.begin() + stIndex + stCount; ++iterTask)
	{
		TTaskPtr& spTask = *iterTask;

		// kill task if needed
		spTask->KillThread();

		spTask->OnUnregisterTask();
	}

	// remove elements from array
	m_vTasks.erase(m_vTasks.begin() + stIndex, m_vTasks.begin() + stIndex + stCount);
}

void TTaskManager::RemoveAll()
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);

	StopAllTasksNL();

	m_vTasks.clear();
}

void TTaskManager::RemoveAllFinished()
{
	std::vector<TTaskPtr> vTasksToRemove;

	// separate scope for locking
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);

		size_t stIndex = m_vTasks.size();
		while(stIndex--)
		{
			TTaskPtr spTask = m_vTasks.at(stIndex);

			// delete only when the thread is finished
			if((spTask->GetTaskState() == eTaskState_Finished || spTask->GetTaskState() == eTaskState_Cancelled))
			{
				spTask->OnUnregisterTask();

				vTasksToRemove.push_back(spTask);
				m_vTasks.erase(m_vTasks.begin() + stIndex);
			}
		}
	}

	BOOST_FOREACH(TTaskPtr& spTask, vTasksToRemove)
	{
		// delete associated files
		spTask->DeleteProgress();
	}
}

void TTaskManager::RemoveFinished(const TTaskPtr& spSelTask)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);

	// this might be optimized by copying tasks to a local table in critical section, and then deleting progress files outside of the critical section
	for(std::vector<TTaskPtr>::iterator iterTask = m_vTasks.begin(); iterTask != m_vTasks.end(); ++iterTask)
	{
		TTaskPtr& spTask = *iterTask;

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
		BOOST_FOREACH(TTaskPtr& spTask, m_vTasks)
		{
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

void TTaskManager::SaveData()
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	BOOST_FOREACH(TTaskPtr& spTask, m_vTasks)
	{
		spTask->Store();
	}
}

void TTaskManager::LoadDataProgress()
{
	if(m_pathTasksDir.IsEmpty())
		THROW_CORE_EXCEPTION(eErr_MissingTaskSerializationPath);

	TTaskPtr spTask;
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

void TTaskManager::TasksBeginProcessing()
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	BOOST_FOREACH(TTaskPtr& spTask, m_vTasks)
	{
		spTask->BeginProcessing();
	}
}

void TTaskManager::TasksPauseProcessing()
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	BOOST_FOREACH(TTaskPtr& spTask, m_vTasks)
	{
		spTask->PauseProcessing();
	}
}

void TTaskManager::TasksResumeProcessing()
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	BOOST_FOREACH(TTaskPtr& spTask, m_vTasks)
	{
		spTask->ResumeProcessing();
	}
}

void TTaskManager::TasksRestartProcessing()
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	BOOST_FOREACH(TTaskPtr& spTask, m_vTasks)
	{
		spTask->RestartProcessing();
	}
}

bool TTaskManager::TasksRetryProcessing()
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	bool bChanged=false;
	BOOST_FOREACH(TTaskPtr& spTask, m_vTasks)
	{
		if(spTask->RetryProcessing())
			bChanged = true;
	}

	return bChanged;
}

void TTaskManager::TasksCancelProcessing()
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	BOOST_FOREACH(TTaskPtr& spTask, m_vTasks)
	{
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
		BOOST_FOREACH(TTaskPtr& spTask, m_vTasks)
		{
			ETaskCurrentState eState = spTask->GetTaskState();
			bFlag = (eState == eTaskState_Finished || eState == eTaskState_Cancelled || eState == eTaskState_Paused || eState == eTaskState_Error);

			if(!bFlag)
				break;
		}
	}

	return bFlag;
}

void TTaskManager::SetTasksDir(const TSmartPath& pathDir)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_pathTasksDir = pathDir;
}

void TTaskManager::GetStatsSnapshot(TTaskManagerStatsSnapshotPtr& spSnapshot) const
{
	if(!spSnapshot)
		THROW_CORE_EXCEPTION(eErr_InvalidArgument);

	spSnapshot->Clear();

	boost::shared_lock<boost::shared_mutex> lock(m_lock);

	size_t stRunningTasks = 0;
	BOOST_FOREACH(const TTaskPtr& spTask, m_vTasks)
	{
		TTaskStatsSnapshotPtr spStats(new TTaskStatsSnapshot);
		spTask->GetStatsSnapshot(spStats);

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

	BOOST_FOREACH(const TTaskPtr& spTask, m_vTasks)
	{
		if(spTask->IsRunning() && spTask->GetTaskState() == eTaskState_Processing)
			++stRunningTasks;
	}

	return stRunningTasks;
}

void TTaskManager::StopAllTasksNL()
{
	// kill all unfinished tasks - send kill request
	BOOST_FOREACH(TTaskPtr& spTask, m_vTasks)
	{
		spTask->RequestStopThread();
	}

	// wait for finishing
	BOOST_FOREACH(TTaskPtr& spTask, m_vTasks)
	{
		spTask->KillThread();
	}
}

END_CHCORE_NAMESPACE
