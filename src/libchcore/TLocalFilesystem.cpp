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
#include "DataBuffer.h"
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

BEGIN_CHCORE_NAMESPACE

UINT TLocalFilesystem::GetDriveData(const TSmartPath& spPath)
{
	UINT uiDrvType = DRIVE_UNKNOWN;
	if(!spPath.IsNetworkPath())
	{
		if(!spPath.IsEmpty())
		{
			TSmartPath pathDrive = spPath.GetDrive();
			pathDrive.AppendSeparatorIfDoesNotExist();

			uiDrvType = GetDriveType(pathDrive.ToString());
			if(uiDrvType == DRIVE_NO_ROOT_DIR)
				uiDrvType = DRIVE_UNKNOWN;
		}
	}
	else
		uiDrvType = DRIVE_REMOTE;

	return uiDrvType;
}

bool TLocalFilesystem::PathExist(TSmartPath pathToCheck)
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

bool TLocalFilesystem::SetFileDirectoryTime(const TSmartPath& pathFileDir, const TFileTime& ftCreationTime, const TFileTime& ftLastAccessTime, const TFileTime& ftLastWriteTime)
{
	TAutoFileHandle hFile = CreateFile(PrependPathExtensionIfNeeded(pathFileDir).ToString(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS, NULL);
	if(hFile == INVALID_HANDLE_VALUE)
		return false;

	BOOL bResult = SetFileTime(hFile, &ftCreationTime.GetAsFiletime(), &ftLastAccessTime.GetAsFiletime(), &ftLastWriteTime.GetAsFiletime());

	if(!hFile.Close())
		return false;

	return bResult != FALSE;
}

bool TLocalFilesystem::SetAttributes(const TSmartPath& pathFileDir, DWORD dwAttributes)
{
	return ::SetFileAttributes(PrependPathExtensionIfNeeded(pathFileDir).ToString(), dwAttributes) != FALSE;
}

bool TLocalFilesystem::CreateDirectory(const TSmartPath& pathDirectory, bool bCreateFullPath)
{
	if(!bCreateFullPath)
		return ::CreateDirectory(PrependPathExtensionIfNeeded(pathDirectory).ToString(), NULL) != FALSE;
	else
	{
		TPathContainer vComponents;
		pathDirectory.SplitPath(vComponents);

		TSmartPath pathToTest;
		for(size_t stIndex = 0; stIndex < vComponents.GetCount(); ++stIndex)
		{
			const TSmartPath& pathComponent = vComponents.GetAt(stIndex);

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

bool TLocalFilesystem::RemoveDirectory(const TSmartPath& pathFile)
{
	return ::RemoveDirectory(PrependPathExtensionIfNeeded(pathFile).ToString()) != FALSE;
}

bool TLocalFilesystem::DeleteFile(const TSmartPath& pathFile)
{
	return ::DeleteFile(PrependPathExtensionIfNeeded(pathFile).ToString()) != FALSE;
}

bool TLocalFilesystem::GetFileInfo(const TSmartPath& pathFile, TFileInfoPtr& rFileInfo, const TBasePathDataPtr& spBasePathData)
{
	if(!rFileInfo)
		THROW_CORE_EXCEPTION(eErr_InvalidArgument);

	WIN32_FIND_DATA wfd;
	HANDLE hFind = FindFirstFile(PrependPathExtensionIfNeeded(pathFile).ToString(), &wfd);

	if(hFind != INVALID_HANDLE_VALUE)
	{
		FindClose(hFind);

		// new instance of path to accomodate the corrected path (i.e. input path might have lower case names, but we'd like to
		// preserve the original case contained in the filesystem)
		TSmartPath pathNew(pathFile);
		pathNew.DeleteFileName();

		// copy data from W32_F_D
		rFileInfo->Init(spBasePathData, pathNew + PathFromString(wfd.cFileName),
			wfd.dwFileAttributes, (((ULONGLONG) wfd.nFileSizeHigh) << 32) + wfd.nFileSizeLow, wfd.ftCreationTime,
			wfd.ftLastAccessTime, wfd.ftLastWriteTime, 0);

		return true;
	}
	else
	{
		FILETIME fi = { 0, 0 };
		rFileInfo->Init(TSmartPath(), (DWORD)-1, 0, fi, fi, fi, 0);
		return false;
	}
}

bool TLocalFilesystem::FastMove(const TSmartPath& pathSource, const TSmartPath& pathDestination)
{
	return ::MoveFile(PrependPathExtensionIfNeeded(pathSource).ToString(), PrependPathExtensionIfNeeded(pathDestination).ToString()) != FALSE;
}

TLocalFilesystemFind TLocalFilesystem::CreateFinderObject(const TSmartPath& pathDir, const TSmartPath& pathMask)
{
	return TLocalFilesystemFind(pathDir, pathMask);
}

TLocalFilesystemFile TLocalFilesystem::CreateFileObject()
{
	return TLocalFilesystemFile();
}

TSmartPath TLocalFilesystem::PrependPathExtensionIfNeeded(const TSmartPath& pathInput)
{
	if(pathInput.GetLength() >= 248)
		return PathFromString(_T("\\\\?\\")) + pathInput;
	else
		return pathInput;
}

TLocalFilesystem::EPathsRelation TLocalFilesystem::GetPathsRelation(const TSmartPath& pathFirst, const TSmartPath& pathSecond)
{
	if(pathFirst.IsEmpty() || pathSecond.IsEmpty())
		THROW_CORE_EXCEPTION(eErr_InvalidArgument);

	// get information about both paths
	UINT uiFirstDriveType = 0;
	uiFirstDriveType = GetDriveData(pathFirst);

	UINT uiSecondDriveType = 0;
	uiSecondDriveType = GetDriveData(pathSecond);

	// what kind of relation...
	EPathsRelation eRelation = eRelation_Other;
	if(uiFirstDriveType == DRIVE_REMOTE || uiSecondDriveType == DRIVE_REMOTE)
		eRelation = eRelation_Network;
	else if(uiFirstDriveType == DRIVE_CDROM || uiSecondDriveType == DRIVE_CDROM)
		eRelation = eRelation_CDRom;
	else if(uiFirstDriveType == DRIVE_FIXED && uiSecondDriveType == DRIVE_FIXED)
	{
		// two hdd's - is this the same physical disk ?
		wchar_t wchFirstDrive = pathFirst.GetDriveLetter();
		wchar_t wchSecondDrive = pathSecond.GetDriveLetter();

		if(wchFirstDrive == L'\0' || wchSecondDrive == L'\0')
			THROW_CORE_EXCEPTION(eErr_FixedDriveWithoutDriveLetter);

		if(wchFirstDrive == wchSecondDrive)
			eRelation = eRelation_SinglePhysicalDisk;
		else
		{
			DWORD dwFirstPhysicalDisk = GetPhysicalDiskNumber(wchFirstDrive);
			DWORD dwSecondPhysicalDisk = GetPhysicalDiskNumber(wchSecondDrive);
			if(dwFirstPhysicalDisk == std::numeric_limits<DWORD>::max() || dwSecondPhysicalDisk == std::numeric_limits<DWORD>::max())
			{
				// NOTE: disabled throwing an exception here - when testing, it came out that some DRIVE_FIXED
				//       volumes might have problems handling this detection (TrueCrypt volumes for example).
				//       So for now we report it as two different physical disks.
				//THROW(_T("Problem with physical disk detection"), 0, 0, 0);
				eRelation = eRelation_TwoPhysicalDisks;
			}

			if(dwFirstPhysicalDisk == dwSecondPhysicalDisk)
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
		if(iterMap != m_mapDriveLetterToPhysicalDisk.end())
			return (*iterMap).second;
	}

	wchar_t szDrive[] = { L'\\', L'\\', L'.', L'\\', wchDrive, L':', L'\0' };

	HANDLE hDevice = CreateFile(szDrive, FILE_READ_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if(hDevice == INVALID_HANDLE_VALUE)
		return std::numeric_limits<DWORD>::max();

	// buffer for data (cannot make member nor static because this function might be called by many threads at once)
	// buffer is larger than one extent to allow getting information in multi-extent volumes (raid?)
	const int stSize = sizeof(VOLUME_DISK_EXTENTS) + 20 * sizeof(DISK_EXTENT);
	boost::shared_array<BYTE> spData(new BYTE[stSize]);

	VOLUME_DISK_EXTENTS* pVolumeDiskExtents = (VOLUME_DISK_EXTENTS*)spData.get();
	DWORD dwBytesReturned = 0;
	BOOL bResult = DeviceIoControl(hDevice, IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS, NULL, 0, pVolumeDiskExtents, stSize, &dwBytesReturned, NULL);
	if(!bResult)
	{
		// NOTE: when ERROR_INVALID_FUNCTION is reported here, it probably means that underlying volume
		//       cannot support IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS properly (such case includes TrueCrypt volumes)
		return std::numeric_limits<DWORD>::max();
	}

	CloseHandle(hDevice);

	if(pVolumeDiskExtents->NumberOfDiskExtents == 0)
		return std::numeric_limits<DWORD>::max();

	DISK_EXTENT* pDiskExtent = &pVolumeDiskExtents->Extents[0];

	boost::unique_lock<boost::shared_mutex> lock(m_lockDriveLetterToPhysicalDisk);
	m_mapDriveLetterToPhysicalDisk.insert(std::make_pair(wchDrive, pDiskExtent->DiskNumber));

	return pDiskExtent->DiskNumber;
}

bool TLocalFilesystem::GetDynamicFreeSpace(const TSmartPath& path, unsigned long long& rullFree)
{
	rullFree = 0;

	ULARGE_INTEGER ui64Available, ui64Total;
	if(GetDiskFreeSpaceEx(path.ToString(), &ui64Available, &ui64Total, NULL))
	{
		rullFree = ui64Available.QuadPart;
		return true;
	}
	else
		return false;
}

/////////////////////////////////////////////////////////////////////////////////////
// class TLocalFilesystemFind

TLocalFilesystemFind::TLocalFilesystemFind(const TSmartPath& pathDir, const TSmartPath& pathMask) :
	m_pathDir(pathDir),
	m_pathMask(pathMask),
	m_hFind(INVALID_HANDLE_VALUE)
{
}

TLocalFilesystemFind::~TLocalFilesystemFind()
{
	Close();
}

bool TLocalFilesystemFind::FindNext(TFileInfoPtr& rspFileInfo)
{
	WIN32_FIND_DATA wfd;
	TSmartPath pathCurrent = m_pathDir + m_pathMask;

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
				rspFileInfo->Init(m_pathDir + PathFromString(wfd.cFileName), wfd.dwFileAttributes, (((ULONGLONG) wfd.nFileSizeHigh) << 32) + wfd.nFileSizeLow, wfd.ftCreationTime,
					wfd.ftLastAccessTime, wfd.ftLastWriteTime, 0);
				return true;
			}
			else if(wfd.cFileName[0] != _T('.') || (wfd.cFileName[1] != _T('\0') && (wfd.cFileName[1] != _T('.') || wfd.cFileName[2] != _T('\0'))))
			{
				// Add directory itself
				rspFileInfo->Init(m_pathDir + PathFromString(wfd.cFileName),
					wfd.dwFileAttributes, (((ULONGLONG) wfd.nFileSizeHigh) << 32) + wfd.nFileSizeLow, wfd.ftCreationTime,
					wfd.ftLastAccessTime, wfd.ftLastWriteTime, 0);
				return true;
			}
		}
		while(m_hFind != INVALID_HANDLE_VALUE && ::FindNextFile(m_hFind, &wfd));	// checking m_hFind in case other thread changed it (it shouldn't happen though)

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
	m_pathFile(),
	m_bNoBuffering(false)
{
}

TLocalFilesystemFile::~TLocalFilesystemFile()
{
	Close();
}

DWORD TLocalFilesystemFile::GetFlagsAndAttributes(bool bNoBuffering) const
{
	return FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED | FILE_FLAG_SEQUENTIAL_SCAN | (bNoBuffering ? FILE_FLAG_NO_BUFFERING /*| FILE_FLAG_WRITE_THROUGH*/ : 0);
}

bool TLocalFilesystemFile::OpenExistingForReading(const TSmartPath& pathFile, bool bNoBuffering)
{
	Close();

	m_pathFile = TLocalFilesystem::PrependPathExtensionIfNeeded(pathFile);
	m_hFile = ::CreateFile(m_pathFile.ToString(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, GetFlagsAndAttributes(bNoBuffering), NULL);
	if(m_hFile == INVALID_HANDLE_VALUE)
		return false;
	
	m_bNoBuffering = bNoBuffering;
	return true;
}

bool TLocalFilesystemFile::CreateNewForWriting(const TSmartPath& pathFile, bool bNoBuffering)
{
	Close();

	m_pathFile = TLocalFilesystem::PrependPathExtensionIfNeeded(pathFile);
	m_hFile = ::CreateFile(m_pathFile.ToString(), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_NEW, GetFlagsAndAttributes(bNoBuffering), NULL);
	if(m_hFile == INVALID_HANDLE_VALUE)
		return false;

	m_bNoBuffering = bNoBuffering;
	return true;
}

bool TLocalFilesystemFile::OpenExistingForWriting(const TSmartPath& pathFile, bool bNoBuffering)
{
	Close();

	m_pathFile = TLocalFilesystem::PrependPathExtensionIfNeeded(pathFile);
	m_hFile = CreateFile(m_pathFile.ToString(), GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, GetFlagsAndAttributes(bNoBuffering), NULL);
	if(m_hFile == INVALID_HANDLE_VALUE)
		return false;

	m_bNoBuffering = bNoBuffering;
	return true;
}

bool TLocalFilesystemFile::SetFilePointer(long long llNewPos, DWORD dwMoveMethod)
{
	if(!IsOpen())
		return false;

	LARGE_INTEGER li = { 0, 0 };
	LARGE_INTEGER liNew = { 0, 0 };

	li.QuadPart = llNewPos;

	return SetFilePointerEx(m_hFile, li, &liNew, dwMoveMethod) != FALSE;
}

bool TLocalFilesystemFile::SetEndOfFile()
{
	if(!IsOpen())
		return false;

	return ::SetEndOfFile(m_hFile) != FALSE;
}

bool TLocalFilesystemFile::ReadFile(TOverlappedDataBuffer& rBuffer)
{
	if (!IsOpen())
		THROW_CORE_EXCEPTION(eErr_InternalProblem);

	ATLTRACE(_T("Reading %lu bytes\n"), rBuffer.GetRequestedDataSize());
	if(!::ReadFileEx(m_hFile, rBuffer.GetBufferPtr(), rBuffer.GetRequestedDataSize(), &rBuffer, OverlappedReadCompleted))
	{
		DWORD dwLastError = GetLastError();
		switch(dwLastError)
		{
		case ERROR_IO_PENDING:
			return true;

		case ERROR_HANDLE_EOF:
			{
				rBuffer.SetBytesTransferred(0);
				rBuffer.SetStatusCode(0);
				rBuffer.SetErrorCode(ERROR_SUCCESS);
				rBuffer.SetLastPart(true);

				rBuffer.RequeueAsFull();	// basically the same as OverlappedReadCompleted

				return true;
			}
		}

		return false;
	}
	return true;
}

bool TLocalFilesystemFile::WriteFile(TOverlappedDataBuffer& rBuffer)
{
	if (!IsOpen())
		THROW_CORE_EXCEPTION(eErr_InternalProblem);

	DWORD dwToWrite = boost::numeric_cast<DWORD>(rBuffer.GetRealDataSize());

	if (m_bNoBuffering && rBuffer.IsLastPart())
		dwToWrite = RoundUp<DWORD>(dwToWrite, MaxSectorSize);

	ATLTRACE(_T("Writing %lu bytes\n"), dwToWrite);
	if (!::WriteFileEx(m_hFile, rBuffer.GetBufferPtr(), dwToWrite, &rBuffer, OverlappedWriteCompleted))
	{
		if (GetLastError() == ERROR_IO_PENDING)
			return true;
		return false;
	}

	return true;
}

bool TLocalFilesystemFile::FinalizeFile(TOverlappedDataBuffer& rBuffer)
{
	if (!IsOpen())
		THROW_CORE_EXCEPTION(eErr_InternalProblem);

	if (m_bNoBuffering && rBuffer.IsLastPart())
	{
		DWORD dwToWrite = boost::numeric_cast<DWORD>(rBuffer.GetRealDataSize());
		DWORD dwReallyWritten = RoundUp<DWORD>(dwToWrite, MaxSectorSize);

		ATLTRACE(_T("Finalize file - size diff: written: %I64u, required: %I64u\n"), dwReallyWritten, dwToWrite);

		if (dwToWrite != dwReallyWritten)
		{
			unsigned long long ullNewFileSize = rBuffer.GetFilePosition() + dwToWrite;	// new size

			if (!OpenExistingForWriting(m_pathFile, false))
				return false;

			//seek
			ATLTRACE(_T("Truncating file to %I64u bytes\n"), ullNewFileSize);
			if (!SetFilePointer(ullNewFileSize, FILE_BEGIN))
				return false;

			//set eof
			if (!SetEndOfFile())
				return false;
		}
	}

	return true;
}

void TLocalFilesystemFile::Close()
{
	if(m_hFile != INVALID_HANDLE_VALUE)
		::CloseHandle(m_hFile);
	m_hFile = INVALID_HANDLE_VALUE;
}

unsigned long long TLocalFilesystemFile::GetFileSize() const
{
	if(!IsOpen())
		return 0;

	BY_HANDLE_FILE_INFORMATION bhfi;

	if(!::GetFileInformationByHandle(m_hFile, &bhfi))
		return 0;

	ULARGE_INTEGER uli;
	uli.HighPart = bhfi.nFileSizeHigh;
	uli.LowPart = bhfi.nFileSizeLow;

	return uli.QuadPart;
}

END_CHCORE_NAMESPACE
