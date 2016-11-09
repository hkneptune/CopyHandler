// ============================================================================
//  Copyright (C) 2001-2010 by Jozef Starosczyk
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
/// @file  TLocalFilesystem.h
/// @date  2011/03/24
/// @brief Contains class responsible for accessing local filesystem.
// ============================================================================
#ifndef __TLOCALFILESYSTEM_H__
#define __TLOCALFILESYSTEM_H__

#include "libchcore.h"
#include "TPath.h"
#include "TBasePathData.h"
#include "TFileInfoFwd.h"
#include "IFilesystem.h"
#include "..\liblogger\TLogger.h"

namespace chcore
{
	class TAutoFileHandle;
	class TLocalFilesystemFind;
	class TLocalFilesystemFile;
	class TSimpleDataBuffer;
	class TFileTime;
	class TOverlappedDataBuffer;

	class LIBCHCORE_API TLocalFilesystem : public IFilesystem
	{
	public:
		explicit TLocalFilesystem(const logger::TLogFileDataPtr& spLogFileData);
		virtual ~TLocalFilesystem();

		virtual bool PathExist(const TSmartPath& strPath) override;	// check for file or folder existence

		virtual void SetFileDirectoryTime(const TSmartPath& pathFileDir, const TFileTime& ftCreationTime, const TFileTime& ftLastAccessTime, const TFileTime& ftLastWriteTime) override;
		virtual void SetAttributes(const TSmartPath& pathFileDir, DWORD dwAttributes) override;

		virtual void CreateDirectory(const TSmartPath& pathDirectory, bool bCreateFullPath) override;
		virtual void RemoveDirectory(const TSmartPath& pathFile) override;
		virtual void DeleteFile(const TSmartPath& pathFile) override;

		virtual void GetFileInfo(const TSmartPath& pathFile, TFileInfoPtr& rFileInfo, const TBasePathDataPtr& spBasePathData = TBasePathDataPtr()) override;
		virtual void FastMove(const TSmartPath& pathSource, const TSmartPath& pathDestination) override;

		virtual IFilesystemFindPtr CreateFinderObject(const TSmartPath& pathDir, const TSmartPath& pathMask) override;
		virtual IFilesystemFilePtr CreateFileObject(IFilesystemFile::EOpenMode eMode, const TSmartPath& pathFile, bool bNoBuffering) override;

		virtual EPathsRelation GetPathsRelation(const TSmartPath& pathFirst, const TSmartPath& pathSecond) override;

		virtual void GetDynamicFreeSpace(const TSmartPath& path, unsigned long long& rullFree, unsigned long long& rullTotal) override;

	private:
		static TSmartPath PrependPathExtensionIfNeeded(const TSmartPath& pathInput);
		static UINT GetDriveData(const TSmartPath& spPath);
		DWORD GetPhysicalDiskNumber(wchar_t wchDrive);

	private:
#pragma warning(push)
#pragma warning(disable: 4251)
		std::map<wchar_t, DWORD> m_mapDriveLetterToPhysicalDisk;	// caches drive letter -> physical disk number
		boost::shared_mutex m_lockDriveLetterToPhysicalDisk;
		logger::TLoggerPtr m_spLog;
#pragma warning(pop)

		friend class TLocalFilesystemFind;
		friend class TLocalFilesystemFile;
	};
}

#endif
