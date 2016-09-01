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
#include "TBasePathData.h"
#include <bitset>
#include "TSharedModificationTracker.h"
#include "TFileTime.h"

namespace chcore
{
	class LIBCHCORE_API TFileInfo
	{
	public:
		enum EFlags
		{
			// flag stating that file has been processed (used to determine if file can be deleted at the end of copying)
			eFlag_Processed = 1,
		};

	public:
		TFileInfo();
		TFileInfo(const TFileInfo& rSrc);
		TFileInfo(const TBasePathDataPtr& spBasePathData, const TSmartPath& rpathFile,
			DWORD dwAttributes, ULONGLONG uhFileSize, const TFileTime& ftCreation, const TFileTime& ftLastAccess, const TFileTime& ftLastWrite,
			unsigned int uiFlags);
		~TFileInfo();

		TFileInfo& operator=(const TFileInfo& rSrc);

		// operators
		bool operator==(const TFileInfo& rInfo) const;

		// with base path
		void Init(const TBasePathDataPtr& spBasePathData, const TSmartPath& rpathFile,
			DWORD dwAttributes, ULONGLONG uhFileSize, const TFileTime& ftCreation, const TFileTime& ftLastAccess, const TFileTime& ftLastWrite,
			unsigned int uiFlags);

		// without base path
		void Init(const TSmartPath& rpathFile, DWORD dwAttributes, ULONGLONG uhFileSize, const TFileTime& ftCreation,
			const TFileTime& ftLastAccess, const TFileTime& ftLastWrite, unsigned int uiFlags);

		// unique object id
		object_id_t GetObjectID() const;
		void SetObjectID(object_id_t oidObjectID);

		// parent object
		TBasePathDataPtr GetBasePathData() const;
		void SetParentObject(const TBasePathDataPtr& spBasePathData);
		object_id_t GetSrcObjectID() const;

		// file path
		const TSmartPath& GetFilePath() const;	// returns path with m_pathFile (probably not full)
		TSmartPath GetFullFilePath() const;		// returns full path
		void SetFilePath(const TSmartPath& tPath);

		// file size
		ULONGLONG GetLength64() const;
		void SetLength64(ULONGLONG uhSize);

		// file times
		void SetFileTimes(const TFileTime& rCreation, const TFileTime& rLastAccess, const TFileTime& rLastWrite);
		const TFileTime& GetCreationTime() const;
		const TFileTime& GetLastAccessTime() const;
		const TFileTime& GetLastWriteTime() const;

		// attributes
		DWORD GetAttributes() const;
		void SetAttributes(DWORD dwAttributes);
		bool IsDirectory() const;
		bool IsArchived() const;
		bool IsReadOnly() const;
		bool IsCompressed() const;
		bool IsSystem() const;
		bool IsHidden() const;
		bool IsTemporary() const;
		bool IsNormal() const;

		void MarkAsProcessed(bool bProcessed);
		bool IsProcessed() const;
		bool IsBasePathProcessed() const;

		void Store(const ISerializerContainerPtr& spContainer) const;
		static void InitColumns(IColumnsDefinition& rColumns);
		void Load(const ISerializerRowReaderPtr& spRowReader, const TBasePathDataContainerPtr& spSrcContainer);

	private:
		enum EModifications
		{
			eMod_None = 0,
			eMod_Added,
			eMod_Path,
			eMod_BasePath,
			eMod_Attributes,
			eMod_FileSize,
			eMod_TimeCreated,
			eMod_TimeLastAccess,
			eMod_TimeLastWrite,
			eMod_Flags,

			// do not use - must be the last value in this enum
			eMod_Last
		};

#pragma warning(push)
#pragma warning(disable: 4251)
		typedef std::bitset<eMod_Last> Bitset;
		mutable Bitset m_setModifications;

		object_id_t m_oidObjectID;

		TSharedModificationTracker<TSmartPath, Bitset, eMod_Path> m_pathFile;
		TSharedModificationTracker<TBasePathDataPtr, Bitset, eMod_BasePath> m_spBasePathData;
		TSharedModificationTracker<DWORD, Bitset, eMod_Attributes> m_dwAttributes;	// attributes
		TSharedModificationTracker<ULONGLONG, Bitset, eMod_FileSize> m_uhFileSize;
		TSharedModificationTracker<TFileTime, Bitset, eMod_TimeCreated> m_ftCreation;
		TSharedModificationTracker<TFileTime, Bitset, eMod_TimeLastAccess> m_ftLastAccess;
		TSharedModificationTracker<TFileTime, Bitset, eMod_TimeLastWrite> m_ftLastWrite;
		TSharedModificationTracker<unsigned int, Bitset, eMod_Flags> m_uiFlags;
#pragma warning(pop)
	};

	typedef std::shared_ptr<TFileInfo> TFileInfoPtr;
}

#endif
