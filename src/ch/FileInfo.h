/***************************************************************************
*   Copyright (C) 2001-2008 by Jozef Starosczyk                           *
*   ixen@copyhandler.com                                                  *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU Library General Public License          *
*   (version 2) as published by the Free Software Foundation;             *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU Library General Public     *
*   License along with this program; if not, write to the                 *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/
// File was originally based on FileInfo.h by Antonio Tejada Lacaci.
// Almost everything has changed since then.

#ifndef __FILEINFO_H__
#define __FILEINFO_H__

#include "../libchcore/TPath.h"

// CFileInfo flags
// flag stating that file has been processed (used to determine if file can be deleted at the end of copying)
#define FIF_PROCESSED		0x00000001

class CFiltersArray;

class CFileInfo
{  
public:
	CFileInfo();
	CFileInfo(const CFileInfo& finf);
	~CFileInfo();

	void Create(const WIN32_FIND_DATA* pwfd, const chcore::TSmartPath& tFilePath, size_t stSrcIndex);
	bool Create(const chcore::TSmartPath& strFilePath, size_t stSrcIndex);

	ULONGLONG GetLength64() const { return m_uhFileSize; }
	void SetLength64(ULONGLONG uhSize) { m_uhFileSize=uhSize; }

	const chcore::TSmartPath& GetFilePath() const { return m_pathFile; }	// returns path with m_pathFile (probably not full)
	chcore::TSmartPath GetFullFilePath() const;		// returns full path
	void SetFilePath(const chcore::TSmartPath& tPath) { m_pathFile = tPath; };

	/* Get File times info (equivalent to CFindFile members) */
	const FILETIME& GetCreationTime() const { return m_ftCreation; };
	const FILETIME& GetLastAccessTime() const { return m_ftLastAccess; };
	const FILETIME& GetLastWriteTime() const { return m_ftLastWrite; };

	/* Get File attributes info (equivalent to CFindFile members) */
	DWORD GetAttributes() const { return m_dwAttributes; }
	bool IsDirectory() const { return (m_dwAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0; }
	bool IsArchived() const { return (m_dwAttributes & FILE_ATTRIBUTE_ARCHIVE) != 0; }
	bool IsReadOnly() const { return (m_dwAttributes & FILE_ATTRIBUTE_READONLY) != 0; }
	bool IsCompressed() const { return (m_dwAttributes & FILE_ATTRIBUTE_COMPRESSED) != 0; }
	bool IsSystem() const { return (m_dwAttributes & FILE_ATTRIBUTE_SYSTEM) != 0; }
	bool IsHidden() const { return (m_dwAttributes & FILE_ATTRIBUTE_HIDDEN) != 0; }
	bool IsTemporary() const { return (m_dwAttributes & FILE_ATTRIBUTE_TEMPORARY) != 0; }
	bool IsNormal() const { return m_dwAttributes == 0; }

	uint_t GetFlags() const { return m_uiFlags; }
	void SetFlags(uint_t uiFlags, uint_t uiMask = 0xffffffff) { m_uiFlags = (m_uiFlags & ~(uiFlags & uiMask)) | (uiFlags & uiMask); }

	// operations
	void SetClipboard(const chcore::TPathContainer* pBasePaths) { m_pBasePaths = pBasePaths; }

	void SetSrcIndex(size_t stIndex) { m_stSrcIndex = stIndex; };
	size_t GetSrcIndex() const { return m_stSrcIndex; };

	// operators
	bool operator==(const CFileInfo& rInfo);

	template<class Archive>
	void serialize(Archive& ar, unsigned int /*uiVersion*/)
	{
		ar & m_pathFile;
		ar & m_stSrcIndex;
		ar & m_dwAttributes;
		ar & m_uhFileSize;
		ar & m_ftCreation.dwHighDateTime;
		ar & m_ftCreation.dwLowDateTime;
		ar & m_ftLastAccess.dwHighDateTime;
		ar & m_ftLastAccess.dwLowDateTime;
		ar & m_ftLastWrite.dwHighDateTime;
		ar & m_ftLastWrite.dwLowDateTime;
		ar & m_uiFlags;
	}

private:
	chcore::TSmartPath m_pathFile;	// contains relative path (first path is in CClipboardArray)

	size_t m_stSrcIndex;		// index in CClipboardArray table (which contains the first part of the path)
	const chcore::TPathContainer* m_pBasePaths;

	DWORD m_dwAttributes;	// attributes
	ULONGLONG m_uhFileSize;
	FILETIME  m_ftCreation;
	FILETIME  m_ftLastAccess;
	FILETIME  m_ftLastWrite;

	uint_t m_uiFlags;
};

typedef boost::shared_ptr<CFileInfo> CFileInfoPtr;

class CFileInfoArray
{
public:
	CFileInfoArray(const chcore::TPathContainer& rBasePaths);
	~CFileInfoArray();

	// Adds a new object info to this container
	void AddFileInfo(const CFileInfoPtr& spFileInfo);

	/// Retrieves count of elements in this object
	size_t GetSize() const;

	/// Retrieves an element at the specified index
	CFileInfoPtr GetAt(size_t stIndex) const;

	/// Retrieves a copy of the element at a specified index
	CFileInfo GetCopyAt(size_t stIndex) const;

	/// Removes all elements from this object
	void Clear();

	// specialized operations on contents of m_vFiles
	/// Calculates the size of the first stCount file info objects
	unsigned long long CalculatePartialSize(size_t stCount);

	/// Calculates the size of all file info objects inside this object
	unsigned long long CalculateTotalSize();

	/// Stores infos about elements in the archive
	template<class Archive>
	void Store(Archive& ar, unsigned int /*uiVersion*/, bool bOnlyFlags) const;

	/// Restores info from the archive
	template<class Archive>
	void Load(Archive& ar, unsigned int /*uiVersion*/, bool bOnlyFlags);

protected:
	const chcore::TPathContainer& m_rBasePaths;
	std::vector<CFileInfoPtr> m_vFiles;

	mutable boost::shared_mutex m_lock;
};

template<class Archive>
void CFileInfoArray::Store(Archive& ar, unsigned int /*uiVersion*/, bool bOnlyFlags) const
{
	size_t stCount = m_vFiles.size();
	ar << stCount;
	for(std::vector<CFileInfoPtr>::const_iterator iterFile = m_vFiles.begin(); iterFile != m_vFiles.end(); ++iterFile)
	{
		if(bOnlyFlags)
		{
			uint_t uiFlags = (*iterFile)->GetFlags();
			ar << uiFlags;
		}
		else
			ar << *(*iterFile);
	}
}

template<class Archive>
void CFileInfoArray::Load(Archive& ar, unsigned int /*uiVersion*/, bool bOnlyFlags)
{
	size_t stCount;
	ar >> stCount;

	if(!bOnlyFlags)
	{
		m_vFiles.clear();
		m_vFiles.reserve(stCount);
	}
	else if(stCount != m_vFiles.size())
		THROW(_T("Invalid count of flags received"), 0, 0, 0);

	CFileInfoPtr spFileInfo;

	uint_t uiFlags = 0;
	for(size_t stIndex = 0; stIndex < stCount; stIndex++)
	{
		if(bOnlyFlags)
		{
			CFileInfoPtr& rspFileInfo = m_vFiles.at(stIndex);
			ar >> uiFlags;
			rspFileInfo->SetFlags(uiFlags);
		}
		else
		{
			spFileInfo.reset(new CFileInfo);
			spFileInfo->SetClipboard(&m_rBasePaths);
			ar >> *spFileInfo;
			m_vFiles.push_back(spFileInfo);
		}
	}
}

#endif
