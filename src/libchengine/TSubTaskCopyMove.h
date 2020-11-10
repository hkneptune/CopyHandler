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
/// @file  TSubTaskCopyMove.h
/// @date  2010/09/18
/// @brief Contains declarations of classes responsible for copy and move sub-operation.
// ============================================================================
#ifndef __TSUBTASKCOPYMOVE_H__
#define __TSUBTASKCOPYMOVE_H__

#include "TSubTaskBase.h"
#include "TBufferSizes.h"
#include "../liblogger/TLogger.h"
#include "TOverlappedMemoryPool.h"
#include "FeedbackManager.h"

namespace chengine
{
	class TOverlappedThreadPool;
	struct CUSTOM_COPY_PARAMS;

	class TDataBufferManager;
	class TSimpleDataBuffer;
	class TBufferSizes;
	class TFilesystemFileFeedbackWrapper;

	class LIBCHENGINE_API TSubTaskCopyMove : public TSubTaskBase
	{
	public:
		explicit TSubTaskCopyMove(TSubTaskContext& tSubTaskContext);

		void Reset() override;

		void InitBeforeExec() override;
		ESubOperationResult Exec() override;
		ESubOperationType GetSubOperationType() const override { return eSubOperation_Copying; }

		void Store(const serializer::ISerializerPtr& spSerializer) const override;
		void Load(const serializer::ISerializerPtr& spSerializer) override;

		void InitColumns(const serializer::ISerializerContainerPtr& spContainer) const;

		void GetStatsSnapshot(TSubTaskStatsSnapshotPtr& rStats) const override;

	private:
		TBufferSizes::EBufferType GetBufferIndex(const TBufferSizes& rBufferSizes, const TFileInfoPtr& spFileInfo);
		bool AdjustBufferIfNeeded(const TOverlappedMemoryPoolPtr& spBuffer, TBufferSizes& rBufferSizes, bool bForce = false);

		ESubOperationResult CustomCopyFileFB(const FeedbackManagerPtr& spFeedbackManager,
			TOverlappedThreadPool& rThreadPool,
			CUSTOM_COPY_PARAMS* pData);

		void AdjustProcessedSizeForSkip(const TFileInfoPtr& spSrcFileInfo);

	private:
#pragma warning(push)
#pragma warning(disable: 4251)
		TSubTaskStatsInfoPtr m_spSubTaskStats;
		logger::TLoggerPtr m_spLog;
#pragma warning(pop)
	};
}

#endif
