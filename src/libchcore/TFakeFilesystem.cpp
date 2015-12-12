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
#include "stdafx.h"
#include "TFakeFilesystem.h"
#include "RoundingFunctions.h"
#include "TCoreException.h"
#include "ErrorCodes.h"
#include "TFakeFilesystemFile.h"
#include "TFakeFilesystemFind.h"
#include "TPathContainer.h"

namespace chcore
{
	TFakeFilesystem::TFakeFilesystem()
	{
	}

	TFakeFilesystem::~TFakeFilesystem()
	{
	}

	bool TFakeFilesystem::PathExist(const TSmartPath& strPath)
	{
		for (const TFakeFileDescriptionPtr& spDesc : m_listFilesystemContent)
		{
			if (spDesc->GetFileInfo().GetFullFilePath() == strPath)
				return true;
		}

		return false;
	}

	bool TFakeFilesystem::GetDynamicFreeSpace(const TSmartPath& path, unsigned long long& rullFree)
	{
		// get total size of volume
		file_size_t fsSize = std::numeric_limits<file_size_t>::max();
		wchar_t wchDrive = path.GetDriveLetter();
		auto iterFind = m_mapVolumeInfo.find(wchDrive);
		if (iterFind != m_mapVolumeInfo.end())
			fsSize = iterFind->second.GetTotalSize();

		// decrease by file sizes
		for (const TFakeFileDescriptionPtr& spDesc : m_listFilesystemContent)
		{
			file_size_t fsFileSize(RoundUp(spDesc->GetFileInfo().GetLength64(), (file_size_t)IFilesystemFile::MaxSectorSize));
			if (fsFileSize > fsSize)
				return false;
			fsSize -= RoundUp(spDesc->GetFileInfo().GetLength64(), (file_size_t)IFilesystemFile::MaxSectorSize);
		}

		rullFree = fsSize;
		return false;
	}

	IFilesystem::EPathsRelation TFakeFilesystem::GetPathsRelation(const TSmartPath& pathFirst, const TSmartPath& pathSecond)
	{
		UINT uiFirstDriveType = DRIVE_FIXED;
		UINT uiSecondDriveType = DRIVE_FIXED;

		auto iterFirst = m_mapVolumeInfo.find(pathFirst.GetDriveLetter());
		auto iterSecond = m_mapVolumeInfo.find(pathSecond.GetDriveLetter());
		if (iterFirst != m_mapVolumeInfo.end())
			uiFirstDriveType = iterFirst->second.GetDriveType();
		if (iterSecond != m_mapVolumeInfo.end())
			uiSecondDriveType = iterSecond->second.GetDriveType();

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
				THROW_CORE_EXCEPTION(eErr_FixedDriveWithoutDriveLetter);

			if (wchFirstDrive == wchSecondDrive)
				eRelation = eRelation_SinglePhysicalDisk;
			else
			{
				DWORD dwFirstPhysicalDisk = 0;
				DWORD dwSecondPhysicalDisk = 1;

				if (iterFirst != m_mapVolumeInfo.end())
					dwFirstPhysicalDisk = iterFirst->second.GetPhysicalDriveNumber();
				if (iterSecond != m_mapVolumeInfo.end())
					dwSecondPhysicalDisk = iterSecond->second.GetPhysicalDriveNumber();

				if (dwFirstPhysicalDisk == std::numeric_limits<DWORD>::max() || dwSecondPhysicalDisk == std::numeric_limits<DWORD>::max())
				{
					// NOTE: disabled throwing an exception here - when testing, it came out that some DRIVE_FIXED
					//       volumes might have problems handling this detection (TrueCrypt volumes for example).
					//       So for now we report it as two different physical disks.
					//THROW(_T("Problem with physical disk detection"), 0, 0, 0);
					eRelation = eRelation_TwoPhysicalDisks;
				}

				if (dwFirstPhysicalDisk == dwSecondPhysicalDisk)
					eRelation = eRelation_SinglePhysicalDisk;
				else
					eRelation = eRelation_TwoPhysicalDisks;
			}
		}

		return eRelation;
	}

	IFilesystemFilePtr TFakeFilesystem::CreateFileObject(const TSmartPath& spFilename, bool bNoBuffering)
	{
		IFilesystemFilePtr spFile = std::make_shared<TFakeFilesystemFile>(spFilename, bNoBuffering, this);
		return spFile;
	}

	IFilesystemFindPtr TFakeFilesystem::CreateFinderObject(const TSmartPath& pathDir, const TSmartPath& pathMask)
	{
		// check if directory exists
		TFakeFileDescriptionPtr spFileDesc = FindFileByLocation(pathDir);
		if (!spFileDesc)
			return nullptr;

		IFilesystemFindPtr spFind = std::make_shared<TFakeFilesystemFind>(pathDir, pathMask, this);
		return spFind;
	}

	bool TFakeFilesystem::FastMove(const TSmartPath& pathSource, const TSmartPath& pathDestination)
	{
		TFakeFileDescriptionPtr spFileDesc = FindFileByLocation(pathSource);
		if (!spFileDesc)
			return false;

		// check parent of pathDestination
		TSmartPath pathParent = pathDestination.GetParent();
		if (pathParent.IsEmpty())
			return false;

		TFakeFileDescriptionPtr spParentDesc = FindFileByLocation(pathParent);
		if (!spParentDesc)
			return false;

		spFileDesc->GetFileInfo().SetFilePath(pathDestination);
		return true;
	}

	bool TFakeFilesystem::GetFileInfo(const TSmartPath& pathFile, TFileInfoPtr& rFileInfo, const TBasePathDataPtr& spBasePathData)
	{
		TFakeFileDescriptionPtr spFileDesc = FindFileByLocation(pathFile);
		if (!spFileDesc)
		{
			FILETIME fi = { 0, 0 };
			rFileInfo->Init(TSmartPath(), (DWORD)-1, 0, fi, fi, fi, 0);
			return false;
		}

		// copy data from W32_F_D
		rFileInfo->Init(spBasePathData, pathFile, spFileDesc->GetFileInfo().GetAttributes(),
			spFileDesc->GetFileInfo().GetLength64(),
			spFileDesc->GetFileInfo().GetCreationTime().GetAsFiletime(),
			spFileDesc->GetFileInfo().GetLastAccessTime().GetAsFiletime(),
			spFileDesc->GetFileInfo().GetLastWriteTime().GetAsFiletime(),
			0);

		return true;
	}

	bool TFakeFilesystem::DeleteFile(const TSmartPath& pathFile)
	{
		// check parent of pathDestination
		TSmartPath pathParent = pathFile.GetParent();
		if (pathParent.IsEmpty())
			return false;

		TFakeFileDescriptionPtr spParentDesc = FindFileByLocation(pathParent);
		if (!spParentDesc)
			return false;

		// similar to FindFileByLocation(), but operating on iterators
		for (auto iterList = m_listFilesystemContent.begin(); iterList != m_listFilesystemContent.end(); ++iterList)
		{
			TFakeFileDescriptionPtr spDesc = (*iterList);
			if (spDesc->GetFileInfo().GetFullFilePath() == pathFile)
			{
				if (spDesc->GetFileInfo().IsDirectory())
					return false;

				m_listFilesystemContent.erase(iterList);
				return true;
			}
		}

		return false;
	}

	bool TFakeFilesystem::RemoveDirectory(const TSmartPath& pathFile)
	{
		// check parent of pathDestination
		TSmartPath pathParent = pathFile.GetParent();
		if (pathParent.IsEmpty())
			return false;

		TFakeFileDescriptionPtr spParentDesc = FindFileByLocation(pathParent);
		if (!spParentDesc)
			return false;

		for (auto iterList = m_listFilesystemContent.begin(); iterList != m_listFilesystemContent.end(); ++iterList)
		{
			TFakeFileDescriptionPtr spDesc = (*iterList);
			if (spDesc->GetFileInfo().GetFullFilePath() == pathFile)
			{
				if (!spDesc->GetFileInfo().IsDirectory())
					return false;

				m_listFilesystemContent.erase(iterList);
				return true;
			}
		}

		return false;
	}

	bool TFakeFilesystem::CreateDirectory(const TSmartPath& pathDirectory, bool bCreateFullPath)
	{
		// check parent of pathDestination
		TFakeFileDescriptionPtr spDesc = FindFileByLocation(pathDirectory);
		if (spDesc)
			return true;

		if(!bCreateFullPath)
		{
			TSmartPath pathParent = pathDirectory.GetParent();
			if (pathParent.IsEmpty())
				return false;

			TFakeFileDescriptionPtr spParentDesc = FindFileByLocation(pathParent);
			if (!spParentDesc)
				return false;

			CreateFakeDirectory(pathDirectory);
		}
		else
			CreateFSObjectByLocation(pathDirectory);

		return true;
	}

	bool TFakeFilesystem::SetAttributes(const TSmartPath& pathFileDir, DWORD dwAttributes)
	{
		TFakeFileDescriptionPtr spFileDesc = FindFileByLocation(pathFileDir);
		if (!spFileDesc)
			return false;

		if (spFileDesc->GetFileInfo().IsDirectory() && !(dwAttributes & FILE_ATTRIBUTE_DIRECTORY))
			return false;
		if (!spFileDesc->GetFileInfo().IsDirectory() && (dwAttributes & FILE_ATTRIBUTE_DIRECTORY))
			return false;

		spFileDesc->GetFileInfo().SetAttributes(dwAttributes);
		return true;
	}

	bool TFakeFilesystem::SetFileDirectoryTime(const TSmartPath& pathFileDir, const TFileTime& ftCreationTime, const TFileTime& ftLastAccessTime, const TFileTime& ftLastWriteTime)
	{
		TFakeFileDescriptionPtr spFileDesc = FindFileByLocation(pathFileDir);
		if (!spFileDesc)
			return false;

		spFileDesc->GetFileInfo().SetFileTimes(ftCreationTime, ftLastAccessTime, ftLastWriteTime);
		return true;
	}

	void TFakeFilesystem::SetVolumeInfo(wchar_t wchVolumeLetter, file_size_t fsSize, UINT uiDriveType, DWORD dwPhysicalDiskNumber)
	{
		if (wchVolumeLetter >= L'a' && wchVolumeLetter <= L'z')
			wchVolumeLetter = wchVolumeLetter - 'a' + 'A';
		else if (wchVolumeLetter < L'A' || wchVolumeLetter > L'Z')
			THROW_CORE_EXCEPTION(eErr_InvalidArgument);

		auto iterMap = m_mapVolumeInfo.find(wchVolumeLetter);
		if (iterMap == m_mapVolumeInfo.end())
			m_mapVolumeInfo.emplace(std::make_pair(wchVolumeLetter, TFakeVolumeInfo(fsSize, uiDriveType, dwPhysicalDiskNumber)));
		else
		{
			iterMap->second.SetDriveType(uiDriveType);
			iterMap->second.SetTotalSize(fsSize);
		}
	}

	TFakeFileDescriptionPtr TFakeFilesystem::FindFileByLocation(const TSmartPath& rPath)
	{
		for (TFakeFileDescriptionPtr spFile : m_listFilesystemContent)
		{
			if (spFile->GetFileInfo().GetFullFilePath() == rPath)
				return spFile;
		}

		return nullptr;
	}

	TFakeFileDescriptionPtr TFakeFilesystem::CreateFSObjectByLocation(const TSmartPath& pathFSObject)
	{
		TPathContainer pathParts;
		pathFSObject.SplitPath(pathParts);

		TSmartPath pathPartial;
		TFakeFileDescriptionPtr spFileDesc;
		for (size_t stIndex = 0; stIndex != pathParts.GetCount(); ++stIndex)
		{
			if (stIndex == 0 && (!pathParts.GetAt(stIndex).IsDrive() && !pathParts.GetAt(stIndex).IsServerName()))
				THROW_CORE_EXCEPTION(eErr_InvalidArgument);

			pathPartial += pathParts.GetAt(stIndex);

			spFileDesc = FindFileByLocation(pathPartial);
			if (!spFileDesc)
				CreateFakeDirectory(pathPartial);
		}

		return spFileDesc;
	}

	FILETIME TFakeFilesystem::GetCurrentFileTime()
	{
		SYSTEMTIME st;
		GetSystemTime(&st);

		FILETIME ft;
		SystemTimeToFileTime(&st, &ft);

		return ft;
	}

	TFakeFileDescriptionPtr TFakeFilesystem::CreateFakeDirectory(const TSmartPath& pathDir)
	{
		FILETIME ftCurrent = GetCurrentFileTime();
		TFakeFileDescriptionPtr spDesc = std::make_shared<TFakeFileDescription>(
			TFileInfo(nullptr, pathDir, FILE_ATTRIBUTE_DIRECTORY, 0, ftCurrent, ftCurrent, ftCurrent, 0),
			TSparseRangeMap()
			);

		m_listFilesystemContent.emplace_back(std::move(spDesc));
		return spDesc;
	}
}
