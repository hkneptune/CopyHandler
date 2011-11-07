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

#ifndef __TFILEINFO_H__
#define __TFILEINFO_H__

#include "libchcore.h"
#include "TPath.h"

BEGIN_CHCORE_NAMESPACE

class TPathContainer;
class TReadBinarySerializer;
class TWriteBinarySerializer;

// CFileInfo flags
// flag stating that file has been processed (used to determine if file can be deleted at the end of copying)
#define FIF_PROCESSED		0x00000001

class LIBCHCORE_API TFileInfo
{  
public:
	TFileInfo();
	TFileInfo(const TFileInfo& finf);
	~TFileInfo();

	// with base path
	void Init(const TSmartPath& rpathFile, size_t stSrcIndex, const TPathContainer* pBasePaths,
		DWORD dwAttributes, ULONGLONG uhFileSize, FILETIME ftCreation, FILETIME ftLastAccess, FILETIME ftLastWrite,
		uint_t uiFlags);

	// without base path
	void Init(const TSmartPath& rpathFile, DWORD dwAttributes, ULONGLONG uhFileSize, FILETIME ftCreation,
		FILETIME ftLastAccess, FILETIME ftLastWrite, uint_t uiFlags);

	// setting parent object
	void SetParentObject(size_t stIndex, const TPathContainer* pBasePaths);

	ULONGLONG GetLength64() const { return m_uhFileSize; }
	void SetLength64(ULONGLONG uhSize) { m_uhFileSize=uhSize; }

	const TSmartPath& GetFilePath() const { return m_pathFile; }	// returns path with m_pathFile (probably not full)
	TSmartPath GetFullFilePath() const;		// returns full path
	void SetFilePath(const TSmartPath& tPath) { m_pathFile = tPath; };

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
	void SetClipboard(const TPathContainer* pBasePaths) { m_pBasePaths = pBasePaths; }

	void SetSrcIndex(size_t stIndex) { m_stSrcIndex = stIndex; };
	size_t GetSrcIndex() const { return m_stSrcIndex; };

	// operators
	bool operator==(const TFileInfo& rInfo);

	void Serialize(TReadBinarySerializer& rSerializer);
	void Serialize(TWriteBinarySerializer& rSerializer) const;

private:
	TSmartPath m_pathFile;	// contains relative path (first path is in CClipboardArray)

	size_t m_stSrcIndex;		// index in CClipboardArray table (which contains the first part of the path)
	const TPathContainer* m_pBasePaths;

	DWORD m_dwAttributes;	// attributes
	ULONGLONG m_uhFileSize;
	FILETIME  m_ftCreation;
	FILETIME  m_ftLastAccess;
	FILETIME  m_ftLastWrite;

	uint_t m_uiFlags;
};

typedef boost::shared_ptr<TFileInfo> TFileInfoPtr;

END_CHCORE_NAMESPACE

#endif
