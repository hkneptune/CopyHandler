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
/// @file  TTaskOperationPlan.cpp
/// @date  2010/09/18
/// @brief File contains class handling planning of the entire operation.
// ============================================================================
#ifndef __TTASKOPERATIONPLAN_H__
#define __TTASKOPERATIONPLAN_H__

#include "libchcore.h"
#include "EOperationTypes.h"
#include "ESubTaskTypes.h"

namespace chcore
{
	///////////////////////////////////////////////////////////////////////////
	// TOperationPlan

	// class describes the sub-operations to be performed
	class LIBCHCORE_API TOperationPlan
	{
	public:
		TOperationPlan();
		TOperationPlan(const TOperationPlan& rSrc);
		~TOperationPlan();

		TOperationPlan& operator=(const TOperationPlan& rSrc);

		void SetOperationType(EOperationType eOperation);
		EOperationType GetOperationType() const;

	private:
		EOperationType m_eOperation;
#pragma warning(push)
#pragma warning(disable: 4251)
		mutable boost::shared_mutex m_lock;
#pragma warning(pop)
	};
}

#endif
