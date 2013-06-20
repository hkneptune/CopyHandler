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
#ifndef __TASKMANAGER_H__
#define __TASKMANAGER_H__

#include "libchcore.h"
#include "FeedbackHandlerBase.h"
#include "TPath.h"
#include "TTaskManagerStatsSnapshot.h"

BEGIN_CHCORE_NAMESPACE

class TTaskDefinition;
class TTask;
typedef boost::shared_ptr<TTask> TTaskPtr;

// special value representing no task
#define NO_TASK_SESSION_UNIQUE_ID				0

///////////////////////////////////////////////////////////////////////////
// TTaskManager

class LIBCHCORE_API TTaskManager
{
public:
	TTaskManager();
	~TTaskManager();

	void Create(IFeedbackHandlerFactory* piFeedbackHandlerFactory);

	TTaskPtr CreateTask(const TTaskDefinition& tTaskDefinition);
	TTaskPtr ImportTask(const TSmartPath& strTaskPath);

	size_t GetSize() const;

	TTaskPtr GetAt(size_t stIndex) const;
	TTaskPtr GetTaskBySessionUniqueID(size_t stSessionUniqueID) const;

	size_t Add(const TTaskPtr& spNewTask);

	void RemoveAt(size_t stIndex, size_t stCount = 1);
	void RemoveAll();
	void RemoveAllFinished();
	void RemoveFinished(const TTaskPtr& spSelTask);

	void ResumeWaitingTasks(size_t stMaxRunningTasks);
	void StopAllTasks();

	void SaveData();
	void LoadDataProgress();

	void TasksBeginProcessing();
	void TasksPauseProcessing();
	void TasksResumeProcessing();
	void TasksRestartProcessing();
	bool TasksRetryProcessing();
	void TasksCancelProcessing();

	bool AreAllFinished();

	void SetTasksDir(const TSmartPath& pathDir);

	void GetStatsSnapshot(TTaskManagerStatsSnapshotPtr& spSnapshot) const;
	size_t GetCountOfRunningTasks() const;

protected:
	void StopAllTasksNL();

	TTaskPtr CreateEmptyTask();

public:
	TSmartPath m_pathTasksDir;

private:
#pragma warning(push)
#pragma warning(disable: 4251)
	mutable boost::shared_mutex m_lock;
	std::vector<TTaskPtr> m_vTasks;		// vector with tasks objects
#pragma warning(pop)

	size_t m_stNextSessionUniqueID;		// global counter for providing unique ids for tasks per session (launch of the program)

protected:
	IFeedbackHandlerFactory* m_piFeedbackFactory;
};

END_CHCORE_NAMESPACE

#endif
