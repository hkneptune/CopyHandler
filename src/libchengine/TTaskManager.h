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

#include "IFeedbackHandlerFactory.h"
#include "TTaskManagerStatsSnapshot.h"
#include "TTaskInfo.h"
#include "TObsoleteFiles.h"
#include "../liblogger/TMultiLoggerConfig.h"
#include "../liblogger/TLogger.h"
#include "../libserializer/ISerializerFactory.h"

namespace chengine
{
	class TTaskDefinition;
	class TTask;
	typedef std::shared_ptr<TTask> TTaskPtr;

	///////////////////////////////////////////////////////////////////////////
	// TTaskManager
	class LIBCHENGINE_API TTaskManager
	{
	public:
		TTaskManager(const serializer::ISerializerFactoryPtr& spSerializerFactory,
			const IFeedbackHandlerFactoryPtr& spFeedbackHandlerFactory,
			const chcore::TSmartPath& pathLogDir,
			const logger::TMultiLoggerConfigPtr& spMultiLoggerConfig,
			const logger::TLogFileDataPtr& spLogFileData,
			bool bForceRecreateSerializer = false);
		TTaskManager(const TTaskManager&) = delete;

		~TTaskManager();

		TTaskManager& operator=(const TTaskManager&) = delete;

		void Store(bool bForce);
		void Load();

		TTaskPtr CreateTask(const TTaskDefinition& tTaskDefinition);

		TTaskPtr ImportTask(const chcore::TSmartPath& strTaskPath);

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

		chcore::TSmartPath CreateTaskLogPath(const string::TString& strTaskUuid) const;
		void RemoveFilesForTask(const TTaskPtr& spTask);

	private:
#pragma warning(push)
#pragma warning(disable: 4251)
		mutable boost::shared_mutex m_lock;
#pragma warning(pop)

		TTaskInfoContainer m_tTasks;	// serializable

		chcore::TSmartPath m_pathLogDir;		// config-based, not serializable

		TObsoleteFiles m_tObsoleteFiles;

#pragma warning(push)
#pragma warning(disable: 4251)
		IFeedbackHandlerFactoryPtr m_spFeedbackFactory;
		serializer::ISerializerPtr m_spSerializer;
		serializer::ISerializerFactoryPtr m_spSerializerFactory;
		logger::TMultiLoggerConfigPtr m_spMultiLoggerConfig;
		logger::TLoggerPtr m_spLog;
#pragma warning(pop)
	};

	typedef std::shared_ptr<TTaskManager> TTaskManagerPtr;
}

#endif
