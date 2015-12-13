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
#ifndef __TFAKEFILESYSTEM_H__
#define __TFAKEFILESYSTEM_H__

#include "libchcore.h"
#include "TFakeFileDescription.h"
#include "IFilesystem.h"
#include "TFakeVolumeInfo.h"

namespace chcore
{
	class LIBCHCORE_API TFakeFilesystem : public IFilesystem
	{
	public:
		TFakeFilesystem();
		~TFakeFilesystem();

		// interface implementation
		virtual bool PathExist(const TSmartPath& strPath) override;
		virtual void SetFileDirectoryTime(const TSmartPath& pathFileDir, const TFileTime& ftCreationTime, const TFileTime& ftLastAccessTime, const TFileTime& ftLastWriteTime) override;
		virtual void SetAttributes(const TSmartPath& pathFileDir, DWORD dwAttributes) override;
		virtual void CreateDirectory(const TSmartPath& pathDirectory, bool bCreateFullPath) override;
		virtual void RemoveDirectory(const TSmartPath& pathFile) override;
		virtual void DeleteFile(const TSmartPath& pathFile) override;
		virtual void GetFileInfo(const TSmartPath& pathFile, TFileInfoPtr& rFileInfo, const TBasePathDataPtr& spBasePathData = TBasePathDataPtr()) override;
		virtual void FastMove(const TSmartPath& pathSource, const TSmartPath& pathDestination) override;
		virtual IFilesystemFindPtr CreateFinderObject(const TSmartPath& pathDir, const TSmartPath& pathMask) override;
		virtual IFilesystemFilePtr CreateFileObject(const TSmartPath& spFilename, bool bNoBuffering) override;
		virtual EPathsRelation GetPathsRelation(const TSmartPath& pathFirst, const TSmartPath& pathSecond) override;
		virtual void GetDynamicFreeSpace(const TSmartPath& path, unsigned long long& rullFree) override;

		// fake handling api
		void SetVolumeInfo(wchar_t wchVolumeLetter, file_size_t fsSize, UINT uiDriveType, DWORD dwPhysicalDiskNumber);
		void AddFSObjectDescription(const TFakeFileDescriptionPtr& spFileDesc);

	private:
		TFakeFileDescriptionPtr FindFileByLocation(const TSmartPath& rPath);
		TFakeFileDescriptionPtr CreateFSObjectByLocation(const TSmartPath& pathFSObject);
		static FILETIME GetCurrentFileTime();
		TFakeFileDescriptionPtr CreateFakeDirectory(const TSmartPath& pathDir);

	private:
#pragma warning(push)
#pragma warning(disable: 4251)
		std::list<TFakeFileDescriptionPtr> m_listFilesystemContent;
		std::map<wchar_t, TFakeVolumeInfo> m_mapVolumeInfo;
#pragma warning(pop)

		friend class TFakeFilesystemFile;
		friend class TFakeFilesystemFind;
	};
}

#endif
