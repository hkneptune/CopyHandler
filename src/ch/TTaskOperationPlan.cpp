// ============================================================================
//  Copyright (C) 2001-2009 by Jozef Starosczyk
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
/// @file  TSubTasks.cpp
/// @date  2010/09/18
/// @brief Contains implementation of classes for subtasks management.
// ============================================================================
#include "stdafx.h"
#include "TTaskOperationPlan.h"

////////////////////////////////////////////////////////////////////////////
// class TOperationPlan

TOperationPlan::TOperationPlan() :
	m_eOperation(eOperation_None)
{
}

TOperationPlan::~TOperationPlan()
{
}

void TOperationPlan::SetOperationType(EOperationType eOperation)
{
	switch(eOperation)
	{
	case eOperation_None:
		THROW(_T("Cannot set operation type 'none'"), 0, 0, 0);
		break;

	case eOperation_Copy:
		{
			boost::unique_lock<boost::shared_mutex> lock(m_lock);
			m_vSubOperations.clear();
			m_vSubOperations.push_back(std::make_pair(eSubOperation_Scanning, 0.05));
			m_vSubOperations.push_back(std::make_pair(eSubOperation_Copying, 0.95));
			break;
		}

	case eOperation_Move:
		{
			boost::unique_lock<boost::shared_mutex> lock(m_lock);
			m_vSubOperations.clear();
			m_vSubOperations.push_back(std::make_pair(eSubOperation_Scanning, 0.05));
			m_vSubOperations.push_back(std::make_pair(eSubOperation_Copying, 0.90));
			m_vSubOperations.push_back(std::make_pair(eSubOperation_Deleting, 0.05));
			break;
		}

	BOOST_STATIC_ASSERT(eOperation_Move == eOperation_Max - 1);

	default:
		THROW(_T("Unhandled case"), 0, 0, 0);
	}

	m_eOperation = eOperation;
}

EOperationType TOperationPlan::GetOperationType() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_eOperation;
}

size_t TOperationPlan::GetSubOperationsCount() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_vSubOperations.size();
}

ESubOperationType TOperationPlan::GetSubOperationAt(size_t stIndex) const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	if(stIndex >= m_vSubOperations.size())
		THROW(_T("Index out of bounds"), 0, 0, 0);
	else
		return m_vSubOperations[stIndex].first;
}

double TOperationPlan::GetEstimatedTimeAt(size_t stIndex) const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	if(stIndex >= m_vSubOperations.size())
		THROW(_T("Index out of bounds"), 0, 0, 0);
	else
		return m_vSubOperations[stIndex].second;
}
