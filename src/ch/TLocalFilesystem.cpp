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
/// @file  TLocalFilesystem.cpp
/// @date  2011/03/24
/// @brief 
// ============================================================================
#include "stdafx.h"
#include "TLocalFilesystem.h"
#include "TAutoHandles.h"
#include "FileInfo.h"
#include "DataBuffer.h"
#include <boost\algorithm\string\case_conv.hpp>
#include "FileSupport.h"

void TLocalFilesystem::GetDriveData(const chcore::TSmartPath& spPath, int* piDrvNum, UINT* puiDrvType)
{
	chcore::TSmartPath pathDrive = spPath.GetDrive();

	if(!spPath.IsNetworkPath())
	{
		std::wstring wstrDrive = spPath.ToWString();

		if(wstrDrive.empty())
		{
			if(piDrvNum)
				*piDrvNum = -1;

			if(puiDrvType)
				*puiDrvType = DRIVE_UNKNOWN;
		}
		else
		{
			// disk number
			if(piDrvNum)
			{
				boost::to_upper(wstrDrive);
				*piDrvNum = wstrDrive.at(0) - _T('A');
			}

			// disk type
			if(puiDrvType)
			{
				pathDrive.AppendSeparatorIfDoesNotExist();

				*puiDrvType = GetDriveType(pathDrive.ToString());
				if(*puiDrvType == DRIVE_NO_ROOT_DIR)
					*puiDrvType = DRIVE_UNKNOWN;
			}
		}
	}
	else
	{
		// network path
		if(piDrvNum)
			*piDrvNum = -1;

		if(puiDrvType)
			*puiDrvType = DRIVE_REMOTE;
	}
}

bool TLocalFilesystem::PathExist(chcore::TSmartPath pathToCheck)
{
	WIN32_FIND_DATA fd;

	// search by exact name
	HANDLE hFind = FindFirstFile(PrependPathExtensionIfNeeded(pathToCheck).ToString(), &fd);
	if(hFind != INVALID_HANDLE_VALUE)
	{
		FindClose(hFind);
		return true;
	}

	// another try (add '\\' if needed and '*' for marking that we look for ie. c:\*
	// instead of c:\, which would never be found prev. way)
	pathToCheck.AppendIfNotExists(_T("*"), false);

	hFind = FindFirstFile(PrependPathExtensionIfNeeded(pathToCheck).ToString(), &fd);
	if(hFind != INVALID_HANDLE_VALUE)
	{
		::FindClose(hFind);
		return true;
	}
	else
		return false;
}

bool TLocalFilesystem::SetFileDirectoryTime(const chcore::TSmartPath& pathFileDir, const FILETIME& ftCreationTime, const FILETIME& ftLastAccessTime, const FILETIME& ftLastWriteTime)
{
	TAutoFileHandle hFile = CreateFile(PrependPathExtensionIfNeeded(pathFileDir).ToString(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS, NULL);
	if(hFile == INVALID_HANDLE_VALUE)
		return false;

	BOOL bResult = SetFileTime(hFile, &ftCreationTime, &ftLastAccessTime, &ftLastWriteTime);

	if(!hFile.Close())
		return false;

	return bResult != FALSE;
}

bool TLocalFilesystem::SetAttributes(const chcore::TSmartPath& pathFileDir, DWORD dwAttributes)
{
	return ::SetFileAttributes(PrependPathExtensionIfNeeded(pathFileDir).ToString(), dwAttributes) != FALSE;
}

bool TLocalFilesystem::CreateDirectory(const chcore::TSmartPath& pathDirectory, bool bCreateFullPath)
{
	if(!bCreateFullPath)
		return ::CreateDirectory(PrependPathExtensionIfNeeded(pathDirectory).ToString(), NULL) != FALSE;
	else
	{
		std::vector<chcore::TSmartPath> vComponents;
		pathDirectory.SplitPath(vComponents);

		chcore::TSmartPath pathToTest;
		BOOST_FOREACH(const chcore::TSmartPath& pathComponent, vComponents)
		{
			pathToTest += pathComponent;
			// try to create subsequent paths
			if(!pathToTest.IsDrive() && !pathToTest.IsServerName())
			{
				// try to create the specified path
				BOOL bRes = ::CreateDirectory(PrependPathExtensionIfNeeded(pathToTest).ToString(), NULL);
				if(!bRes && GetLastError() != ERROR_ALREADY_EXISTS)
					return false;
			}
		}
	}

	return true;
}

bool TLocalFilesystem::RemoveDirectory(const chcore::TSmartPath& pathFile)
{
	return ::RemoveDirectory(PrependPathExtensionIfNeeded(pathFile).ToString()) != FALSE;
}

bool TLocalFilesystem::DeleteFile(const chcore::TSmartPath& pathFile)
{
	return ::DeleteFile(PrependPathExtensionIfNeeded(pathFile).ToString()) != FALSE;
}

bool TLocalFilesystem::GetFileInfo(const chcore::TSmartPath& pathFile, CFileInfoPtr& rFileInfo, size_t stSrcIndex, const chcore::TPathContainer* pBasePaths)
{
	if(!rFileInfo)
		THROW(_T("Invalid argument"), 0, 0, 0);

	WIN32_FIND_DATA wfd;
	HANDLE hFind = FindFirstFile(PrependPathExtensionIfNeeded(pathFile).ToString(), &wfd);

	if(hFind != INVALID_HANDLE_VALUE)
	{
		FindClose(hFind);

		// new instance of path to accomodate the corrected path (i.e. input path might have lower case names, but we'd like to
		// preserve the original case contained in the filesystem)
		chcore::TSmartPath pathNew(pathFile);
		pathNew.DeleteFileName();

		// copy data from W32_F_D
		rFileInfo->Init(pathNew + chcore::PathFromString(wfd.cFileName), stSrcIndex, pBasePaths,
			wfd.dwFileAttributes, (((ULONGLONG) wfd.nFileSizeHigh) << 32) + wfd.nFileSizeLow, wfd.ftCreationTime,
			wfd.ftLastAccessTime, wfd.ftLastWriteTime, 0);

		return true;
	}
	else
	{
		FILETIME fi = { 0, 0 };
		rFileInfo->Init(chcore::TSmartPath(), std::numeric_limits<size_t>::max(), NULL, (DWORD)-1, 0, fi, fi, fi, 0);
		return false;
	}
}

bool TLocalFilesystem::FastMove(const chcore::TSmartPath& pathSource, const chcore::TSmartPath& pathDestination)
{
	return ::MoveFile(PrependPathExtensionIfNeeded(pathSource).ToString(), PrependPathExtensionIfNeeded(pathDestination).ToString()) != FALSE;
}

TLocalFilesystemFind TLocalFilesystem::CreateFinderObject(const chcore::TSmartPath& pathDir, const chcore::TSmartPath& pathMask)
{
	return TLocalFilesystemFind(pathDir, pathMask);
}

TLocalFilesystemFile TLocalFilesystem::CreateFileObject()
{
	return TLocalFilesystemFile();
}

chcore::TSmartPath TLocalFilesystem::PrependPathExtensionIfNeeded(const chcore::TSmartPath& pathInput)
{
	if(pathInput.GetLength() >= 248)
		return chcore::PathFromString(_T("\\\\?\\")) + pathInput;
	else
		return pathInput;
}

TLocalFilesystemFind::TLocalFilesystemFind(const chcore::TSmartPath& pathDir, const chcore::TSmartPath& pathMask) :
	m_pathDir(pathDir),
	m_pathMask(pathMask),
	m_hFind(INVALID_HANDLE_VALUE)
{
}

TLocalFilesystemFind::~TLocalFilesystemFind()
{
	Close();
}

bool TLocalFilesystemFind::FindNext(CFileInfoPtr& rspFileInfo)
{
	WIN32_FIND_DATA wfd;
	chcore::TSmartPath pathCurrent = m_pathDir + m_pathMask;

	// Iterate through dirs & files
	bool bContinue = true;
	if(m_hFind != INVALID_HANDLE_VALUE)
		bContinue = (FindNextFile(m_hFind, &wfd) != FALSE);
	else
	{
		m_hFind = FindFirstFile(TLocalFilesystem::PrependPathExtensionIfNeeded(pathCurrent).ToString(), &wfd);	// in this case we always continue
		bContinue = (m_hFind != INVALID_HANDLE_VALUE);
	}
	if(bContinue)
	{
		do
		{
			if(!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				rspFileInfo->Init(m_pathDir + chcore::PathFromString(wfd.cFileName), wfd.dwFileAttributes, (((ULONGLONG) wfd.nFileSizeHigh) << 32) + wfd.nFileSizeLow, wfd.ftCreationTime,
					wfd.ftLastAccessTime, wfd.ftLastWriteTime, 0);
				return true;
			}
			else if(wfd.cFileName[0] != _T('.') || (wfd.cFileName[1] != _T('\0') && (wfd.cFileName[1] != _T('.') || wfd.cFileName[2] != _T('\0'))))
			{
				// Add directory itself
				rspFileInfo->Init(m_pathDir + chcore::PathFromString(wfd.cFileName),
					wfd.dwFileAttributes, (((ULONGLONG) wfd.nFileSizeHigh) << 32) + wfd.nFileSizeLow, wfd.ftCreationTime,
					wfd.ftLastAccessTime, wfd.ftLastWriteTime, 0);
				return true;
			}
		}
		while(::FindNextFile(m_hFind, &wfd));

		Close();
	}

	return false;
}

void TLocalFilesystemFind::Close()
{
	if(m_hFind != INVALID_HANDLE_VALUE)
		FindClose(m_hFind);
	m_hFind = INVALID_HANDLE_VALUE;
}

TLocalFilesystemFile::TLocalFilesystemFile() :
	m_hFile(INVALID_HANDLE_VALUE),
	m_pathFile()
{
}

TLocalFilesystemFile::~TLocalFilesystemFile()
{
	Close();
}

bool TLocalFilesystemFile::OpenExistingForReading(const chcore::TSmartPath& pathFile, bool bNoBuffering)
{
	Close();

	m_hFile = ::CreateFile(TLocalFilesystem::PrependPathExtensionIfNeeded(pathFile).ToString(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN | (bNoBuffering ? FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH : 0), NULL);
	if(m_hFile == INVALID_HANDLE_VALUE)
		return false;
	return true;
}

bool TLocalFilesystemFile::CreateNewForWriting(const chcore::TSmartPath& pathFile, bool bNoBuffering)
{
	Close();

	m_hFile = ::CreateFile(TLocalFilesystem::PrependPathExtensionIfNeeded(pathFile).ToString(), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN | (bNoBuffering ? FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH : 0), NULL);
	if(m_hFile == INVALID_HANDLE_VALUE)
		return false;
	return true;
}

bool TLocalFilesystemFile::OpenExistingForWriting(const chcore::TSmartPath& pathFile, bool bNoBuffering)
{
	Close();

	m_hFile = CreateFile(TLocalFilesystem::PrependPathExtensionIfNeeded(pathFile).ToString(), GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN | (bNoBuffering ? FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH : 0), NULL);
	if(m_hFile == INVALID_HANDLE_VALUE)
		return false;
	return true;
}

bool TLocalFilesystemFile::SetFilePointer(long long llNewPos, DWORD dwMoveMethod)
{
	if(!IsOpen())
		return false;

	return (SetFilePointer64(m_hFile, llNewPos, dwMoveMethod) != -1);
}

bool TLocalFilesystemFile::SetEndOfFile()
{
	if(!IsOpen())
		return false;

	return ::SetEndOfFile(m_hFile) != FALSE;
}

bool TLocalFilesystemFile::ReadFile(CDataBuffer& rBuffer, DWORD dwToRead, DWORD& rdwBytesRead)
{
	if(!IsOpen())
		return false;

	return ::ReadFile(m_hFile, rBuffer, dwToRead, &rdwBytesRead, NULL) != FALSE;
}

bool TLocalFilesystemFile::WriteFile(CDataBuffer& rBuffer, DWORD dwToWrite, DWORD& rdwBytesWritten)
{
	if(!IsOpen())
		return false;

	return ::WriteFile(m_hFile, rBuffer, dwToWrite, &rdwBytesWritten, NULL) != NULL && dwToWrite == rdwBytesWritten;
}

void TLocalFilesystemFile::Close()
{
	if(m_hFile != INVALID_HANDLE_VALUE)
		::CloseHandle(m_hFile);
	m_hFile = INVALID_HANDLE_VALUE;
}
