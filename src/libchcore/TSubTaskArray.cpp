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
#include <boost\smart_ptr\make_shared.hpp>
#include "TSubTaskScanDirectory.h"
#include "TSubTaskCopyMove.h"
#include "TSubTaskDelete.h"
#include "TSubTaskContext.h"
#include "TTaskLocalStats.h"
#include "TSubTaskFastMove.h"
#include "TTaskStatsSnapshot.h"
#include "TCoreException.h"
#include "ErrorCodes.h"
#include <boost/numeric/conversion/cast.hpp>

BEGIN_CHCORE_NAMESPACE

///////////////////////////////////////////////////////////////////////////
// TSubTasksArray

TSubTasksArray::TSubTasksArray(TSubTaskContext& rSubTaskContext) :
m_rSubTaskContext(rSubTaskContext),
	m_eOperationType(m_setModifications, eOperation_None),
	m_lSubOperationIndex(0),
	m_lLastStoredIndex(-1)
{
	m_setModifications[eMod_Added] = true;
}

TSubTasksArray::TSubTasksArray(const TOperationPlan& rOperationPlan, TSubTaskContext& rSubTaskContext) :
	m_rSubTaskContext(rSubTaskContext),
	m_eOperationType(m_setModifications, eOperation_None),
	m_lSubOperationIndex(0),
	m_lLastStoredIndex(-1)
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
	m_lSubOperationIndex.store(0, boost::memory_order_release);

	m_eOperationType = rOperationPlan.GetOperationType();

	switch(m_eOperationType)
	{
	case eOperation_Copy:
		{
			TSubTaskBasePtr spOperation = boost::make_shared<TSubTaskScanDirectories>(boost::ref(m_rSubTaskContext));
			AddSubTask(spOperation, true);
			spOperation = boost::make_shared<TSubTaskCopyMove>(boost::ref(m_rSubTaskContext));
			AddSubTask(spOperation, false);

			break;
		}
	case eOperation_Move:
		{
			TSubTaskBasePtr spOperation = boost::make_shared<TSubTaskFastMove>(boost::ref(m_rSubTaskContext));
			AddSubTask(spOperation, true);
			spOperation = boost::make_shared<TSubTaskScanDirectories>(boost::ref(m_rSubTaskContext));
			AddSubTask(spOperation, false);
			spOperation = boost::make_shared<TSubTaskCopyMove>(boost::ref(m_rSubTaskContext));
			AddSubTask(spOperation, false);
			spOperation = boost::make_shared<TSubTaskDelete>(boost::ref(m_rSubTaskContext));
			AddSubTask(spOperation, false);

			break;
		}
	default:
		THROW_CORE_EXCEPTION(eErr_UndefinedOperation);
	}
}

void TSubTasksArray::ResetProgressAndStats()
{
	m_lSubOperationIndex.store(0, boost::memory_order_release);

	std::pair<TSubTaskBasePtr, bool> tupleRow;
	BOOST_FOREACH(tupleRow, m_vSubTasks)
	{
		if(tupleRow.first == NULL)
			THROW_CORE_EXCEPTION(eErr_InternalProblem);

		tupleRow.first->Reset();
	}
}

TSubTaskBase::ESubOperationResult TSubTasksArray::Execute(bool bRunOnlyEstimationSubTasks)
{
	TSubTaskBase::ESubOperationResult eResult = TSubTaskBase::eSubResult_Continue;

	size_t stSize = m_vSubTasks.size();
	long lIndex = m_lSubOperationIndex.load(boost::memory_order_acquire);

	while(boost::numeric_cast<size_t>(lIndex) < stSize)
	{
		std::pair<TSubTaskBasePtr, bool>& rCurrentSubTask = m_vSubTasks.at(lIndex);
		TSubTaskBasePtr spCurrentSubTask = rCurrentSubTask.first;

		// if we run in estimation mode only, then stop processing and return to the caller
		if(bRunOnlyEstimationSubTasks && !rCurrentSubTask.second)
		{
			eResult = TSubTaskBase::eSubResult_Continue;
			break;
		}

		eResult = spCurrentSubTask->Exec();
		if(eResult != TSubTaskBase::eSubResult_Continue)
			break;

		lIndex = m_lSubOperationIndex.fetch_add(1, boost::memory_order_release) + 1;
	}

	return eResult;
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
	long lIndex = m_lSubOperationIndex.load(boost::memory_order_acquire);
	rSnapshot.SetCurrentSubtaskIndex(lIndex);

	// progress
	for(size_t stSubOperationIndex = 0; stSubOperationIndex < m_vSubTasks.size(); ++stSubOperationIndex)
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
		long lCurrentIndex = m_lSubOperationIndex.load(boost::memory_order_acquire);

		// subtasks are stored only once when added as they don't change (at least in context of their order and type)
		if(bAdded)
		{
			if(m_lLastStoredIndex != -1)
				THROW_CORE_EXCEPTION(eErr_InternalProblem);

			for(size_t stSubOperationIndex = 0; stSubOperationIndex < m_vSubTasks.size(); ++stSubOperationIndex)
			{
				const std::pair<TSubTaskBasePtr, bool>& rCurrentSubTask = m_vSubTasks[stSubOperationIndex];

				ISerializerRowData& rRow = spContainer->GetRow(stSubOperationIndex, bAdded);
				rRow.SetValue(_T("type"), rCurrentSubTask.first->GetSubOperationType());
				rRow.SetValue(_T("is_current"), false);
				rRow.SetValue(_T("is_estimation"), rCurrentSubTask.second);
			}
		}

		// serialize current index
		if(bAdded || lCurrentIndex != m_lLastStoredIndex)
		{
			// mark subtask at current index as "current"; don't do that if we just finished.
			if(boost::numeric_cast<size_t>(lCurrentIndex) != m_vSubTasks.size())
			{
				ISerializerRowData& rRow = spContainer->GetRow(lCurrentIndex, false);
				rRow.SetValue(_T("is_current"), true);
			}

			// unmark the old "current" subtask
			if(m_lLastStoredIndex != -1)
			{
				ISerializerRowData& rRow = spContainer->GetRow(m_lLastStoredIndex, false);
				rRow.SetValue(_T("is_current"), false);
			}
		}

		m_lLastStoredIndex = lCurrentIndex;
	}

	m_setModifications.reset();

	///////////////////////////////////////////////////////////////////////
	// store all the subtasks
	for(size_t stSubOperationIndex = 0; stSubOperationIndex < m_vSubTasks.size(); ++stSubOperationIndex)
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
		if(spRowReader->Next())
			spRowReader->GetValue(_T("operation"), *(int*)&m_eOperationType.Modify());
	}

	///////////////////////////////////////////////////////////////////////
	{
		m_lLastStoredIndex = -1;

		ISerializerContainerPtr spContainer = spSerializer->GetContainer(_T("subtasks"));
		ISerializerRowReaderPtr spRowReader = spContainer->GetRowReader();

		InitSubtasksColumns(spContainer);

		while(spRowReader->Next())
		{
			long lID = 0;
			int iType = 0;
			bool bIsCurrent = false;
			bool bIsEstimation = false;

			spRowReader->GetValue(_T("id"), lID);
			spRowReader->GetValue(_T("type"), iType);
			spRowReader->GetValue(_T("is_current"), bIsCurrent);
			spRowReader->GetValue(_T("is_estimation"), bIsEstimation);

			if(bIsCurrent)
			{
				m_lSubOperationIndex.store(lID, boost::memory_order_release);
				m_lLastStoredIndex = lID;
			}

			// create subtask, load it and put into the array
			TSubTaskBasePtr spSubTask = CreateSubtask((ESubOperationType)iType, m_rSubTaskContext);
			spSubTask->Load(spSerializer);

			if(boost::numeric_cast<size_t>(lID) != m_vSubTasks.size())
				THROW_CORE_EXCEPTION(eErr_InvalidData);

			m_vSubTasks.push_back(std::make_pair(spSubTask, bIsEstimation));
		}

		if(m_lLastStoredIndex == -1)
		{
			m_lSubOperationIndex.store(boost::numeric_cast<long>(m_vSubTasks.size()), boost::memory_order_release);
			m_lLastStoredIndex = boost::numeric_cast<long>(m_vSubTasks.size());
		}
	}

	m_setModifications.reset();
}

TSubTaskBasePtr TSubTasksArray::CreateSubtask(ESubOperationType eType, TSubTaskContext& rContext)
{
	switch(eType)
	{
	case eSubOperation_FastMove:
		return boost::make_shared<TSubTaskFastMove>(boost::ref(rContext));

	case eSubOperation_Scanning:
		return boost::make_shared<TSubTaskScanDirectories>(boost::ref(rContext));

	case eSubOperation_Copying:
		return boost::make_shared<TSubTaskCopyMove>(boost::ref(rContext));

	case eSubOperation_Deleting:
		return boost::make_shared<TSubTaskDelete>(boost::ref(rContext));

	default:
		THROW_CORE_EXCEPTION(eErr_UnhandledCase);
	}
}

IColumnsDefinition& TSubTasksArray::InitSubtasksColumns(const ISerializerContainerPtr& spContainer) const
{
	IColumnsDefinition& rColumns = spContainer->GetColumnsDefinition();
	if(rColumns.IsEmpty())
	{
		rColumns.AddColumn(_T("id"), IColumnsDefinition::eType_long);
		rColumns.AddColumn(_T("type"), IColumnsDefinition::eType_int);
		rColumns.AddColumn(_T("is_current"), IColumnsDefinition::eType_bool);
		rColumns.AddColumn(_T("is_estimation"), IColumnsDefinition::eType_bool);
	}

	return rColumns;
}

IColumnsDefinition& TSubTasksArray::InitSubtasksInfoColumns(const ISerializerContainerPtr& spContainer) const
{
	IColumnsDefinition& rColumns = spContainer->GetColumnsDefinition();
	if(rColumns.IsEmpty())
	{
		rColumns.AddColumn(_T("id"), IColumnsDefinition::eType_long);
		rColumns.AddColumn(_T("operation"), IColumnsDefinition::eType_int);
	}

	return rColumns;
}

END_CHCORE_NAMESPACE
