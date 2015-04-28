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
#include "DataBuffer.h"
#include "CommonDataTypes.h"

BEGIN_CHCORE_NAMESPACE

class TLocalFilesystemFile;
typedef boost::shared_ptr<TFileInfo> TFileInfoPtr;
struct CUSTOM_COPY_PARAMS;

class TDataBufferManager;
class TSimpleDataBuffer;
class TBufferSizes;
class TOverlappedDataBufferQueue;
class TOverlappedDataBuffer;

class LIBCHCORE_API TSubTaskCopyMove : public TSubTaskBase
{
public:
	TSubTaskCopyMove(TSubTaskContext& tSubTaskContext);

	virtual void Reset();

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

	ESubOperationResult OpenSrcAndDstFilesFB(const IFeedbackHandlerPtr& spFeedbackHandler, CUSTOM_COPY_PARAMS* pData, TLocalFilesystemFile &fileSrc, TLocalFilesystemFile &fileDst, bool bNoBuffer, bool& bSkip);

	ESubOperationResult OpenSourceFileFB(const IFeedbackHandlerPtr& spFeedbackHandler, TLocalFilesystemFile& fileSrc, const TSmartPath& spPathToOpen, bool bNoBuffering);
	ESubOperationResult OpenDestinationFileFB(const IFeedbackHandlerPtr& spFeedbackHandler, TLocalFilesystemFile& fileDst, const TSmartPath& pathDstFile, bool bNoBuffering, const TFileInfoPtr& spSrcFileInfo, unsigned long long& ullSeekTo, bool& bFreshlyCreated);
	ESubOperationResult OpenExistingDestinationFileFB(const IFeedbackHandlerPtr& spFeedbackHandler, TLocalFilesystemFile& fileDst, const TSmartPath& pathDstFilePath, bool bNoBuffering);

	ESubOperationResult SetFilePointerFB(const IFeedbackHandlerPtr& spFeedbackHandler, TLocalFilesystemFile& file, long long llDistance, const TSmartPath& pathFile, bool& bSkip);
	ESubOperationResult SetEndOfFileFB(const IFeedbackHandlerPtr& spFeedbackHandler, TLocalFilesystemFile& file, const TSmartPath& pathFile, bool& bSkip);

	ESubOperationResult ReadFileFB(const IFeedbackHandlerPtr& spFeedbackHandler, TLocalFilesystemFile& file, TOverlappedDataBuffer& rBuffer, const TSmartPath& pathFile, bool& bSkip);
	ESubOperationResult HandleReadError(const IFeedbackHandlerPtr& spFeedbackHandler, TOverlappedDataBuffer& rBuffer, const TSmartPath& pathFile, bool& bSkip);

	ESubOperationResult WriteFileFB(const IFeedbackHandlerPtr& spFeedbackHandler, TLocalFilesystemFile& file, TOverlappedDataBuffer& rBuffer, const TSmartPath& pathFile, bool& bSkip);
	ESubOperationResult HandleWriteError(const IFeedbackHandlerPtr& spFeedbackHandler, TOverlappedDataBuffer& rBuffer, const TSmartPath& pathFile, bool& bSkip);
	ESubOperationResult CreateDirectoryFB(const IFeedbackHandlerPtr& spFeedbackHandler, const TSmartPath& pathDirectory);

	ESubOperationResult CheckForFreeSpaceFB(const IFeedbackHandlerPtr& spFeedbackHandler);

private:
#pragma warning(push)
#pragma warning(disable: 4251)
	TSubTaskStatsInfo m_tSubTaskStats;
#pragma warning(pop)
};

END_CHCORE_NAMESPACE

#endif
