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

class CFileInfo;
typedef boost::shared_ptr<CFileInfo> CFileInfoPtr;

class TLocalFilesystemFind;

class TLocalFilesystem
{
public:
	static void GetDriveData(const chcore::TSmartPath& spPath, int *piDrvNum, UINT *puiDrvType);
	static bool PathExist(chcore::TSmartPath strPath);	// check for file or folder existence

	static bool SetFileDirectoryTime(const chcore::TSmartPath& pathFileDir, const FILETIME& ftCreationTime, const FILETIME& ftLastAccessTime, const FILETIME& ftLastWriteTime);
	static bool SetAttributes(const chcore::TSmartPath& pathFileDir, DWORD dwAttributes);

	static bool CreateDirectory(const chcore::TSmartPath& pathDirectory);
	static bool DeleteFile(const chcore::TSmartPath& pathFile);

	static bool GetFileInfo(const chcore::TSmartPath& pathFile, CFileInfoPtr& rFileInfo, size_t stSrcIndex = std::numeric_limits<size_t>::max(), const chcore::TPathContainer* pBasePaths = NULL);
	static bool FastMove(const chcore::TSmartPath& pathSource, const chcore::TSmartPath& pathDestination);

	static TLocalFilesystemFind CreateFinder(const chcore::TSmartPath& pathDir, const chcore::TSmartPath& pathMask);

private:
	static chcore::TSmartPath PrependPathExtensionIfNeeded(const chcore::TSmartPath& pathInput);

	friend class TLocalFilesystemFind;
};

class TLocalFilesystemFind
{
public:
	~TLocalFilesystemFind();

	bool FindNext(CFileInfoPtr& rspFileInfo);
	void Close();

private:
	TLocalFilesystemFind(const chcore::TSmartPath& pathDir, const chcore::TSmartPath& pathMask);

private:
	const chcore::TSmartPath m_pathDir;
	const chcore::TSmartPath m_pathMask;
	HANDLE m_hFind;

	friend class TLocalFilesystem;
};
#endif

