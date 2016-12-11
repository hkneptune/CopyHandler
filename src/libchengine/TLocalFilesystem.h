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

#include "../libchcore/TPath.h"
#include "TBasePathData.h"
#include "TFileInfoFwd.h"
#include "IFilesystem.h"
#include "../liblogger/TLogger.h"

namespace chcore
{
	class TFileTime;
}

namespace chengine
{
	class TLocalFilesystemFind;
	class TLocalFilesystemFile;
	class TSimpleDataBuffer;
	class TOverlappedDataBuffer;

	class LIBCHENGINE_API TLocalFilesystem : public IFilesystem
	{
	public:
		explicit TLocalFilesystem(const logger::TLogFileDataPtr& spLogFileData);
		TLocalFilesystem(const TLocalFilesystem&) = delete;

		virtual ~TLocalFilesystem();

		TLocalFilesystem& operator=(const TLocalFilesystem&) = delete;

		bool PathExist(const chcore::TSmartPath& strPath) override;	// check for file or folder existence

		void SetFileDirBasicInfo(const chcore::TSmartPath& pathFileDir, DWORD dwAttributes, const chcore::TFileTime& ftCreationTime, const chcore::TFileTime& ftLastAccessTime, const chcore::TFileTime& ftLastWriteTime) override;
		void SetAttributes(const chcore::TSmartPath& pathFileDir, DWORD dwAttributes) override;

		void CreateDirectory(const chcore::TSmartPath& pathDirectory, bool bCreateFullPath) override;
		void RemoveDirectory(const chcore::TSmartPath& pathFile) override;
		void DeleteFile(const chcore::TSmartPath& pathFile) override;

		void GetFileInfo(const chcore::TSmartPath& pathFile, TFileInfoPtr& rFileInfo, const TBasePathDataPtr& spBasePathData = TBasePathDataPtr()) override;
		void FastMove(const chcore::TSmartPath& pathSource, const chcore::TSmartPath& pathDestination) override;

		IFilesystemFindPtr CreateFinderObject(const chcore::TSmartPath& pathDir, const chcore::TSmartPath& pathMask) override;
		IFilesystemFilePtr CreateFileObject(IFilesystemFile::EOpenMode eMode, const chcore::TSmartPath& pathFile, bool bNoBuffering, bool bProtectReadOnlyFiles) override;

		EPathsRelation GetPathsRelation(const chcore::TSmartPath& pathFirst, const chcore::TSmartPath& pathSecond) override;

		void GetDynamicFreeSpace(const chcore::TSmartPath& path, unsigned long long& rullFree, unsigned long long& rullTotal) override;

	private:
		static chcore::TSmartPath PrependPathExtensionIfNeeded(const chcore::TSmartPath& pathInput);
		static UINT GetDriveData(const chcore::TSmartPath& spPath);
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
