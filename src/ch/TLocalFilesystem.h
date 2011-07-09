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

class TAutoFileHandle;
class TLocalFilesystemFind;
class TLocalFilesystemFile;
class CDataBuffer;

class TLocalFilesystem
{
public:
	static void GetDriveData(const chcore::TSmartPath& spPath, int *piDrvNum, UINT *puiDrvType);
	static bool PathExist(chcore::TSmartPath strPath);	// check for file or folder existence

	static bool SetFileDirectoryTime(const chcore::TSmartPath& pathFileDir, const FILETIME& ftCreationTime, const FILETIME& ftLastAccessTime, const FILETIME& ftLastWriteTime);
	static bool SetAttributes(const chcore::TSmartPath& pathFileDir, DWORD dwAttributes);

	static bool CreateDirectory(const chcore::TSmartPath& pathDirectory, bool bCreateFullPath);
	static bool RemoveDirectory(const chcore::TSmartPath& pathFile);
	static bool DeleteFile(const chcore::TSmartPath& pathFile);

	static bool GetFileInfo(const chcore::TSmartPath& pathFile, CFileInfoPtr& rFileInfo, size_t stSrcIndex = std::numeric_limits<size_t>::max(), const chcore::TPathContainer* pBasePaths = NULL);
	static bool FastMove(const chcore::TSmartPath& pathSource, const chcore::TSmartPath& pathDestination);

	static TLocalFilesystemFind CreateFinderObject(const chcore::TSmartPath& pathDir, const chcore::TSmartPath& pathMask);
	static TLocalFilesystemFile CreateFileObject();

private:
	static chcore::TSmartPath PrependPathExtensionIfNeeded(const chcore::TSmartPath& pathInput);

	friend class TLocalFilesystemFind;
	friend class TLocalFilesystemFile;
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
	chcore::TSmartPath m_pathDir;
	chcore::TSmartPath m_pathMask;
	HANDLE m_hFind;

	friend class TLocalFilesystem;
};

class TLocalFilesystemFile
{
public:
	~TLocalFilesystemFile();

	bool OpenExistingForReading(const chcore::TSmartPath& pathFile, bool bNoBuffering);
	bool CreateNewForWriting(const chcore::TSmartPath& pathFile, bool bNoBuffering);
	bool OpenExistingForWriting(const chcore::TSmartPath& pathFile, bool bNoBuffering);

	bool SetFilePointer(long long llNewPos, DWORD dwMoveMethod);
	bool SetEndOfFile();

	bool ReadFile(CDataBuffer& rBuffer, DWORD dwToRead, DWORD& rdwBytesRead);
	bool WriteFile(CDataBuffer& rBuffer, DWORD dwToWrite, DWORD& rdwBytesWritten);

	bool IsOpen() const { return m_hFile != INVALID_HANDLE_VALUE; }

	void Close();

private:
	TLocalFilesystemFile();

private:
	chcore::TSmartPath m_pathFile;
	HANDLE m_hFile;

	friend class TLocalFilesystem;
};
#endif
