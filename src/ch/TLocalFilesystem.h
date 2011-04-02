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

class TLocalFilesystem
{
public:
	static void GetDriveData(const chcore::TSmartPath& spPath, int *piDrvNum, UINT *puiDrvType);
	static bool PathExist(chcore::TSmartPath strPath);	// check for file or folder existence
	static bool SetFileDirectoryTime(LPCTSTR lpszName, const FILETIME& ftCreationTime, const FILETIME& ftLastAccessTime, const FILETIME& ftLastWriteTime);
	static bool CreateDirectory(const chcore::TSmartPath& pathDirectory);

	static bool GetFileInfo(const chcore::TSmartPath& pathFile, CFileInfoPtr& rFileInfo, size_t stSrcIndex = std::numeric_limits<size_t>::max(), const chcore::TPathContainer* pBasePaths = NULL);
};

#endif

