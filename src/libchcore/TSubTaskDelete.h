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
/// @file  TSubTaskDelete.h
/// @date  2010/09/18
/// @brief Contains declarations of classes responsible for delete sub-operation.
// ============================================================================
#ifndef __TSUBTASKDELETE_H__
#define __TSUBTASKDELETE_H__

#include "libchcore.h"
#include "TSubTaskBase.h"

namespace chcore
{
	///////////////////////////////////////////////////////////////////////////
	// TSubTaskDelete

	class LIBCHCORE_API TSubTaskDelete : public TSubTaskBase
	{
	public:
		TSubTaskDelete(TSubTaskContext& rContext);

		virtual void Reset();

		virtual void InitBeforeExec() override;
		virtual ESubOperationResult Exec(const IFeedbackHandlerPtr& spFeedbackHandler) override;
		virtual ESubOperationType GetSubOperationType() const override { return eSubOperation_Deleting; }

		virtual void Store(const ISerializerPtr& spSerializer) const;
		virtual void Load(const ISerializerPtr& spSerializer);

		void InitColumns(const ISerializerContainerPtr& spContainer) const;

		virtual void GetStatsSnapshot(TSubTaskStatsSnapshotPtr& spStats) const;

	private:
#pragma warning(push)
#pragma warning(disable: 4251)
		TSubTaskStatsInfo m_tSubTaskStats;
#pragma warning(pop)
	};
}

#endif
