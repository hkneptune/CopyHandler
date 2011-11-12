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

BEGIN_CHCORE_NAMESPACE

class TOperationPlan;
class TSubTaskContext;

class LIBCHCORE_API TSubTasksArray
{
public:
	TSubTasksArray(const TOperationPlan& rOperationPlan, TSubTaskContext& rSubTaskContext);
	~TSubTasksArray();

	TSubTaskBase::ESubOperationResult Execute(bool bRunOnlyEstimationSubTasks);

private:
	TSubTasksArray(const TSubTasksArray& rSrc);
	TSubTasksArray& operator=(const TSubTasksArray& rSrc);

private:
#pragma warning(push)
#pragma warning(disable: 4251)
	std::vector<boost::tuples::tuple<TSubTaskBasePtr, double, bool> > m_vSubTasks;	// pointer to the subtask object / part of the whole process / is this the part of estimation?
#pragma warning(pop)
	TSubTaskContext& m_rSubTaskContext;
};

END_CHCORE_NAMESPACE

#endif
