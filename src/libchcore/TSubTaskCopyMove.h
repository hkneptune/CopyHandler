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

BEGIN_CHCORE_NAMESPACE

class TDataBuffer;
class TLocalFilesystemFile;
typedef boost::shared_ptr<TFileInfo> TFileInfoPtr;
struct CUSTOM_COPY_PARAMS;
class TReadBinarySerializer;
class TWriteBinarySerializer;

namespace details
{
	///////////////////////////////////////////////////////////////////////////
	// TCopyMoveProgressInfo

	class TCopyMoveProgressInfo : public TSubTaskProgressInfo
	{
	public:
		TCopyMoveProgressInfo();
		virtual ~TCopyMoveProgressInfo();

		virtual void Serialize(TReadBinarySerializer& rSerializer);
		virtual void Serialize(TWriteBinarySerializer& rSerializer) const;

		virtual void ResetProgress();

		// file being processed
		void SetCurrentIndex(size_t stIndex);
		void IncreaseCurrentIndex();
		size_t GetCurrentIndex() const;

		// part of file being processed
		void SetCurrentFileProcessedSize(unsigned long long ullSize);
		unsigned long long GetCurrentFileProcessedSize() const;
		void IncreaseCurrentFileProcessedSize(unsigned long long ullSizeToAdd);

	private:
		volatile size_t m_stCurrentIndex;
		volatile unsigned long long m_ullCurrentFileProcessedSize;	// count of bytes processed for current file
		mutable boost::shared_mutex m_lock;
	};
}

class LIBCHCORE_API TSubTaskCopyMove : public TSubTaskBase
{
public:
	TSubTaskCopyMove(TSubTaskContext& tSubTaskContext);

	virtual void Reset();

	virtual ESubOperationResult Exec();
	virtual ESubOperationType GetSubOperationType() const { return eSubOperation_Copying; }

	virtual TSubTaskProgressInfo& GetProgressInfo() { return m_tProgressInfo; }
	virtual void GetStatsSnapshot(TSubTaskStatsSnapshot& rStats) const;

private:
	int GetBufferIndex(const TFileInfoPtr& spFileInfo);

	ESubOperationResult CustomCopyFileFB(CUSTOM_COPY_PARAMS* pData);

	ESubOperationResult OpenSourceFileFB(TLocalFilesystemFile& fileSrc, const TSmartPath& spPathToOpen, bool bNoBuffering);
	ESubOperationResult OpenDestinationFileFB(TLocalFilesystemFile& fileDst, const TSmartPath& pathDstFile, bool bNoBuffering, const TFileInfoPtr& spSrcFileInfo, unsigned long long& ullSeekTo, bool& bFreshlyCreated);
	ESubOperationResult OpenExistingDestinationFileFB(TLocalFilesystemFile& fileDst, const TSmartPath& pathDstFilePath, bool bNoBuffering);

	ESubOperationResult SetFilePointerFB(TLocalFilesystemFile& file, long long llDistance, const TSmartPath& pathFile, bool& bSkip);
	ESubOperationResult SetEndOfFileFB(TLocalFilesystemFile& file, const TSmartPath& pathFile, bool& bSkip);

	ESubOperationResult ReadFileFB(TLocalFilesystemFile& file, TDataBuffer& rBuffer, DWORD dwToRead, DWORD& rdwBytesRead, const TSmartPath& pathFile, bool& bSkip);
	ESubOperationResult WriteFileFB(TLocalFilesystemFile& file, TDataBuffer& rBuffer, DWORD dwToWrite, DWORD& rdwBytesWritten, const TSmartPath& pathFile, bool& bSkip);
	ESubOperationResult CreateDirectoryFB(const TSmartPath& pathDirectory);

	ESubOperationResult CheckForFreeSpaceFB();

private:
#pragma warning(push)
#pragma warning(disable: 4251)
	details::TCopyMoveProgressInfo m_tProgressInfo;
	TSubTaskStatsInfo m_tSubTaskStats;
#pragma warning(pop)
};

END_CHCORE_NAMESPACE

#endif
