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
// disable "warning C4201: nonstandard extension used : nameless struct/union"
// for standard VS2008 with SDK 6.0A where winioctl.h generates some warnings
// converted to errors by the project settings.
#pragma warning(push)
#pragma warning(disable: 4201)
#include <winioctl.h>
#pragma warning(pop)
#include "TCoreException.h"
#include "ErrorCodes.h"
#include "TPathContainer.h"
#include "TFileTime.h"
#include "TOverlappedDataBuffer.h"
#include "RoundingFunctions.h"
#include <atltrace.h>
#include "TBufferSizes.h"
#include "TLocalFilesystemFile.h"
#include <memory>
#include "TLocalFilesystemFind.h"
#include "TFileException.h"
#include "TDateTime.h"

namespace chcore
{
	TLocalFilesystem::TLocalFilesystem()
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

		HANDLE hFind = FindFirstFile(PrependPathExtensionIfNeeded(findPath).ToString(), &fd);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			::FindClose(hFind);
			return true;
		}
		else
			return false;
	}

	void TLocalFilesystem::SetFileDirectoryTime(const TSmartPath& pathFileDir, const TFileTime& ftCreationTime, const TFileTime& ftLastAccessTime, const TFileTime& ftLastWriteTime)
	{
		TAutoFileHandle hFile = TAutoFileHandle(CreateFile(PrependPathExtensionIfNeeded(pathFileDir).ToString(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS, nullptr));
		if (hFile == INVALID_HANDLE_VALUE)
		{
			DWORD dwLastError = GetLastError();
			throw TFileException(eErr_CannotOpenFile, dwLastError, pathFileDir, L"Cannot open file for setting file/directory times", LOCATION);
		}

		if (!SetFileTime(hFile, &ftCreationTime.GetAsFiletime(), &ftLastAccessTime.GetAsFiletime(), &ftLastWriteTime.GetAsFiletime()))
		{
			DWORD dwLastError = GetLastError();
			throw TFileException(eErr_CannotSetFileTimes, dwLastError, pathFileDir, L"Cannot set file/directory times", LOCATION);
		}
	}

	void TLocalFilesystem::SetAttributes(const TSmartPath& pathFileDir, DWORD dwAttributes)
	{
		if (!::SetFileAttributes(PrependPathExtensionIfNeeded(pathFileDir).ToString(), dwAttributes))
		{
			DWORD dwLastError = GetLastError();
			throw TFileException(eErr_CannotSetFileAttributes, dwLastError, pathFileDir, L"Cannot set file/directory attributes", LOCATION);
		}
	}

	void TLocalFilesystem::CreateDirectory(const TSmartPath& pathDirectory, bool bCreateFullPath)
	{
		if (!bCreateFullPath)
		{
			if (!::CreateDirectory(PrependPathExtensionIfNeeded(pathDirectory).ToString(), nullptr))
			{
				DWORD dwLastError = GetLastError();
				throw TFileException(eErr_CannotCreateDirectory, dwLastError, pathDirectory, L"Cannot create directory", LOCATION);
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
				// try to create subsequent paths
				if (!pathToTest.IsDrive() && !pathToTest.IsServerName())
				{
					// try to create the specified path
					BOOL bRes = ::CreateDirectory(PrependPathExtensionIfNeeded(pathToTest).ToString(), nullptr);
					if (!bRes)
					{
						DWORD dwLastError = GetLastError();
						if (dwLastError != ERROR_ALREADY_EXISTS)
							throw TFileException(eErr_CannotCreateDirectory, dwLastError, pathToTest, L"Cannot create directory", LOCATION);
					}
				}
			}
		}
	}

	void TLocalFilesystem::RemoveDirectory(const TSmartPath& pathDirectory)
	{
		if (!::RemoveDirectory(PrependPathExtensionIfNeeded(pathDirectory).ToString()))
		{
			DWORD dwLastError = GetLastError();
			throw TFileException(eErr_CannotRemoveDirectory, dwLastError, pathDirectory, L"Cannot delete directory", LOCATION);
		}
	}

	void TLocalFilesystem::DeleteFile(const TSmartPath& pathFile)
	{
		if (!::DeleteFile(PrependPathExtensionIfNeeded(pathFile).ToString()))
		{
			DWORD dwLastError = GetLastError();
			throw TFileException(eErr_CannotDeleteFile, dwLastError, pathFile, L"Cannot delete file", LOCATION);
		}
	}

	void TLocalFilesystem::GetFileInfo(const TSmartPath& pathFile, TFileInfoPtr& spFileInfo, const TBasePathDataPtr& spBasePathData)
	{
		if (!spFileInfo)
			throw TCoreException(eErr_InvalidArgument, L"spFileInfo", LOCATION);

		WIN32_FIND_DATA wfd;

		TSmartPath findPath = pathFile;
		bool bIsDrive = pathFile.IsDrive() || (pathFile.IsDriveWithRootDir());
		if(bIsDrive)
		{
			// add '\\' if needed and '*' for marking that we look for e.g. c:\*
			// instead of c:\, which would never be found the other way
			findPath.AppendIfNotExists(_T("*"), false);
		}

		HANDLE hFind = FindFirstFileEx(PrependPathExtensionIfNeeded(findPath).ToString(), FindExInfoStandard, &wfd, FindExSearchNameMatch, nullptr, 0);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			FindClose(hFind);

			if(bIsDrive)
			{
				TFileTime ftTime;
				ftTime.SetCurrentTime();

				spFileInfo->Init(spBasePathData, pathFile, FILE_ATTRIBUTE_DIRECTORY, 0, ftTime, ftTime, ftTime, 0);
			}
			else
			{
				// new instance of path to accommodate the corrected path (i.e. input path might have lower case names, but we'd like to
				// preserve the original case contained in the file system)
				TSmartPath pathNew(pathFile);
				pathNew.DeleteFileName();

				// copy data from W32_F_D
				spFileInfo->Init(spBasePathData, pathNew + PathFromString(wfd.cFileName),
					wfd.dwFileAttributes, (((ULONGLONG)wfd.nFileSizeHigh) << 32) + wfd.nFileSizeLow, wfd.ftCreationTime,
					wfd.ftLastAccessTime, wfd.ftLastWriteTime, 0);
			}
		}
		else
		{
			DWORD dwLastError = GetLastError();
			throw TFileException(eErr_CannotGetFileInfo, dwLastError, pathFile, L"Cannot retrieve file information", LOCATION);
		}
	}

	void TLocalFilesystem::FastMove(const TSmartPath& pathSource, const TSmartPath& pathDestination)
	{
		if (!::MoveFileEx(PrependPathExtensionIfNeeded(pathSource).ToString(), PrependPathExtensionIfNeeded(pathDestination).ToString(), 0))
		{
			DWORD dwLastError = GetLastError();
			// there is also the destination path that is important; tracking that would require adding a new exception class
			// complicating the solution. For now it's not necessary to have that information in the exception.
			throw TFileException(eErr_CannotFastMove, dwLastError, pathSource, L"Cannot fast move file/directory", LOCATION);
		}
	}

	IFilesystemFindPtr TLocalFilesystem::CreateFinderObject(const TSmartPath& pathDir, const TSmartPath& pathMask)
	{
		return std::shared_ptr<TLocalFilesystemFind>(new TLocalFilesystemFind(pathDir, pathMask));
	}

	IFilesystemFilePtr TLocalFilesystem::CreateFileObject(const TSmartPath& pathFile, bool bNoBuffering)
	{
		return std::shared_ptr<TLocalFilesystemFile>(new TLocalFilesystemFile(pathFile, bNoBuffering));
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

		// get information about both paths
		UINT uiFirstDriveType = 0;
		uiFirstDriveType = GetDriveData(pathFirst);

		UINT uiSecondDriveType = 0;
		uiSecondDriveType = GetDriveData(pathSecond);

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

		return eRelation;
	}

	DWORD TLocalFilesystem::GetPhysicalDiskNumber(wchar_t wchDrive)
	{
		{
			boost::shared_lock<boost::shared_mutex> lock(m_lockDriveLetterToPhysicalDisk);

			std::map<wchar_t, DWORD>::iterator iterMap = m_mapDriveLetterToPhysicalDisk.find(wchDrive);
			if (iterMap != m_mapDriveLetterToPhysicalDisk.end())
				return (*iterMap).second;
		}

		wchar_t szDrive[] = { L'\\', L'\\', L'.', L'\\', wchDrive, L':', L'\0' };

		HANDLE hDevice = CreateFile(szDrive, FILE_READ_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
		if (hDevice == INVALID_HANDLE_VALUE)
			return std::numeric_limits<DWORD>::max();

		// buffer for data (cannot make member nor static because this function might be called by many threads at once)
		// buffer is larger than one extent to allow getting information in multi-extent volumes (raid?)
		const int stSize = sizeof(VOLUME_DISK_EXTENTS) + 20 * sizeof(DISK_EXTENT);
		boost::shared_array<BYTE> spData(new BYTE[stSize]);

		VOLUME_DISK_EXTENTS* pVolumeDiskExtents = (VOLUME_DISK_EXTENTS*)spData.get();
		DWORD dwBytesReturned = 0;
		BOOL bResult = DeviceIoControl(hDevice, IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS, nullptr, 0, pVolumeDiskExtents, stSize, &dwBytesReturned, nullptr);
		if (!bResult)
		{
			CloseHandle(hDevice);

			// NOTE: when ERROR_INVALID_FUNCTION is reported here, it probably means that underlying volume
			//       cannot support IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS properly (such case includes TrueCrypt volumes)
			return std::numeric_limits<DWORD>::max();
		}

		CloseHandle(hDevice);

		if (pVolumeDiskExtents->NumberOfDiskExtents == 0)
			return std::numeric_limits<DWORD>::max();

		DISK_EXTENT* pDiskExtent = &pVolumeDiskExtents->Extents[0];

		boost::unique_lock<boost::shared_mutex> lock(m_lockDriveLetterToPhysicalDisk);
		m_mapDriveLetterToPhysicalDisk.insert(std::make_pair(wchDrive, pDiskExtent->DiskNumber));

		return pDiskExtent->DiskNumber;
	}

	void TLocalFilesystem::GetDynamicFreeSpace(const TSmartPath& path, unsigned long long& rullFree)
	{
		rullFree = 0;

		ULARGE_INTEGER ui64Available, ui64Total;
		if (GetDiskFreeSpaceEx(path.ToString(), &ui64Available, &ui64Total, nullptr))
			rullFree = ui64Available.QuadPart;
		else
		{
			DWORD dwLastError = GetLastError();
			throw TFileException(eErr_CannotGetFreeSpace, dwLastError, path, L"Failed to retrieve free space information", LOCATION);
		}
	}
}
