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
#include "IFeedbackHandlerFactory.h"
#include "TPath.h"
#include "TTaskManagerStatsSnapshot.h"
#include "TTaskInfo.h"
#include "ISerializer.h"
#include "ISerializerFactory.h"
#include "TObsoleteFiles.h"
#include "..\liblogger\TMultiLoggerConfig.h"

namespace chcore
{
	class TTaskDefinition;
	class TTask;
	typedef std::shared_ptr<TTask> TTaskPtr;

	///////////////////////////////////////////////////////////////////////////
	// TTaskManager
	class LIBCHCORE_API TTaskManager
	{
	public:
		TTaskManager(const ISerializerFactoryPtr& spSerializerFactory,
			const IFeedbackHandlerFactoryPtr& spFeedbackHandlerFactory,
			const TSmartPath& pathLogDir,
			const TMultiLoggerConfigPtr& spMultiLoggerConfig,
			bool bForceRecreateSerializer = false);

		~TTaskManager();

		void Store(bool bForce);
		void Load();

		TTaskPtr CreateTask(const TTaskDefinition& tTaskDefinition);

		TTaskPtr ImportTask(const TSmartPath& strTaskPath);

		size_t GetSize() const;

		TTaskPtr GetAt(size_t stIndex) const;
		TTaskPtr GetTaskByTaskID(taskid_t tTaskID) const;

		void Add(const TTaskPtr& spNewTask);

		void ClearBeforeExit();
		void RemoveAllFinished();
		void RemoveFinished(const TTaskPtr& spSelTask);

		void ResumeWaitingTasks(size_t stMaxRunningTasks);
		void StopAllTasks();

		void TasksBeginProcessing();
		void TasksPauseProcessing();
		void TasksResumeProcessing();
		void TasksRestartProcessing();
		bool TasksRetryProcessing();
		void TasksCancelProcessing();

		bool AreAllFinished();

		void GetStatsSnapshot(TTaskManagerStatsSnapshotPtr& spSnapshot) const;
		size_t GetCountOfRunningTasks() const;

	protected:
		void StopAllTasksNL();

		TSmartPath CreateTaskLogPath(const TString& strTaskUuid) const;

	private:
#pragma warning(push)
#pragma warning(disable: 4251)
		mutable boost::shared_mutex m_lock;
#pragma warning(pop)

		TTaskInfoContainer m_tTasks;	// serializable

		TSmartPath m_pathLogDir;		// config-based, not serializable

		TObsoleteFiles m_tObsoleteFiles;

#pragma warning(push)
#pragma warning(disable: 4251)
		IFeedbackHandlerFactoryPtr m_spFeedbackFactory;
		ISerializerPtr m_spSerializer;
		ISerializerFactoryPtr m_spSerializerFactory;
		TMultiLoggerConfigPtr m_spMultiLoggerConfig;
#pragma warning(pop)
	};

	typedef std::shared_ptr<TTaskManager> TTaskManagerPtr;
}

#endif
