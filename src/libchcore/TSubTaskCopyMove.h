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

#include "libchcore.h"
#include "TSubTaskBase.h"
#include "CommonDataTypes.h"
#include "TBufferSizes.h"
#include "IFilesystemFile.h"
#include "../liblogger/TLogger.h"

namespace chcore
{
	typedef std::shared_ptr<TFileInfo> TFileInfoPtr;
	struct CUSTOM_COPY_PARAMS;

	class TDataBufferManager;
	class TSimpleDataBuffer;
	class TBufferSizes;
	class TOverlappedDataBufferQueue;
	class TOverlappedDataBuffer;
	class TFilesystemFileFeedbackWrapper;

	class LIBCHCORE_API TSubTaskCopyMove : public TSubTaskBase
	{
	public:
		explicit TSubTaskCopyMove(TSubTaskContext& tSubTaskContext);

		virtual void Reset();

		virtual void InitBeforeExec() override;
		virtual ESubOperationResult Exec(const IFeedbackHandlerPtr& spFeedbackHandler) override;
		virtual ESubOperationType GetSubOperationType() const override { return eSubOperation_Copying; }

		virtual void Store(const ISerializerPtr& spSerializer) const;
		virtual void Load(const ISerializerPtr& spSerializer);

		void InitColumns(const ISerializerContainerPtr& spContainer) const;

		virtual void GetStatsSnapshot(TSubTaskStatsSnapshotPtr& rStats) const;

	private:
		TBufferSizes::EBufferType GetBufferIndex(const TBufferSizes& rBufferSizes, const TFileInfoPtr& spFileInfo);
		bool AdjustBufferIfNeeded(TOverlappedDataBufferQueue& rBuffer, TBufferSizes& rBufferSizes, bool bForce = false);

		ESubOperationResult CustomCopyFileFB(const IFeedbackHandlerPtr& spFeedbackHandler, CUSTOM_COPY_PARAMS* pData);

		ESubOperationResult OpenSrcAndDstFilesFB(TFilesystemFileFeedbackWrapper& rFileFBWrapper, CUSTOM_COPY_PARAMS* pData,
			const IFilesystemFilePtr& spFileSrc, const IFilesystemFilePtr& spFileDst, bool& bSkip);

		ESubOperationResult HandleReadError(const IFeedbackHandlerPtr& spFeedbackHandler, TOverlappedDataBuffer& rBuffer,
			const TSmartPath& pathFile, bool& bSkip);
		ESubOperationResult HandleWriteError(const IFeedbackHandlerPtr& spFeedbackHandler, TOverlappedDataBuffer& rBuffer,
			const TSmartPath& pathFile, bool& bSkip);

		void AdjustProcessedSize(file_size_t fsWritten, const TFileInfoPtr& spSrcFileInfo, const IFilesystemFilePtr& spSrcFile);
		void AdjustFinalSize(const TFileInfoPtr& spSrcFileInfo, const IFilesystemFilePtr& spSrcFile);
		void AdjustProcessedSizeForSkip(const TFileInfoPtr& spSrcFileInfo);

	private:
#pragma warning(push)
#pragma warning(disable: 4251)
		TSubTaskStatsInfo m_tSubTaskStats;
		TLoggerPtr m_spLog;
#pragma warning(pop)
	};
}

#endif
