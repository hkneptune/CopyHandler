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
#include "SerializationHelpers.h"
#include "TBinarySerializer.h"
#include "TTaskStatsSnapshot.h"
#include "TCoreException.h"
#include "ErrorCodes.h"

BEGIN_CHCORE_NAMESPACE

///////////////////////////////////////////////////////////////////////////
// TSubTasksArray

TSubTasksArray::TSubTasksArray() :
	m_pSubTaskContext(NULL),
	m_eOperationType(eOperation_None),
	m_lSubOperationIndex(0)
{
}

TSubTasksArray::TSubTasksArray(const TOperationPlan& rOperationPlan, TSubTaskContext& rSubTaskContext) :
	m_pSubTaskContext(NULL),
	m_eOperationType(eOperation_None),
	m_lSubOperationIndex(0)
{
	Init(rOperationPlan, rSubTaskContext);
}

TSubTasksArray::~TSubTasksArray()
{
}

void TSubTasksArray::Init(const TOperationPlan& rOperationPlan, TSubTaskContext& rSubTaskContext)
{
	m_vSubTasks.clear();
	m_lSubOperationIndex = 0;
	m_pSubTaskContext = &rSubTaskContext;

	m_eOperationType = rOperationPlan.GetOperationType();

	switch(m_eOperationType)
	{
	case eOperation_Copy:
		{
			TSubTaskBasePtr spOperation = boost::make_shared<TSubTaskScanDirectories>(boost::ref(rSubTaskContext));
			AddSubTask(spOperation, true);
			spOperation = boost::make_shared<TSubTaskCopyMove>(boost::ref(rSubTaskContext));
			AddSubTask(spOperation, false);

			break;
		}
	case eOperation_Move:
		{
			TSubTaskBasePtr spOperation = boost::make_shared<TSubTaskFastMove>(boost::ref(rSubTaskContext));
			AddSubTask(spOperation, true);
			spOperation = boost::make_shared<TSubTaskScanDirectories>(boost::ref(rSubTaskContext));
			AddSubTask(spOperation, false);
			spOperation = boost::make_shared<TSubTaskCopyMove>(boost::ref(rSubTaskContext));
			AddSubTask(spOperation, false);
			spOperation = boost::make_shared<TSubTaskDelete>(boost::ref(rSubTaskContext));
			AddSubTask(spOperation, false);

			break;
		}
	default:
		THROW_CORE_EXCEPTION(eErr_UndefinedOperation);
	}
}

void TSubTasksArray::ResetProgressAndStats()
{
	InterlockedExchange(&m_lSubOperationIndex, 0);

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
	if(!m_pSubTaskContext)
		THROW_CORE_EXCEPTION(eErr_InternalProblem);

	TSubTaskBase::ESubOperationResult eResult = TSubTaskBase::eSubResult_Continue;

	size_t stSize = m_vSubTasks.size();
	long lIndex = InterlockedCompareExchange(&m_lSubOperationIndex, 0, 0);

	while(lIndex < stSize && eResult == TSubTaskBase::eSubResult_Continue)
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

		lIndex = InterlockedIncrement(&m_lSubOperationIndex);
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
	long lIndex = InterlockedCompareExchange(const_cast<volatile long*>(&m_lSubOperationIndex), 0L, 0L);
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

END_CHCORE_NAMESPACE
