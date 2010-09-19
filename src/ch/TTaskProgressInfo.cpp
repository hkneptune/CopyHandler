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
/// @file  TTaskProgressInfo.cpp
/// @date  2010/09/19
/// @brief Contains implementation of classes related to task progress reporting.
// ============================================================================
#include "stdafx.h"
#include "TTaskProgressInfo.h"

///////////////////////////////////////////////////////////////////////////
// TProgressSnapshot

TProgressSnapshot::TProgressSnapshot() :
	m_stCurrentOperation(0),
	m_dCountTotalProgress(0.0),
	m_dSizeTotalProgress(0.0)
{
}

TProgressSnapshot::~TProgressSnapshot()
{
}

///////////////////////////////////////////////////////////////////////////
// TTaskProgressInfo

TTaskProgressInfo::TTaskProgressInfo() :
	m_stSubOperationIndex(0)
{
}

TTaskProgressInfo::~TTaskProgressInfo()
{
}

void TTaskProgressInfo::CreateFromOperationPlan(const TOperationPlan& rOperationPlan)
{
	m_stSubOperationIndex = 0;
	m_vProgressInfo.clear();

	BOOST_ASSERT(rOperationPlan.GetSubOperationsCount() != 0);
	if(rOperationPlan.GetSubOperationsCount() == 0)
		return;

	for(size_t stIndex = 0; stIndex < rOperationPlan.GetSubOperationsCount(); ++stIndex)
	{
		TSubTaskProgressInfoPtr spProgressInfo(boost::make_shared<TSubTaskProgressInfo>());
		m_vProgressInfo.push_back(boost::make_tuple(rOperationPlan.GetSubOperationAt(stIndex), rOperationPlan.GetEstimatedTimeAt(stIndex), spProgressInfo));
	}
}

void TTaskProgressInfo::GetProgressSnapshot(TProgressSnapshot& rSnapshot) const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);

	// current operation index
	rSnapshot.m_stCurrentOperation = m_stSubOperationIndex;

	// reset other stats
	rSnapshot.m_tCurrentSubTaskProgress.Clear();

	// whole progress
	rSnapshot.m_dCountTotalProgress = 0.0;
	rSnapshot.m_dSizeTotalProgress = 0.0;

	for(size_t stIndex = 0; stIndex < m_vProgressInfo.size(); ++stIndex)
	{
		const boost::tuple<ESubOperationType, double, TSubTaskProgressInfoPtr>& rProgressInfo = m_vProgressInfo.at(stIndex);
		if(stIndex < m_stSubOperationIndex)
		{
			// this operation is already finished, so assuming 100%
			rSnapshot.m_dCountTotalProgress += boost::get<1>(rProgressInfo);
			rSnapshot.m_dSizeTotalProgress += boost::get<1>(rProgressInfo);
		}
		else if(stIndex == m_stSubOperationIndex)
		{
			// this operation is in progress, so calculate percentages and store snapshot of current subtask progress info
			const TSubTaskProgressInfoPtr& spInfo = boost::get<2>(rProgressInfo);
			if(!spInfo)
				THROW(_T("Invalid pointer"), 0, 0, 0);

			rSnapshot.m_tCurrentSubTaskProgress.GetSnapshot(*spInfo);

			rSnapshot.m_dCountTotalProgress += spInfo->GetCountProgressInPercent() * boost::get<1>(rProgressInfo);
			rSnapshot.m_dSizeTotalProgress += spInfo->GetSizeProgressInPercent() * boost::get<1>(rProgressInfo);
		}
		else
			break;
	}
}

void TTaskProgressInfo::SetSubOperationIndex(size_t stSubOperationIndex)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_stSubOperationIndex = stSubOperationIndex;
}

size_t TTaskProgressInfo::GetSubOperationIndex() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_stSubOperationIndex;
}

void TTaskProgressInfo::IncreaseSubOperationIndex()
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	++m_stSubOperationIndex;
}

