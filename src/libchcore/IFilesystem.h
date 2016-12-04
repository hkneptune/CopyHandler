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
#ifndef __IFILESYSTEM_H__
#define __IFILESYSTEM_H__

#include "TPath.h"
#include "TFileInfoFwd.h"
#include "TBasePathDataFwd.h"
#include "TFileTime.h"
#include "IFilesystemFind.h"
#include "IFilesystemFile.h"

namespace chcore
{
	class LIBCHCORE_API IFilesystem
	{
	public:
		enum EPathsRelation
		{
			eRelation_Network,				// at least one of the paths is network one
			eRelation_CDRom,				// at least one of the paths relates to cd/dvd drive
			eRelation_TwoPhysicalDisks,		// paths lies on two separate physical disks
			eRelation_SinglePhysicalDisk,	// paths lies on the same physical disk
			eRelation_Other					// other type of relation
		};

	public:
		virtual ~IFilesystem();

		virtual bool PathExist(const TSmartPath& strPath) = 0;

		virtual void SetFileDirBasicInfo(const TSmartPath& pathFileDir, DWORD dwAttributes, const TFileTime& ftCreationTime, const TFileTime& ftLastAccessTime, const TFileTime& ftLastWriteTime) = 0;
		virtual void SetAttributes(const TSmartPath& pathFileDir, DWORD dwAttributes) = 0;

		virtual void CreateDirectory(const TSmartPath& pathDirectory, bool bCreateFullPath) = 0;
		virtual void RemoveDirectory(const TSmartPath& pathFile) = 0;
		virtual void DeleteFile(const TSmartPath& pathFile) = 0;

		virtual void GetFileInfo(const TSmartPath& pathFile, TFileInfoPtr& rFileInfo, const TBasePathDataPtr& spBasePathData = TBasePathDataPtr()) = 0;
		virtual void FastMove(const TSmartPath& pathSource, const TSmartPath& pathDestination) = 0;

		virtual IFilesystemFindPtr CreateFinderObject(const TSmartPath& pathDir, const TSmartPath& pathMask) = 0;
		virtual IFilesystemFilePtr CreateFileObject(IFilesystemFile::EOpenMode eMode, const TSmartPath& pathFile, bool bNoBuffering, bool bProtectReadOnlyFiles) = 0;

		virtual EPathsRelation GetPathsRelation(const TSmartPath& pathFirst, const TSmartPath& pathSecond) = 0;

		virtual void GetDynamicFreeSpace(const TSmartPath& path, unsigned long long& rullFree, unsigned long long& rullTotal) = 0;
	};

	typedef std::shared_ptr<IFilesystem> IFilesystemPtr;
}

#endif
