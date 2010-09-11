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

#include "DestPath.h"
#include <boost/shared_ptr.hpp>
#include <boost/foreach.hpp>

void FindFreeSubstituteName(CString strSrcPath, CString strDstPath, CString* pstrResult);
extern void GetDriveData(LPCTSTR lpszPath, int *piDrvNum, UINT *puiDrvType);

// CFileInfo flags
// flag stating that file has been processed (used to determine if file can be deleted at the end of copying)
#define FIF_PROCESSED		0x00000001

class CFiltersArray;
/////////////////////////////////////////////////////////////////////////////
// CClipboardEntry
class CClipboardEntry
{
public:
	CClipboardEntry();
	CClipboardEntry(const CClipboardEntry& rEntry);

	void SetPath(const CString& strPath);
	void CalcBufferIndex(const CDestPath& dpDestPath);
	const CString& GetPath() const { return m_strPath; }
	CString GetFileName() const;

	void SetMove(bool bValue) { m_bMove=bValue; }
	bool GetMove() { return m_bMove; }

	int GetDriveNumber() const { return m_iDriveNumber; }
	UINT GetDriveType() const { return m_uiDriveType; }

	int GetBufferIndex() const { return m_iBufferIndex; }

	template<class Archive>
	void Serialize(Archive& ar, unsigned int /*uiVersion*/, bool bData)
	{
		if(bData)
		{
			ar & m_strPath;
			ar & m_bMove;
			ar & m_iDriveNumber;
			ar & m_uiDriveType;
			ar & m_iBufferIndex;
		}
		else
			ar & m_strDstPath;
	}

	void SetDestinationPath(const CString& strPath);
	CString GetDestinationPath();
	bool IsDestinationPathSet() const { return !m_strDstPath.IsEmpty(); }

private:
	CString m_strPath;				// path (ie. c:\\windows\\) - always with ending '\\'
	bool m_bMove;					// specifies if we can use MoveFile (if will be moved)

	int m_iDriveNumber;		// disk number (-1 - none)
	UINT m_uiDriveType;		// path type

	int m_iBufferIndex;		// buffer number, with which we'll copy this data

	CString m_strDstPath;	// dest path
};

typedef boost::shared_ptr<CClipboardEntry> CClipboardEntryPtr;

//////////////////////////////////////////////////////////////////////////
// CClipboardArray

class CClipboardArray
{
public:
	~CClipboardArray();

	template<class Archive>
	void Store(Archive& ar, unsigned int /*uiVersion*/, bool bData) const
	{
		boost::shared_lock<boost::shared_mutex> lock(m_lock);
		// write data
		size_t stCount = m_vEntries.size();
		ar << stCount;
		
		BOOST_FOREACH(const CClipboardEntryPtr& spEntry, m_vEntries)
		{
			spEntry->Serialize(ar, 0, bData);
		}
	}

	template<class Archive>
	void Load(Archive& ar, unsigned int /*uiVersion*/, bool bData)
	{
		size_t stCount;
		ar >> stCount;

		boost::unique_lock<boost::shared_mutex> lock(m_lock);

		if(!bData && m_vEntries.size() != stCount)
			THROW(_T("Count of entries with data differs from the count of state entries"), 0, 0, 0);

		if(bData)
		{
			m_vEntries.clear();
			m_vEntries.reserve(stCount);
		}

		CClipboardEntryPtr spEntry;
		for(size_t stIndex = 0; stIndex < stCount; ++stIndex)
		{
			if(bData)
				spEntry.reset(new CClipboardEntry);
			else
				spEntry = m_vEntries.at(stIndex);
			spEntry->Serialize(ar, 0, bData);

			if(bData)
				m_vEntries.push_back(spEntry);
		}
	}

	CClipboardEntryPtr GetAt(size_t iPos) const;

	size_t GetSize() const;
	void Add(const CClipboardEntryPtr& pEntry);
	void SetAt(size_t nIndex, const CClipboardEntryPtr& pEntry);
	void RemoveAt(size_t nIndex, size_t nCount = 1);
	void RemoveAll();

	int ReplacePathsPrefix(CString strOld, CString strNew);

protected:
	std::vector<CClipboardEntryPtr> m_vEntries;
	mutable boost::shared_mutex m_lock;
};

class CFileInfo
{  
public:
	CFileInfo();
	CFileInfo(const CFileInfo& finf);
	~CFileInfo();

	// static member
	static bool Exist(CString strPath);	// check for file or folder existence

	void Create(const WIN32_FIND_DATA* pwfd, LPCTSTR pszFilePath, size_t stSrcIndex);
	bool Create(CString strFilePath, size_t stSrcIndex);

	ULONGLONG GetLength64() const { return m_uhFileSize; }
	void SetLength64(ULONGLONG uhSize) { m_uhFileSize=uhSize; }

	// disk - path and disk number (-1 if none - ie. net disk)
	CString GetFileDrive(void) const;		// returns string with src disk
	int GetDriveNumber() const;				// disk number A - 0, b-1, c-2, ...
	UINT GetDriveType() const;				// drive type

	CString GetFileDir() const;	// @rdesc Returns \WINDOWS\ for C:\WINDOWS\WIN.INI 
	CString GetFileTitle() const;	// @cmember returns WIN for C:\WINDOWS\WIN.INI
	CString GetFileExt() const;		/** @cmember returns INI for C:\WINDOWS\WIN.INI */
	CString GetFileRoot() const;	/** @cmember returns C:\WINDOWS\ for C:\WINDOWS\WIN.INI */
	CString GetFileName() const;	/** @cmember returns WIN.INI for C:\WINDOWS\WIN.INI */

	const CString& GetFilePath(void) const { return m_strFilePath; }	// returns path with m_strFilePath (probably not full)
	CString GetFullFilePath() const;		/** @cmember returns C:\WINDOWS\WIN.INI for C:\WINDOWS\WIN.INI */
	void SetFilePath(LPCTSTR lpszPath) { m_strFilePath=lpszPath; };

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
	void SetClipboard(CClipboardArray *pClipboard) { m_pClipboard=pClipboard; };
	CString GetDestinationPath(CString strPath, int iFlags);

	void SetSrcIndex(size_t stIndex) { m_stSrcIndex = stIndex; };
	size_t GetSrcIndex() const { return m_stSrcIndex; };

	bool GetMove() { if (m_stSrcIndex != std::numeric_limits<size_t>::max()) return m_pClipboard->GetAt(m_stSrcIndex)->GetMove(); else return true; };

	int GetBufferIndex() const;

	// operators
	bool operator==(const CFileInfo& rInfo);

	template<class Archive>
	void serialize(Archive& ar, unsigned int /*uiVersion*/)
	{
		ar & m_strFilePath;
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
	CString m_strFilePath;	// contains relative path (first path is in CClipboardArray)
	size_t m_stSrcIndex;		// index in CClipboardArray table (which contains the first part of the path)

	DWORD m_dwAttributes;	// attributes
	ULONGLONG m_uhFileSize;
	FILETIME  m_ftCreation;
	FILETIME  m_ftLastAccess;
	FILETIME  m_ftLastWrite;

	uint_t m_uiFlags;

	// ptrs to elements providing data
	CClipboardArray* m_pClipboard;
};

typedef boost::shared_ptr<CFileInfo> CFileInfoPtr;

class CFileInfoArray
{
public:
	CFileInfoArray(CClipboardArray& rClipboardArray);
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
	void Store(Archive& ar, unsigned int /*uiVersion*/, bool bOnlyFlags);

	/// Restores info from the archive
	template<class Archive>
	void Load(Archive& ar, unsigned int /*uiVersion*/, bool bOnlyFlags);

protected:
	CClipboardArray& m_rClipboard;
	std::vector<CFileInfoPtr> m_vFiles;
	mutable boost::shared_mutex m_lock;
};

template<class Archive>
void CFileInfoArray::Store(Archive& ar, unsigned int /*uiVersion*/, bool bOnlyFlags)
{
	size_t stCount = m_vFiles.size();
	ar << stCount;
	for(std::vector<CFileInfoPtr>::iterator iterFile = m_vFiles.begin(); iterFile != m_vFiles.end(); ++iterFile)
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
			CFileInfoPtr& spFileInfo = m_vFiles.at(stIndex);
			ar >> uiFlags;
			spFileInfo->SetFlags(uiFlags);
		}
		else
		{
			spFileInfo.reset(new CFileInfo);
			spFileInfo->SetClipboard(&m_rClipboard);
			ar >> *spFileInfo;
			m_vFiles.push_back(spFileInfo);
		}
	}
}

#endif
