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
/// @file  TSubTaskArray.h
/// @date  2011/11/08
/// @brief File contain definition of a class handling a sequence of subtasks.
// ============================================================================
#ifndef __TSUBTASKSARRAY_H__
#define __TSUBTASKSARRAY_H__

#include "libchcore.h"
#include <boost/tuple/tuple.hpp>
#include "TSubTaskBase.h"
#include "TTaskLocalStats.h"
#include "TSubTaskArrayStatsSnapshot.h"

BEGIN_CHCORE_NAMESPACE

class TOperationPlan;
class TSubTaskContext;

class TReadBinarySerializer;
class TWriteBinarySerializer;

namespace details
{
	///////////////////////////////////////////////////////////////////////////
	// TTaskBasicProgressInfo

	class LIBCHCORE_API TTaskBasicProgressInfo
	{
	public:
		TTaskBasicProgressInfo();
		~TTaskBasicProgressInfo();

		void ResetProgress();

		void SetSubOperationIndex(size_t stSubOperationIndex);
		size_t GetSubOperationIndex() const;
		void IncreaseSubOperationIndex();

		void Serialize(TReadBinarySerializer& rSerializer);
		void Serialize(TWriteBinarySerializer& rSerializer) const;

	private:
		TTaskBasicProgressInfo(const TTaskBasicProgressInfo& rSrc);

	private:
		volatile size_t m_stSubOperationIndex;		 // index of sub-operation from TOperationDescription

#pragma warning(push)
#pragma warning(disable: 4251)
		mutable boost::shared_mutex m_lock;
#pragma warning(pop)
	};
}

///////////////////////////////////////////////////////////////////////////
// TTaskBasicProgressInfo
class LIBCHCORE_API TSubTasksArray
{
public:
	TSubTasksArray();
	TSubTasksArray(const TOperationPlan& rOperationPlan, TSubTaskContext& rSubTaskContext);
	~TSubTasksArray();

	void Init(const TOperationPlan& rOperationPlan, TSubTaskContext& rSubTaskContext);

	// Stats handling
	void GetStatsSnapshot(TSubTaskArrayStatsSnapshot& rSnapshot) const;
	void ResetProgressAndStats();

	// progress handling
	void SerializeProgress(TReadBinarySerializer& rSerializer);
	void SerializeProgress(TWriteBinarySerializer& rSerializer) const;

	TSubTaskBase::ESubOperationResult Execute(bool bRunOnlyEstimationSubTasks);

private:
	TSubTasksArray(const TSubTasksArray& rSrc);
	TSubTasksArray& operator=(const TSubTasksArray& rSrc);

	void AddSubTask(const TSubTaskBasePtr& spOperation, bool bIsPartOfEstimation);

private:
	TSubTaskContext* m_pSubTaskContext;

#pragma warning(push)
#pragma warning(disable: 4251)
	std::vector<std::pair<TSubTaskBasePtr, bool> > m_vSubTasks;	// pointer to the subtask object / part of the whole process / is this the part of estimation?

	details::TTaskBasicProgressInfo m_tProgressInfo;
#pragma warning(pop)

	friend class TTaskProcessingGuard;
};

END_CHCORE_NAMESPACE

#endif
