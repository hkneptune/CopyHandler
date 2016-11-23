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
#include <boost/smart_ptr/shared_array.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include "TAutoHandles.h"
#include "TFileInfo.h"
#include <winioctl.h>
#include "TCoreException.h"
#include "ErrorCodes.h"
#include "TPathContainer.h"
#include "TFileTime.h"
#include "TLocalFilesystemFile.h"
#include <memory>
#include "TLocalFilesystemFind.h"
#include "TFileException.h"
#include <boost/thread/locks.hpp>
#include "StreamingHelpers.h"

namespace chcore
{
	TLocalFilesystem::TLocalFilesystem(const logger::TLogFileDataPtr& spLogFileData) :
		m_spLog(logger::MakeLogger(spLogFileData, L"Filesystem"))
	{
	}

	TLocalFilesystem::~TLocalFilesystem()
	{
	}

	UINT TLocalFilesystem::GetDriveData(const TSmartPath& spPath)
	{
		UINT uiDrvType = DRIVE_UNKNOWN;
		if (!spPath.IsNetworkPath())
		{
			if (!spPath.IsEmpty())
			{
				TSmartPath pathDrive = spPath.GetDrive();
				pathDrive.AppendSeparatorIfDoesNotExist();

				uiDrvType = GetDriveType(pathDrive.ToString());
				if (uiDrvType == DRIVE_NO_ROOT_DIR)
					uiDrvType = DRIVE_UNKNOWN;
			}
		}
		else
			uiDrvType = DRIVE_REMOTE;

		return uiDrvType;
	}

	bool TLocalFilesystem::PathExist(const TSmartPath& pathToCheck)
	{
		WIN32_FIND_DATA fd;

		TSmartPath findPath = pathToCheck;
		bool bIsDrive = pathToCheck.IsDrive() || (pathToCheck.IsDriveWithRootDir());

		if(bIsDrive)
		{
			// add '\\' if needed and '*' for marking that we look for e.g. c:\*
			// instead of c:\, which would never be found the other way
			findPath.AppendIfNotExists(_T("*"), false);
		}

		findPath = PrependPathExtensionIfNeeded(findPath);

		LOG_DEBUG(m_spLog) << L"Checking for path existence: " << pathToCheck << L" (using search pattern: " << findPath << L")";

		HANDLE hFind = FindFirstFile(findPath.ToString(), &fd);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			::FindClose(hFind);
			LOG_DEBUG(m_spLog) << L"Path: " << pathToCheck << L" exists";
			return true;
		}

		LOG_DEBUG(m_spLog) << L"Path: " << pathToCheck << L" does not exist";
		return false;
	}

	void TLocalFilesystem::SetFileDirectoryTime(const TSmartPath& pathFileDir, const TFileTime& ftCreationTime, const TFileTime& ftLastAccessTime, const TFileTime& ftLastWriteTime)
	{
		TSmartPath fullPath = PrependPathExtensionIfNeeded(pathFileDir);

		LOG_TRACE(m_spLog) << L"Setting file/directory times for " << fullPath <<
			L", creation-time: " << ftCreationTime.GetAsFiletime() <<
			L", last-access-time: " << ftLastAccessTime.GetAsFiletime() <<
			L", last-write-time: " << ftLastWriteTime.GetAsFiletime();

		TAutoFileHandle hFile = TAutoFileHandle(CreateFile(fullPath.ToString(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS, nullptr));
		if (hFile == INVALID_HANDLE_VALUE)
		{
			DWORD dwLastError = GetLastError();
			LOG_ERROR(m_spLog) << L"Open file failed with error: " << dwLastError << L". Cannot set file/directory times.";

			throw TFileException(eErr_CannotOpenFile, dwLastError, pathFileDir, L"Cannot open file for setting file/directory times", LOCATION);
		}

		if (!SetFileTime(hFile, &ftCreationTime.GetAsFiletime(), &ftLastAccessTime.GetAsFiletime(), &ftLastWriteTime.GetAsFiletime()))
		{
			DWORD dwLastError = GetLastError();
			LOG_ERROR(m_spLog) << L"Failed to set file/directory times. Error: " << dwLastError;
			throw TFileException(eErr_CannotSetFileTimes, dwLastError, pathFileDir, L"Cannot set file/directory times", LOCATION);
		}
		LOG_TRACE(m_spLog) << L"File/directory times set successfully";
	}

	void TLocalFilesystem::SetAttributes(const TSmartPath& pathFileDir, DWORD dwAttributes)
	{
		TSmartPath fullPath = PrependPathExtensionIfNeeded(pathFileDir);

		LOG_TRACE(m_spLog) << L"Setting file/directory attributes for " << fullPath <<
			L", attributes: " << dwAttributes;

		if (!::SetFileAttributes(fullPath.ToString(), dwAttributes))
		{
			DWORD dwLastError = GetLastError();
			LOG_ERROR(m_spLog) << L"Failed to set file/directory attributes. Error: " << dwLastError;
			throw TFileException(eErr_CannotSetFileAttributes, dwLastError, pathFileDir, L"Cannot set file/directory attributes", LOCATION);
		}
		LOG_TRACE(m_spLog) << L"File/directory attributes set successfully";
	}

	void TLocalFilesystem::CreateDirectory(const TSmartPath& pathDirectory, bool bCreateFullPath)
	{
		LOG_DEBUG(m_spLog) << L"Creating directory " << pathDirectory <<
			L", create-full-path: " << bCreateFullPath;

		if (!bCreateFullPath)
		{
			if (!::CreateDirectory(PrependPathExtensionIfNeeded(pathDirectory).ToString(), nullptr))
			{
				DWORD dwLastError = GetLastError();
				if(dwLastError != ERROR_ALREADY_EXISTS)
				{
					LOG_ERROR(m_spLog) << L"Creating directory " << pathDirectory << L" failed with error " << dwLastError;
					throw TFileException(eErr_CannotCreateDirectory, dwLastError, pathDirectory, L"Cannot create directory", LOCATION);
				}
				
				LOG_DEBUG(m_spLog) << L"Directory " << pathDirectory << L" already exists, nothing to do";
				return;
			}
		}
		else
		{
			TPathContainer vComponents;
			pathDirectory.SplitPath(vComponents);

			TSmartPath pathToTest;
			for (size_t stIndex = 0; stIndex < vComponents.GetCount(); ++stIndex)
			{
				const TSmartPath& pathComponent = vComponents.GetAt(stIndex);
				pathToTest += pathComponent;

				LOG_DEBUG(m_spLog) << L"Creating " << pathDirectory << L" - partial path: " << pathToTest;

				// try to create subsequent paths
				if(!pathToTest.IsDrive() && !pathToTest.IsServerName())
				{
					// try to create the specified path
					BOOL bRes = ::CreateDirectory(PrependPathExtensionIfNeeded(pathToTest).ToString(), nullptr);
					if(!bRes)
					{
						DWORD dwLastError = GetLastError();
						if(dwLastError != ERROR_ALREADY_EXISTS)
						{
							LOG_ERROR(m_spLog) << L"Creating directory " << pathToTest << L" failed with error " << dwLastError;
							throw TFileException(eErr_CannotCreateDirectory, dwLastError, pathToTest, L"Cannot create directory", LOCATION);
						}
					}
				}
				else
					LOG_DEBUG(m_spLog) << L"Skipping drive/server part of a path when creating directory " << pathToTest;
			}
		}

		LOG_DEBUG(m_spLog) << L"Directory " << pathDirectory << L" created successfully";
	}

	void TLocalFilesystem::RemoveDirectory(const TSmartPath& pathDirectory)
	{
		TSmartPath fullPath = PrependPathExtensionIfNeeded(pathDirectory);
		LOG_DEBUG(m_spLog) << L"Removing directory " << fullPath;

		if (!::RemoveDirectory(fullPath.ToString()))
		{
			DWORD dwLastError = GetLastError();
			LOG_ERROR(m_spLog) << L"Failed to remove directory " << fullPath << L". Error: " << dwLastError;
			throw TFileException(eErr_CannotRemoveDirectory, dwLastError, pathDirectory, L"Cannot delete directory", LOCATION);
		}

		LOG_DEBUG(m_spLog) << L"Directory " << fullPath << L" removed successfully";
	}

	void TLocalFilesystem::DeleteFile(const TSmartPath& pathFile)
	{
		TSmartPath fullPath = PrependPathExtensionIfNeeded(pathFile);
		LOG_DEBUG(m_spLog) << L"Removing file " << fullPath;

		if (!::DeleteFile(fullPath.ToString()))
		{
			DWORD dwLastError = GetLastError();
			LOG_ERROR(m_spLog) << L"Failed to remove file " << fullPath << L". Error: " << dwLastError;
			throw TFileException(eErr_CannotDeleteFile, dwLastError, pathFile, L"Cannot delete file", LOCATION);
		}

		LOG_DEBUG(m_spLog) << L"File " << fullPath << L" removed successfully";
	}

	void TLocalFilesystem::GetFileInfo(const TSmartPath& pathFile, TFileInfoPtr& spFileInfo, const TBasePathDataPtr& spBasePathData)
	{
		if (!spFileInfo)
			throw TCoreException(eErr_InvalidArgument, L"spFileInfo", LOCATION);

		LOG_DEBUG(m_spLog) << L"Retrieving file information for " << pathFile <<
			L" with base path data: " << TSmartPath(spBasePathData ? spBasePathData->GetSrcPath() : TSmartPath());

		WIN32_FIND_DATA wfd;

		TSmartPath findPath = pathFile;
		bool bIsDrive = pathFile.IsDrive() || (pathFile.IsDriveWithRootDir());
		if(bIsDrive)
		{
			// add '\\' if needed and '*' for marking that we look for e.g. c:\*
			// instead of c:\, which would never be found the other way
			findPath.AppendIfNotExists(_T("*"), false);
		}

		LOG_DEBUG(m_spLog) << L"Retrieving file information for " << pathFile << L". Using search pattern: " << findPath;

		HANDLE hFind = FindFirstFileEx(PrependPathExtensionIfNeeded(findPath).ToString(), FindExInfoStandard, &wfd, FindExSearchNameMatch, nullptr, 0);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			FindClose(hFind);

			if(bIsDrive)
			{
				TFileTime ftTime;
				ftTime.SetCurrentTime();

				spFileInfo->Init(spBasePathData, pathFile, FILE_ATTRIBUTE_DIRECTORY, 0, ftTime, ftTime, ftTime, 0);
				LOG_DEBUG(m_spLog) << L"Retrieved information for drive - using current timestamp file information for " << pathFile;
			}
			else
			{
				// new instance of path to accommodate the corrected path (i.e. input path might have lower case names, but we'd like to
				// preserve the original case contained in the file system)
				TSmartPath pathNew(pathFile);
				pathNew.DeleteFileName();
				pathNew += PathFromString(wfd.cFileName);

				unsigned long long ullFullSize = (((unsigned long long)wfd.nFileSizeHigh) << 32) + wfd.nFileSizeLow;

				// copy data from W32_F_D
				spFileInfo->Init(spBasePathData, pathNew,
					wfd.dwFileAttributes, ullFullSize, wfd.ftCreationTime,
					wfd.ftLastAccessTime, wfd.ftLastWriteTime, 0);

				LOG_DEBUG(m_spLog) << L"Retrieved information for file/directory " << pathFile <<
					L", full-path: " << pathNew <<
					L", attributes: " << wfd.dwFileAttributes <<
					L", size: " << ullFullSize <<
					L", creation-time: " << wfd.ftCreationTime <<
					L", last-access-time: " << wfd.ftLastAccessTime <<
					L", last-write-time: " << wfd.ftLastWriteTime;
			}
		}
		else
		{
			DWORD dwLastError = GetLastError();
			LOG_ERROR(m_spLog) << L"Failed to retrieve file information for " << pathFile << L". Error: " << dwLastError;
			throw TFileException(eErr_CannotGetFileInfo, dwLastError, pathFile, L"Cannot retrieve file information", LOCATION);
		}
	}

	void TLocalFilesystem::FastMove(const TSmartPath& pathSource, const TSmartPath& pathDestination)
	{
		TSmartPath pathMoveFrom = PrependPathExtensionIfNeeded(pathSource);
		TSmartPath pathMoveTo = PrependPathExtensionIfNeeded(pathDestination);

		LOG_DEBUG(m_spLog) << L"Trying to fast-move file " << pathMoveFrom << L" to " << pathMoveTo;

		if (!::MoveFileEx(pathMoveFrom.ToString(), pathMoveTo.ToString(), 0))
		{
			DWORD dwLastError = GetLastError();

			LOG_ERROR(m_spLog) << L"Failed to fast-move file " << pathMoveFrom << L" to " << pathMoveTo << L". Error: " << dwLastError;
			throw TFileException(eErr_CannotFastMove, dwLastError, pathSource, L"Cannot fast move file/directory", LOCATION);
		}

		LOG_DEBUG(m_spLog) << L"Successfully fast-moved file " << pathMoveFrom << L" to " << pathMoveTo;
	}

	IFilesystemFindPtr TLocalFilesystem::CreateFinderObject(const TSmartPath& pathDir, const TSmartPath& pathMask)
	{
		LOG_TRACE(m_spLog) << L"Creating file finder object for path " << pathDir << L" with mask " << pathMask;

		return std::shared_ptr<TLocalFilesystemFind>(new TLocalFilesystemFind(pathDir, pathMask, m_spLog->GetLogFileData()));
	}

	IFilesystemFilePtr TLocalFilesystem::CreateFileObject(IFilesystemFile::EOpenMode eMode, const TSmartPath& pathFile, bool bNoBuffering, bool bProtectReadOnlyFiles)
	{
		LOG_TRACE(m_spLog) << L"Creating file object for path " << pathFile << L" with no-buffering set to " << bNoBuffering << L" and protect-read-only set to " << bProtectReadOnlyFiles;
		return std::shared_ptr<TLocalFilesystemFile>(new TLocalFilesystemFile(eMode, pathFile, bNoBuffering, bProtectReadOnlyFiles, m_spLog->GetLogFileData()));
	}

	TSmartPath TLocalFilesystem::PrependPathExtensionIfNeeded(const TSmartPath& pathInput)
	{
		const TSmartPath pathPrefix = PathFromString(L"\\\\?\\");

		if (pathInput.GetLength() >= 248 && !pathInput.StartsWith(pathPrefix))
			return pathPrefix + pathInput;

		return pathInput;
	}

	TLocalFilesystem::EPathsRelation TLocalFilesystem::GetPathsRelation(const TSmartPath& pathFirst, const TSmartPath& pathSecond)
	{
		if (pathFirst.IsEmpty())
			throw TCoreException(eErr_InvalidArgument, L"pathFirst", LOCATION);
		if(pathSecond.IsEmpty())
			throw TCoreException(eErr_InvalidArgument, L"pathSecond", LOCATION);

		LOG_DEBUG(m_spLog) << L"Trying to find relation between paths: " << pathFirst << L" and " << pathSecond;

		// get information about both paths
		UINT uiFirstDriveType = GetDriveData(pathFirst);
		UINT uiSecondDriveType = GetDriveData(pathSecond);

		LOG_TRACE(m_spLog) << L"Drive type for " << pathFirst << L" is " << uiFirstDriveType << L", drive type for " << pathSecond << L" is " << uiSecondDriveType;

		// what kind of relation...
		EPathsRelation eRelation = eRelation_Other;
		if (uiFirstDriveType == DRIVE_REMOTE || uiSecondDriveType == DRIVE_REMOTE)
			eRelation = eRelation_Network;
		else if (uiFirstDriveType == DRIVE_CDROM || uiSecondDriveType == DRIVE_CDROM)
			eRelation = eRelation_CDRom;
		else if (uiFirstDriveType == DRIVE_FIXED && uiSecondDriveType == DRIVE_FIXED)
		{
			// two hdd's - is this the same physical disk ?
			wchar_t wchFirstDrive = pathFirst.GetDriveLetter();
			wchar_t wchSecondDrive = pathSecond.GetDriveLetter();

			if (wchFirstDrive == L'\0' || wchSecondDrive == L'\0')
				throw TCoreException(eErr_FixedDriveWithoutDriveLetter, L"Fixed drive does not have drive letter assigned", LOCATION);

			if (wchFirstDrive == wchSecondDrive)
				eRelation = eRelation_SinglePhysicalDisk;
			else
			{
				DWORD dwFirstPhysicalDisk = GetPhysicalDiskNumber(wchFirstDrive);
				DWORD dwSecondPhysicalDisk = GetPhysicalDiskNumber(wchSecondDrive);

				LOG_TRACE(m_spLog) << L"Physical disk for " << pathFirst << L" is " << dwFirstPhysicalDisk << L", physical disk for " << pathSecond << L" is " << dwSecondPhysicalDisk;

				if (dwFirstPhysicalDisk == std::numeric_limits<DWORD>::max() || dwSecondPhysicalDisk == std::numeric_limits<DWORD>::max())
				{
					// NOTE: disabled throwing an exception here - when testing, it came out that some DRIVE_FIXED
					//       volumes might have problems handling this detection (TrueCrypt volumes for example).
					//       So for now we report it as two different physical disks.
					//THROW(_T("Problem with physical disk detection"), 0, 0, 0);
					eRelation = eRelation_TwoPhysicalDisks;
				}
				else if (dwFirstPhysicalDisk == dwSecondPhysicalDisk)
					eRelation = eRelation_SinglePhysicalDisk;
				else
					eRelation = eRelation_TwoPhysicalDisks;
			}
		}

		LOG_DEBUG(m_spLog) << L"Drive relation for " << pathFirst << L" and " << pathSecond << L" is " << eRelation;

		return eRelation;
	}

	DWORD TLocalFilesystem::GetPhysicalDiskNumber(wchar_t wchDrive)
	{
		LOG_TRACE(m_spLog) << L"Checking physical disk number for drive " << wchDrive;

		{
			boost::shared_lock<boost::shared_mutex> lock(m_lockDriveLetterToPhysicalDisk);

			std::map<wchar_t, DWORD>::iterator iterMap = m_mapDriveLetterToPhysicalDisk.find(wchDrive);
			if (iterMap != m_mapDriveLetterToPhysicalDisk.end())
			{
				LOG_TRACE(m_spLog) << L"Physical disk number for drive " << wchDrive << L" is " << (*iterMap).second << L" (cached)";
				return (*iterMap).second;
			}
		}

		wchar_t szDrive[] = { L'\\', L'\\', L'.', L'\\', wchDrive, L':', L'\0' };

		LOG_DEBUG(m_spLog) << L"Creating handle for drive " << szDrive;

		HANDLE hDevice = CreateFile(szDrive, FILE_READ_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
		if (hDevice == INVALID_HANDLE_VALUE)
		{
			DWORD dwLastError = GetLastError();
			LOG_DEBUG(m_spLog) << L"Failed to create handle for drive " << szDrive << L". Error: " << dwLastError;
			return std::numeric_limits<DWORD>::max();
		}

		// buffer for data (cannot make member nor static because this function might be called by many threads at once)
		// buffer is larger than one extent to allow getting information in multi-extent volumes (raid?)
		const int stSize = sizeof(VOLUME_DISK_EXTENTS) + 20 * sizeof(DISK_EXTENT);
		boost::shared_array<BYTE> spData(new BYTE[stSize]);

		LOG_DEBUG(m_spLog) << L"Retrieving volume disk extents for drive " << szDrive;

		VOLUME_DISK_EXTENTS* pVolumeDiskExtents = (VOLUME_DISK_EXTENTS*)spData.get();
		DWORD dwBytesReturned = 0;
		BOOL bResult = DeviceIoControl(hDevice, IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS, nullptr, 0, pVolumeDiskExtents, stSize, &dwBytesReturned, nullptr);
		if (!bResult)
		{
			DWORD dwLastError = GetLastError();
			LOG_ERROR(m_spLog) << L"Failed to retrieve volume disk extents for drive " << szDrive << L". Error: " << dwLastError;

			CloseHandle(hDevice);

			// NOTE: when ERROR_INVALID_FUNCTION is reported here, it probably means that underlying volume
			//       cannot support IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS properly (such case includes TrueCrypt volumes)
			return std::numeric_limits<DWORD>::max();
		}

		CloseHandle(hDevice);

		if (pVolumeDiskExtents->NumberOfDiskExtents == 0)
		{
			LOG_DEBUG(m_spLog) << L"No disk extents available for drive " << szDrive;
			return std::numeric_limits<DWORD>::max();
		}

		DISK_EXTENT* pDiskExtent = &pVolumeDiskExtents->Extents[0];

		LOG_DEBUG(m_spLog) << L"Retrieved disk number for drive " << szDrive << L": " << pDiskExtent->DiskNumber;

		boost::unique_lock<boost::shared_mutex> lock(m_lockDriveLetterToPhysicalDisk);
		m_mapDriveLetterToPhysicalDisk.insert(std::make_pair(wchDrive, pDiskExtent->DiskNumber));

		return pDiskExtent->DiskNumber;
	}

	void TLocalFilesystem::GetDynamicFreeSpace(const TSmartPath& path, unsigned long long& rullFree, unsigned long long& rullTotal)
	{
		LOG_DEBUG(m_spLog) << L"Retrieving free space for path " << path;

		rullFree = 0;

		ULARGE_INTEGER ui64Available, ui64Total;
		if (GetDiskFreeSpaceEx(path.ToString(), &ui64Available, &ui64Total, nullptr))
		{
			rullFree = ui64Available.QuadPart;
			rullTotal = ui64Total.QuadPart;
			LOG_DEBUG(m_spLog) << L"Free space for path " << path << L" is " << rullFree;
		}
		else
		{
			DWORD dwLastError = GetLastError();
			LOG_ERROR(m_spLog) << L"Failed to retrieve free space for path " << path << L". Error: " << dwLastError;
			throw TFileException(eErr_CannotGetFreeSpace, dwLastError, path, L"Failed to retrieve free space information", LOCATION);
		}
	}
}
