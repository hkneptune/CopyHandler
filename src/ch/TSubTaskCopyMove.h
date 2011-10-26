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

namespace chcore
{
	class TDataBuffer;
	typedef boost::shared_ptr<TFileInfo> TFileInfoPtr;
}

struct CUSTOM_COPY_PARAMS;
class TLocalFilesystemFile;

class TSubTaskCopyMove : public TSubTaskBase
{
public:
	TSubTaskCopyMove(TSubTaskContext& tSubTaskContext);

	ESubOperationResult Exec();

private:
	bool GetMove(const chcore::TFileInfoPtr& spFileInfo);
	int GetBufferIndex(const chcore::TFileInfoPtr& spFileInfo);

	ESubOperationResult CustomCopyFileFB(CUSTOM_COPY_PARAMS* pData);

	ESubOperationResult OpenSourceFileFB(TLocalFilesystemFile& fileSrc, const chcore::TSmartPath& spPathToOpen, bool bNoBuffering);
	ESubOperationResult OpenDestinationFileFB(TLocalFilesystemFile& fileDst, const chcore::TSmartPath& pathDstFile, bool bNoBuffering, const chcore::TFileInfoPtr& spSrcFileInfo, unsigned long long& ullSeekTo, bool& bFreshlyCreated);
	ESubOperationResult OpenExistingDestinationFileFB(TLocalFilesystemFile& fileDst, const chcore::TSmartPath& pathDstFilePath, bool bNoBuffering);

	ESubOperationResult SetFilePointerFB(TLocalFilesystemFile& file, long long llDistance, const chcore::TSmartPath& pathFile, bool& bSkip);
	ESubOperationResult SetEndOfFileFB(TLocalFilesystemFile& file, const chcore::TSmartPath& pathFile, bool& bSkip);

	ESubOperationResult ReadFileFB(TLocalFilesystemFile& file, chcore::TDataBuffer& rBuffer, DWORD dwToRead, DWORD& rdwBytesRead, const chcore::TSmartPath& pathFile, bool& bSkip);
	ESubOperationResult WriteFileFB(TLocalFilesystemFile& file, chcore::TDataBuffer& rBuffer, DWORD dwToWrite, DWORD& rdwBytesWritten, const chcore::TSmartPath& pathFile, bool& bSkip);
	ESubOperationResult CreateDirectoryFB(const chcore::TSmartPath& pathDirectory);

	ESubOperationResult CheckForFreeSpaceFB();
};

#endif
