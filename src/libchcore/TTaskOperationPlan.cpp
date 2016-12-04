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
/// @brief File contains implementation of class handling planning of the entire operation
// ============================================================================
#include "stdafx.h"
#include "TTaskOperationPlan.h"
#include <boost/thread/locks.hpp>

namespace chcore
{
	////////////////////////////////////////////////////////////////////////////
	// class TOperationPlan

	TOperationPlan::TOperationPlan() :
		m_eOperation(eOperation_None)
	{
	}

	TOperationPlan::TOperationPlan(const TOperationPlan& rSrc) :
		m_eOperation(eOperation_None)
	{
		SetOperationType(rSrc.GetOperationType());
	}

	TOperationPlan::~TOperationPlan()
	{
	}

	TOperationPlan& TOperationPlan::operator=(const TOperationPlan& rSrc)
	{
		if (this != &rSrc)
			SetOperationType(rSrc.GetOperationType());

		return *this;
	}

	void TOperationPlan::SetOperationType(EOperationType eOperation)
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);
		m_eOperation = eOperation;
	}

	EOperationType TOperationPlan::GetOperationType() const
	{
		boost::shared_lock<boost::shared_mutex> lock(m_lock);
		return m_eOperation;
	}
}
