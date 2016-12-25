// ============================================================================
//  Copyright (C) 2001-2011 by Jozef Starosczyk
//  ixen@copyhandler.com
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU Library General Public License
//  (version 2) as published by the Free Software Foundation;
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU Library General Public
//  License along with this program; if not, write to the
//  Free Software Foundation, Inc.,
//  59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// ============================================================================
/// @file  TSubTaskArray.cpp
/// @date  2011/11/08
/// @brief File contain implementation of a class handling a sequence of subtasks.
// ============================================================================
#include "stdafx.h"
#include "TSubTaskArray.h"
#include "TTaskOperationPlan.h"
#include "TSubTaskScanDirectory.h"
#include "TSubTaskCopyMove.h"
#include "TSubTaskDelete.h"
#include "TSubTaskContext.h"
#include "TTaskLocalStats.h"
#include "TSubTaskFastMove.h"
#include "TTaskStatsSnapshot.h"
#include <boost/numeric/conversion/cast.hpp>
#include "TTaskConfiguration.h"
#include "../libchcore/TCoreException.h"

using namespace serializer;
using namespace chcore;

namespace chengine
{
	///////////////////////////////////////////////////////////////////////////
	// TSubTasksArray

	TSubTasksArray::TSubTasksArray(TSubTaskContext& rSubTaskContext) :
		m_rSubTaskContext(rSubTaskContext),
		m_eOperationType(m_setModifications, eOperation_None),
		m_oidSubOperationIndex(0),
		m_oidLastStoredIndex((object_id_t)-1)
	{
		m_setModifications[eMod_Added] = true;
	}

	TSubTasksArray::TSubTasksArray(const TOperationPlan& rOperationPlan, TSubTaskContext& rSubTaskContext) :
		m_rSubTaskContext(rSubTaskContext),
		m_eOperationType(m_setModifications, eOperation_None),
		m_oidSubOperationIndex(0),
		m_oidLastStoredIndex((object_id_t)-1)
	{
		m_setModifications[eMod_Added] = true;
		Init(rOperationPlan);
	}

	TSubTasksArray::~TSubTasksArray()
	{
	}

	void TSubTasksArray::Init(const TOperationPlan& rOperationPlan)
	{
		m_vSubTasks.clear();
		m_rSubTaskContext.GetFilesCache().Clear();
		m_oidSubOperationIndex.store(0, std::memory_order_release);

		m_eOperationType = rOperationPlan.GetOperationType();

		bool bReadTasksSizeBeforeBlocking = GetTaskPropValue<eTO_ScanDirectoriesBeforeBlocking>(m_rSubTaskContext.GetConfig());
		bool bFastMoveBeforeBlocking = GetTaskPropValue<eTO_FastMoveBeforeBlocking>(m_rSubTaskContext.GetConfig());

		switch (m_eOperationType)
		{
		case eOperation_Copy:
			{
				TSubTaskBasePtr spOperation = std::make_shared<TSubTaskScanDirectories>(boost::ref(m_rSubTaskContext));
				AddSubTask(spOperation, bReadTasksSizeBeforeBlocking);
				spOperation = std::make_shared<TSubTaskCopyMove>(boost::ref(m_rSubTaskContext));
				AddSubTask(spOperation, false);

				break;
			}

		case eOperation_Move:
			{
				TSubTaskBasePtr spOperation;
				bool bReorderFastMove = bReadTasksSizeBeforeBlocking && !bFastMoveBeforeBlocking;

				// fastmove (if not reordered)
				if(!bReorderFastMove)
				{
					spOperation = std::make_shared<TSubTaskFastMove>(boost::ref(m_rSubTaskContext));
					AddSubTask(spOperation, bFastMoveBeforeBlocking);
				}

				// scanning
				spOperation = std::make_shared<TSubTaskScanDirectories>(boost::ref(m_rSubTaskContext));
				AddSubTask(spOperation, bReadTasksSizeBeforeBlocking);

				// fastmove (if reordered)
				if(bReorderFastMove)
				{
					spOperation = std::make_shared<TSubTaskFastMove>(boost::ref(m_rSubTaskContext));
					AddSubTask(spOperation, bFastMoveBeforeBlocking);
				}

				// copy/move
				spOperation = std::make_shared<TSubTaskCopyMove>(boost::ref(m_rSubTaskContext));
				AddSubTask(spOperation, false);

				// delete
				spOperation = std::make_shared<TSubTaskDelete>(boost::ref(m_rSubTaskContext));
				AddSubTask(spOperation, false);

				break;
			}

		default:
			throw TCoreException(eErr_UndefinedOperation, L"Operation type not known to the engine", LOCATION);
		}
	}

	void TSubTasksArray::ResetProgressAndStats()
	{
		m_oidSubOperationIndex.store(0, std::memory_order_release);

		for (const std::pair<TSubTaskBasePtr, bool>& tupleRow : m_vSubTasks)
		{
			if (tupleRow.first == nullptr)
				throw TCoreException(eErr_InternalProblem, L"SubTask is null", LOCATION);

			tupleRow.first->Reset();
		}
	}

	void TSubTasksArray::InitBeforeExec()
	{
		object_id_t oidSize = boost::numeric_cast<object_id_t>(m_vSubTasks.size());
		object_id_t oidIndex = m_oidSubOperationIndex.load(std::memory_order_acquire);

		if(oidIndex < oidSize)
		{
			std::pair<TSubTaskBasePtr, bool>& rCurrentSubTask = m_vSubTasks.at(boost::numeric_cast<size_t>(oidIndex));
			TSubTaskBasePtr spCurrentSubTask = rCurrentSubTask.first;

			spCurrentSubTask->InitBeforeExec();
		}
	}

	TSubTaskBase::ESubOperationResult TSubTasksArray::Execute(const IFeedbackHandlerPtr& spFeedbackHandler, bool bRunOnlyEstimationSubTasks)
	{
		TSubTaskBase::ESubOperationResult eResult = TSubTaskBase::eSubResult_Continue;

		object_id_t oidSize = boost::numeric_cast<object_id_t>(m_vSubTasks.size());
		object_id_t oidIndex = m_oidSubOperationIndex.load(std::memory_order_acquire);

		while (oidIndex < oidSize)
		{
			std::pair<TSubTaskBasePtr, bool>& rCurrentSubTask = m_vSubTasks.at(boost::numeric_cast<size_t>(oidIndex));
			TSubTaskBasePtr spCurrentSubTask = rCurrentSubTask.first;

			spCurrentSubTask->InitBeforeExec();

			// if we run in estimation mode only, then stop processing and return to the caller
			if (bRunOnlyEstimationSubTasks && !rCurrentSubTask.second)
			{
				eResult = TSubTaskBase::eSubResult_Continue;
				break;
			}

			eResult = spCurrentSubTask->Exec(spFeedbackHandler);
			if (eResult != TSubTaskBase::eSubResult_Continue)
				break;

			oidIndex = m_oidSubOperationIndex.fetch_add(1, std::memory_order_release) + 1;
		}

		return eResult;
	}

	bool TSubTasksArray::AreAllBasePathsProcessed() const
	{
		return m_rSubTaskContext.GetBasePaths()->AllMarkedAsSkipFurtherProcessing();
	}

	void TSubTasksArray::AddSubTask(const TSubTaskBasePtr& spOperation, bool bIsPartOfEstimation)
	{
		m_vSubTasks.push_back(std::make_pair(spOperation, bIsPartOfEstimation));
	}

	void TSubTasksArray::GetStatsSnapshot(TSubTaskArrayStatsSnapshot& rSnapshot) const
	{
		rSnapshot.Clear();

		// current task
		// ugly const_cast - const method, non-const interlocked intrinsic and we're really not modifying the member...
		object_id_t oidIndex = m_oidSubOperationIndex.load(std::memory_order_acquire);
		rSnapshot.SetCurrentSubtaskIndex(oidIndex);

		// progress
		for (size_t stSubOperationIndex = 0; stSubOperationIndex < m_vSubTasks.size(); ++stSubOperationIndex)
		{
			const std::pair<TSubTaskBasePtr, bool>& rCurrentSubTask = m_vSubTasks[stSubOperationIndex];
			TSubTaskBasePtr spCurrentSubTask = rCurrentSubTask.first;

			TSubTaskStatsSnapshotPtr spSubtaskSnapshot(new TSubTaskStatsSnapshot);

			spCurrentSubTask->GetStatsSnapshot(spSubtaskSnapshot);
			rSnapshot.AddSubTaskSnapshot(spSubtaskSnapshot);
		}
	}

	EOperationType TSubTasksArray::GetOperationType() const
	{
		return m_eOperationType;
	}

	void TSubTasksArray::Store(const ISerializerPtr& spSerializer) const
	{
		bool bAdded = m_setModifications[eMod_Added];

		///////////////////////////////////////////////////////////////////////
		if (m_eOperationType.IsModified())
		{
			ISerializerContainerPtr spContainer = spSerializer->GetContainer(_T("subtasks_info"));
			InitSubtasksInfoColumns(spContainer);

			ISerializerRowData& rRow = spContainer->GetRow(0, bAdded);

			rRow.SetValue(_T("operation"), m_eOperationType.Get());
		}

		///////////////////////////////////////////////////////////////////////
		{
			ISerializerContainerPtr spContainer = spSerializer->GetContainer(_T("subtasks"));
			InitSubtasksColumns(spContainer);

			// base data
			object_id_t oidCurrentIndex = m_oidSubOperationIndex.load(std::memory_order_acquire);

			// subtasks are stored only once when added as they don't change (at least in context of their order and type)
			if (bAdded)
			{
				if (m_oidLastStoredIndex != -1)
					throw TCoreException(eErr_InternalProblem, L"Last object id is set when array is marked as added", LOCATION);

				for (size_t stSubOperationIndex = 0; stSubOperationIndex < m_vSubTasks.size(); ++stSubOperationIndex)
				{
					const std::pair<TSubTaskBasePtr, bool>& rCurrentSubTask = m_vSubTasks[stSubOperationIndex];

					ISerializerRowData& rRow = spContainer->GetRow(boost::numeric_cast<object_id_t>(stSubOperationIndex), bAdded);
					rRow.SetValue(_T("type"), rCurrentSubTask.first->GetSubOperationType());
					rRow.SetValue(_T("is_current"), false);
					rRow.SetValue(_T("is_estimation"), rCurrentSubTask.second);
				}
			}

			// serialize current index
			if (bAdded || oidCurrentIndex != m_oidLastStoredIndex)
			{
				// mark subtask at current index as "current"; don't do that if we just finished.
				if (boost::numeric_cast<size_t>(oidCurrentIndex) != m_vSubTasks.size())
				{
					ISerializerRowData& rRow = spContainer->GetRow(oidCurrentIndex, false);
					rRow.SetValue(_T("is_current"), true);
				}

				// unmark the old "current" subtask
				if (m_oidLastStoredIndex != -1 && m_oidLastStoredIndex != m_vSubTasks.size())
				{
					ISerializerRowData& rRow = spContainer->GetRow(m_oidLastStoredIndex, false);
					rRow.SetValue(_T("is_current"), false);
				}
			}

			m_oidLastStoredIndex = oidCurrentIndex;
		}

		m_setModifications.reset();

		///////////////////////////////////////////////////////////////////////
		// store all the subtasks
		for (size_t stSubOperationIndex = 0; stSubOperationIndex < m_vSubTasks.size(); ++stSubOperationIndex)
		{
			const std::pair<TSubTaskBasePtr, bool>& rCurrentSubTask = m_vSubTasks[stSubOperationIndex];
			rCurrentSubTask.first->Store(spSerializer);
		}
	}

	void TSubTasksArray::Load(const ISerializerPtr& spSerializer)
	{
		///////////////////////////////////////////////////////////////////////
		{
			ISerializerContainerPtr spContainer = spSerializer->GetContainer(_T("subtasks_info"));

			InitSubtasksInfoColumns(spContainer);

			ISerializerRowReaderPtr spRowReader = spContainer->GetRowReader();
			if (spRowReader->Next())
				spRowReader->GetValue(_T("operation"), *(int*)&m_eOperationType.Modify());
		}

		///////////////////////////////////////////////////////////////////////
		{
			m_oidLastStoredIndex = (object_id_t)-1;

			ISerializerContainerPtr spContainer = spSerializer->GetContainer(_T("subtasks"));
			ISerializerRowReaderPtr spRowReader = spContainer->GetRowReader();

			InitSubtasksColumns(spContainer);

			while (spRowReader->Next())
			{
				object_id_t oidID = 0;
				int iType = 0;
				bool bIsCurrent = false;
				bool bIsEstimation = false;

				spRowReader->GetValue(_T("id"), oidID);
				spRowReader->GetValue(_T("type"), iType);
				spRowReader->GetValue(_T("is_current"), bIsCurrent);
				spRowReader->GetValue(_T("is_estimation"), bIsEstimation);

				if (bIsCurrent)
				{
					m_oidSubOperationIndex.store(oidID, std::memory_order_release);
					m_oidLastStoredIndex = oidID;
				}

				// create subtask, load it and put into the array
				TSubTaskBasePtr spSubTask = CreateSubtask((ESubOperationType)iType, m_rSubTaskContext);
				spSubTask->Load(spSerializer);

				if (boost::numeric_cast<size_t>(oidID) != m_vSubTasks.size())
					throw TCoreException(eErr_InvalidData, L"Object id does not match count of subtasks", LOCATION);

				m_vSubTasks.push_back(std::make_pair(spSubTask, bIsEstimation));
			}

			if (m_oidLastStoredIndex == -1)
				m_oidSubOperationIndex.store(boost::numeric_cast<long>(m_vSubTasks.size()), std::memory_order_release);
		}

		m_setModifications.reset();
	}

	TSubTaskBasePtr TSubTasksArray::CreateSubtask(ESubOperationType eType, TSubTaskContext& rContext)
	{
		switch (eType)
		{
		case eSubOperation_FastMove:
			return std::make_shared<TSubTaskFastMove>(boost::ref(rContext));

		case eSubOperation_Scanning:
			return std::make_shared<TSubTaskScanDirectories>(boost::ref(rContext));

		case eSubOperation_Copying:
			return std::make_shared<TSubTaskCopyMove>(boost::ref(rContext));

		case eSubOperation_Deleting:
			return std::make_shared<TSubTaskDelete>(boost::ref(rContext));

		default:
			throw TCoreException(eErr_UnhandledCase, L"SubTask is unknown to the engine", LOCATION);
		}
	}

	IColumnsDefinition& TSubTasksArray::InitSubtasksColumns(const ISerializerContainerPtr& spContainer) const
	{
		IColumnsDefinition& rColumns = spContainer->GetColumnsDefinition();
		if (rColumns.IsEmpty())
		{
			rColumns.AddColumn(_T("id"), ColumnType<object_id_t>::value);
			rColumns.AddColumn(_T("type"), IColumnsDefinition::eType_int);
			rColumns.AddColumn(_T("is_current"), IColumnsDefinition::eType_bool);
			rColumns.AddColumn(_T("is_estimation"), IColumnsDefinition::eType_bool);
		}

		return rColumns;
	}

	IColumnsDefinition& TSubTasksArray::InitSubtasksInfoColumns(const ISerializerContainerPtr& spContainer) const
	{
		IColumnsDefinition& rColumns = spContainer->GetColumnsDefinition();
		if (rColumns.IsEmpty())
		{
			rColumns.AddColumn(_T("id"), ColumnType<object_id_t>::value);
			rColumns.AddColumn(_T("operation"), IColumnsDefinition::eType_int);
		}

		return rColumns;
	}
}
