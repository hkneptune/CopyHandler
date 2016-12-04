// ============================================================================
//  Copyright (C) 2001-2015 by Jozef Starosczyk
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
#ifndef __IFILESYSTEMFILE_H__
#define __IFILESYSTEMFILE_H__

#include "TPath.h"
#include "CommonDataTypes.h"

namespace chcore
{
	class TFileTime;
	class TOverlappedDataBuffer;
	class TFileInfo;

	class LIBCHCORE_API IFilesystemFile
	{
	public:
		enum EOpenMode
		{
			eMode_Read,
			eMode_Write
		};

	public:
		static const unsigned int MaxSectorSize = 4096;

	public:
		virtual ~IFilesystemFile();

		virtual void Truncate(file_size_t fsNewSize) = 0;

		virtual void ReadFile(TOverlappedDataBuffer& rBuffer) = 0;
		virtual void WriteFile(TOverlappedDataBuffer& rBuffer) = 0;
		virtual void FinalizeFile(TOverlappedDataBuffer& rBuffer) = 0;

		virtual void CancelIo() = 0;

		virtual void Close() = 0;

		virtual bool IsOpen() const = 0;
		virtual bool IsFreshlyCreated() = 0;

		virtual file_size_t GetFileSize() = 0;
		virtual void GetFileInfo(TFileInfo& tFileInfo) = 0;

		virtual TSmartPath GetFilePath() const = 0;
		virtual file_size_t GetSeekPositionForResume(file_size_t fsLastAvailablePosition) = 0;

		virtual void SetBasicInfo(DWORD dwAttributes, const TFileTime& ftCreationTime, const TFileTime& ftLastAccessTime, const TFileTime& ftLastWriteTime) = 0;
	};

	typedef std::shared_ptr<IFilesystemFile> IFilesystemFilePtr;
}

#endif
