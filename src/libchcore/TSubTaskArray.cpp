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
#include "TBasicProgressInfo.h"
#include "TTaskLocalStats.h"
#include "TSubTaskFastMove.h"

BEGIN_CHCORE_NAMESPACE

TSubTasksArray::TSubTasksArray(const TOperationPlan& rOperationPlan, TSubTaskContext& rSubTaskContext) :
	m_rSubTaskContext(rSubTaskContext)
{
	switch(rOperationPlan.GetOperationType())
	{
	case eOperation_Copy:
		{
			TSubTaskBasePtr spOperation = boost::make_shared<TSubTaskScanDirectories>(boost::ref(rSubTaskContext));
			m_vSubTasks.push_back(boost::make_tuple(spOperation, 0.05, true));
			spOperation = boost::make_shared<TSubTaskCopyMove>(boost::ref(rSubTaskContext));
			m_vSubTasks.push_back(boost::make_tuple(spOperation, 0.95, false));

			break;
		}
	case eOperation_Move:
		{
			TSubTaskBasePtr spOperation = boost::make_shared<TSubTaskFastMove>(boost::ref(rSubTaskContext));
			m_vSubTasks.push_back(boost::make_tuple(spOperation, 0.05, true));
			spOperation = boost::make_shared<TSubTaskScanDirectories>(boost::ref(rSubTaskContext));
			m_vSubTasks.push_back(boost::make_tuple(spOperation, 0.05, false));
			spOperation = boost::make_shared<TSubTaskCopyMove>(boost::ref(rSubTaskContext));
			m_vSubTasks.push_back(boost::make_tuple(spOperation, 0.85, false));
			spOperation = boost::make_shared<TSubTaskDelete>(boost::ref(rSubTaskContext));
			m_vSubTasks.push_back(boost::make_tuple(spOperation, 0.05, false));

			break;
		}
	default:
		THROW_CORE_EXCEPTION(eErr_UndefinedOperation);
	}
}

TSubTasksArray::~TSubTasksArray()
{
}

TSubTaskBase::ESubOperationResult TSubTasksArray::Execute(bool bRunOnlyEstimationSubTasks)
{
	TSubTaskBase::ESubOperationResult eResult = TSubTaskBase::eSubResult_Continue;

	size_t stSubOperationIndex = m_rSubTaskContext.GetTaskBasicProgressInfo().GetSubOperationIndex();
	for(; stSubOperationIndex < m_vSubTasks.size() && eResult == TSubTaskBase::eSubResult_Continue; ++stSubOperationIndex)
	{
		boost::tuples::tuple<TSubTaskBasePtr, double, bool>& rCurrentSubTask = m_vSubTasks[stSubOperationIndex];
		TSubTaskBasePtr spCurrentSubTask = rCurrentSubTask.get<0>();

		m_rSubTaskContext.GetTaskLocalStats().SetCurrentSubOperationType(spCurrentSubTask->GetSubOperationType());
		// set current sub-operation index to allow resuming
		m_rSubTaskContext.GetTaskBasicProgressInfo().SetSubOperationIndex(stSubOperationIndex);

		// if we run in estimation mode only, then stop processing and return to the caller
		if(bRunOnlyEstimationSubTasks && !rCurrentSubTask.get<2>())
		{
			eResult = TSubTaskBase::eSubResult_Continue;
			break;
		}

		eResult = spCurrentSubTask->Exec();
	}

	return eResult;
}

END_CHCORE_NAMESPACE
