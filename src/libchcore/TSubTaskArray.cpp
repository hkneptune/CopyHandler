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

namespace details
{
	///////////////////////////////////////////////////////////////////////////
	// TTaskBasicProgressInfo

	TTaskBasicProgressInfo::TTaskBasicProgressInfo() :
		m_stSubOperationIndex(0)
	{
	}

	TTaskBasicProgressInfo::~TTaskBasicProgressInfo()
	{
	}

	void TTaskBasicProgressInfo::ResetProgress()
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);
		m_stSubOperationIndex = 0;
	}

	void TTaskBasicProgressInfo::SetSubOperationIndex(size_t stSubOperationIndex)
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);
		m_stSubOperationIndex = stSubOperationIndex;
	}

	size_t TTaskBasicProgressInfo::GetSubOperationIndex() const
	{
		boost::shared_lock<boost::shared_mutex> lock(m_lock);
		return m_stSubOperationIndex;
	}

	void TTaskBasicProgressInfo::IncreaseSubOperationIndex()
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);
		++m_stSubOperationIndex;
	}

	void TTaskBasicProgressInfo::Serialize(TReadBinarySerializer& rSerializer)
	{
		using Serializers::Serialize;

		size_t stSubOperationIndex = 0;
		Serialize(rSerializer, stSubOperationIndex);

		boost::unique_lock<boost::shared_mutex> lock(m_lock);

		m_stSubOperationIndex = stSubOperationIndex;
	}

	void TTaskBasicProgressInfo::Serialize(TWriteBinarySerializer& rSerializer) const
	{
		using Serializers::Serialize;

		size_t stSubOperationIndex = 0;
		{
			boost::shared_lock<boost::shared_mutex> lock(m_lock);
			stSubOperationIndex = m_stSubOperationIndex;
		}

		Serialize(rSerializer, stSubOperationIndex);
	}
}

///////////////////////////////////////////////////////////////////////////
// TSubTasksArray

TSubTasksArray::TSubTasksArray() :
	m_pSubTaskContext(NULL)
{
}

TSubTasksArray::TSubTasksArray(const TOperationPlan& rOperationPlan, TSubTaskContext& rSubTaskContext) :
	m_pSubTaskContext(NULL)
{
	Init(rOperationPlan, rSubTaskContext);
}

TSubTasksArray::~TSubTasksArray()
{
}

void TSubTasksArray::Init(const TOperationPlan& rOperationPlan, TSubTaskContext& rSubTaskContext)
{
	m_vSubTasks.clear();
	m_tProgressInfo.ResetProgress();
	m_pSubTaskContext = &rSubTaskContext;

	switch(rOperationPlan.GetOperationType())
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
	m_tProgressInfo.ResetProgress();

	std::pair<TSubTaskBasePtr, bool> tupleRow;
	BOOST_FOREACH(tupleRow, m_vSubTasks)
	{
		if(tupleRow.first == NULL)
			THROW_CORE_EXCEPTION(eErr_InternalProblem);

		tupleRow.first->Reset();
	}
}

void TSubTasksArray::SerializeProgress(TReadBinarySerializer& rSerializer)
{
	m_tProgressInfo.Serialize(rSerializer);
	std::pair<TSubTaskBasePtr, bool> tupleRow;
	BOOST_FOREACH(tupleRow, m_vSubTasks)
	{
		tupleRow.first->GetProgressInfo().Serialize(rSerializer);
	}
}

void TSubTasksArray::SerializeProgress(TWriteBinarySerializer& rSerializer) const
{
	m_tProgressInfo.Serialize(rSerializer);
	std::pair<TSubTaskBasePtr, bool> tupleRow;
	BOOST_FOREACH(tupleRow, m_vSubTasks)
	{
		tupleRow.first->GetProgressInfo().Serialize(rSerializer);
	}
}

TSubTaskBase::ESubOperationResult TSubTasksArray::Execute(bool bRunOnlyEstimationSubTasks)
{
	if(!m_pSubTaskContext)
		THROW_CORE_EXCEPTION(eErr_InternalProblem);

	TSubTaskBase::ESubOperationResult eResult = TSubTaskBase::eSubResult_Continue;

	size_t stSubOperationIndex = m_tProgressInfo.GetSubOperationIndex();

	for(; stSubOperationIndex < m_vSubTasks.size() && eResult == TSubTaskBase::eSubResult_Continue; ++stSubOperationIndex)
	{
		std::pair<TSubTaskBasePtr, bool>& rCurrentSubTask = m_vSubTasks[stSubOperationIndex];
		TSubTaskBasePtr spCurrentSubTask = rCurrentSubTask.first;

		// set current sub-operation index to allow resuming
		m_tProgressInfo.SetSubOperationIndex(stSubOperationIndex);

		// if we run in estimation mode only, then stop processing and return to the caller
		if(bRunOnlyEstimationSubTasks && !rCurrentSubTask.second)
		{
			eResult = TSubTaskBase::eSubResult_Continue;
			break;
		}

		eResult = spCurrentSubTask->Exec();
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
	size_t stSubOperationIndex = m_tProgressInfo.GetSubOperationIndex();
	rSnapshot.SetCurrentSubtaskIndex(stSubOperationIndex);

	// progress
	for(stSubOperationIndex = 0; stSubOperationIndex < m_vSubTasks.size(); ++stSubOperationIndex)
	{
		const std::pair<TSubTaskBasePtr, bool>& rCurrentSubTask = m_vSubTasks[stSubOperationIndex];
		TSubTaskBasePtr spCurrentSubTask = rCurrentSubTask.first;

		TSubTaskStatsSnapshotPtr spSubtaskSnapshot(new TSubTaskStatsSnapshot);

		spCurrentSubTask->GetStatsSnapshot(spSubtaskSnapshot);
		rSnapshot.AddSubTaskSnapshot(spSubtaskSnapshot);
	}
}

END_CHCORE_NAMESPACE
