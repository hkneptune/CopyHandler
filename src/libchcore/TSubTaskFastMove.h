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
/// @file  TSubTaskFastMove.h
/// @date  2011/11/13
/// @brief Contains declarations of classes responsible for fast move subtask.
// ============================================================================
#ifndef __TSUBTASKFASTMOVE_H__
#define __TSUBTASKFASTMOVE_H__

#include "libchcore.h"
#include "TSubTaskBase.h"
#include "../liblogger/TLogger.h"

namespace chcore
{
	class TFileFiltersArray;

	///////////////////////////////////////////////////////////////////////////
	// TSubTaskFastMove

	class LIBCHCORE_API TSubTaskFastMove : public TSubTaskBase
	{
	public:
		explicit TSubTaskFastMove(TSubTaskContext& rContext);
		virtual ~TSubTaskFastMove();

		virtual void Reset();

		virtual void InitBeforeExec() override;
		virtual ESubOperationResult Exec(const IFeedbackHandlerPtr& spFeedbackHandler) override;
		virtual ESubOperationType GetSubOperationType() const override { return eSubOperation_FastMove; }

		virtual void Store(const ISerializerPtr& spSerializer) const;
		virtual void Load(const ISerializerPtr& spSerializer);

		void InitColumns(const ISerializerContainerPtr& spContainer) const;

		virtual void GetStatsSnapshot(TSubTaskStatsSnapshotPtr& rStats) const override;

	private:
#pragma warning(push)
#pragma warning(disable: 4251)
		TSubTaskStatsInfo m_tSubTaskStats;
		logger::TLoggerPtr m_spLog;
#pragma warning(pop)
	};
}

#endif
