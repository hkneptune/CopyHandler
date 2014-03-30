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

BEGIN_CHCORE_NAMESPACE

class TFileInfo;
typedef boost::shared_ptr<TFileInfo> TFileInfoPtr;

class TAutoFileHandle;
class TLocalFilesystemFind;
class TLocalFilesystemFile;
class TSimpleDataBuffer;
class TModPathContainer;

class LIBCHCORE_API TLocalFilesystem
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
	static bool PathExist(TSmartPath strPath);	// check for file or folder existence

	static bool SetFileDirectoryTime(const TSmartPath& pathFileDir, const FILETIME& ftCreationTime, const FILETIME& ftLastAccessTime, const FILETIME& ftLastWriteTime);
	static bool SetAttributes(const TSmartPath& pathFileDir, DWORD dwAttributes);

	static bool CreateDirectory(const TSmartPath& pathDirectory, bool bCreateFullPath);
	static bool RemoveDirectory(const TSmartPath& pathFile);
	static bool DeleteFile(const TSmartPath& pathFile);

	static bool GetFileInfo(const TSmartPath& pathFile, TFileInfoPtr& rFileInfo, size_t stSrcIndex = std::numeric_limits<size_t>::max(), const TModPathContainer* pBasePaths = NULL);
	static bool FastMove(const TSmartPath& pathSource, const TSmartPath& pathDestination);

	static TLocalFilesystemFind CreateFinderObject(const TSmartPath& pathDir, const TSmartPath& pathMask);
	static TLocalFilesystemFile CreateFileObject();

	EPathsRelation GetPathsRelation(const TSmartPath& pathFirst, const TSmartPath& pathSecond);

	bool GetDynamicFreeSpace(const TSmartPath& path, unsigned long long& rullFree);

private:
	static TSmartPath PrependPathExtensionIfNeeded(const TSmartPath& pathInput);
	static UINT GetDriveData(const TSmartPath& spPath);
	DWORD  GetPhysicalDiskNumber(wchar_t wchDrive);

private:
#pragma warning(push)
#pragma warning(disable: 4251)
	std::map<wchar_t, DWORD> m_mapDriveLetterToPhysicalDisk;	// caches drive letter -> physical disk number
	boost::shared_mutex m_lockDriveLetterToPhysicalDisk;
#pragma warning(pop)

	friend class TLocalFilesystemFind;
	friend class TLocalFilesystemFile;
};

class LIBCHCORE_API TLocalFilesystemFind
{
public:
	~TLocalFilesystemFind();

	bool FindNext(TFileInfoPtr& rspFileInfo);
	void Close();

private:
	TLocalFilesystemFind(const TSmartPath& pathDir, const TSmartPath& pathMask);

private:
	TSmartPath m_pathDir;
	TSmartPath m_pathMask;
	HANDLE m_hFind;

	friend class TLocalFilesystem;
};

class LIBCHCORE_API TLocalFilesystemFile
{
public:
	~TLocalFilesystemFile();

	bool OpenExistingForReading(const TSmartPath& pathFile, bool bNoBuffering);
	bool CreateNewForWriting(const TSmartPath& pathFile, bool bNoBuffering);
	bool OpenExistingForWriting(const TSmartPath& pathFile, bool bNoBuffering);

	bool SetFilePointer(long long llNewPos, DWORD dwMoveMethod);
	bool SetEndOfFile();

	bool ReadFile(TSimpleDataBuffer& rBuffer, DWORD dwToRead, DWORD& rdwBytesRead);
	bool WriteFile(TSimpleDataBuffer& rBuffer, DWORD dwToWrite, DWORD& rdwBytesWritten);

	bool IsOpen() const { return m_hFile != INVALID_HANDLE_VALUE; }

	void Close();

private:
	TLocalFilesystemFile();

private:
	TSmartPath m_pathFile;
	HANDLE m_hFile;

	friend class TLocalFilesystem;
};

END_CHCORE_NAMESPACE

#endif
