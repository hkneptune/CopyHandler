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
/// @file  TTaskProgressInfo.h
/// @date  2010/09/19
/// @brief File contains declarations of classes related to task progress reporting.
// ============================================================================
#ifndef __TTASKPROGRESSINFO_H__
#define __TTASKPROGRESSINFO_H__
/*

#include "TTaskOperationPlan.h"
#include "TSubTaskProgressInfo.h"

///////////////////////////////////////////////////////////////////////////
// TProgressSnapshot

class TProgressSnapshot
{
public:
	TProgressSnapshot();
	~TProgressSnapshot();

	// total progress (percentage) of the whole task
	double GetTaskCountProgress() const { return m_dCountTotalProgress; }
	double GetTaskSizeProgress() const { return m_dSizeTotalProgress; }

	// progress of current subtask
	const TSubTaskProgressInfo& GetCurrentSubTaskProgress() const { return m_tCurrentSubTaskProgress; }

	// index of current subtask in the operation plan
	size_t GetCurrentSubTaskIndex() const;

private:
	// percentage progress based on count and size of items
	double m_dCountTotalProgress;
	double m_dSizeTotalProgress;

	// copy of progress of current subtask
	TSubTaskProgressInfo m_tCurrentSubTaskProgress;

	// states which subtask is active at the moment
	size_t m_stCurrentOperation;

	friend class TTaskProgressInfo;
};

///////////////////////////////////////////////////////////////////////////
// TTaskProgressInfo

class TTaskProgressInfo
{
public:
	TTaskProgressInfo();
	~TTaskProgressInfo();

	// initializes the progress info 
	void CreateFromOperationPlan(const TOperationPlan& rOperationPlan);

	// progress operations
	void GetProgressSnapshot(TProgressSnapshot& rSnapshot) const;

	void SetSubOperationIndex(size_t stSubOperationIndex);
	void IncreaseSubOperationIndex();
	size_t GetSubOperationIndex() const;

	// retrieve reference to the subtask progress info to pass it to the subtask itself
	TSubTaskProgressInfo& GetProgressInfo(size_t stSubTaskIndex);

	// serialization
	template<class Archive>
	void load(Archive& ar, unsigned int / *uiVersion* /);

	template<class Archive>
	void save(Archive& ar, unsigned int / *uiVersion* /) const;

	BOOST_SERIALIZATION_SPLIT_MEMBER();

private:
	volatile size_t m_stSubOperationIndex;			// index of sub-operation from TOperationDescription

	std::vector<boost::tuple<ESubOperationType, double, TSubTaskProgressInfoPtr> > m_vProgressInfo;

	mutable boost::shared_mutex m_lock;
};

template<class Archive>
void TTaskProgressInfo::load(Archive& ar, unsigned int / *uiVersion* /)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);

	ar >> m_vProgressInfo;
	ar >> m_stSubOperationIndex;

	// note that m_stSubOperationIndex could be equal to m_vProgressInfo.size()
	// in case all subtasks has already finished
	if(m_stSubOperationIndex > m_vProgressInfo.size())
		THROW(_T("Corrupted progress data"), 0, 0, 0);
}

template<class Archive>
void TTaskProgressInfo::save(Archive& ar, unsigned int / *uiVersion* /) const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);

	ar << m_vProgressInfo;
	ar << m_stSubOperationIndex;
}*/

#endif // __TTASKPROGRESSINFO_H__