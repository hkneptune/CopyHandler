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

#include "../libchcore/TPath.h"
#include "TBasePathData.h"
#include <bitset>
#include "../libchcore/TFileTime.h"
#include "TBasePathDataContainer.h"

namespace chengine
{
	class LIBCHENGINE_API TFileInfo
	{
	public:
		enum EFlags
		{
			// flag stating that file has been processed (used to determine if file can be deleted at the end of copying)
			eFlag_Processed = 1,
		};

	public:
		TFileInfo();
		TFileInfo(const TBasePathDataPtr& spBasePathData, const chcore::TSmartPath& rpathFile,
			DWORD dwAttributes, ULONGLONG uhFileSize, const chcore::TFileTime& ftCreation, const chcore::TFileTime& ftLastAccess, const chcore::TFileTime& ftLastWrite,
			unsigned int uiFlags);
		~TFileInfo();

		// operators
		bool operator==(const TFileInfo& rInfo) const;

		// with base path
		void Init(const TBasePathDataPtr& spBasePathData, const chcore::TSmartPath& rpathFile,
			DWORD dwAttributes, ULONGLONG uhFileSize, const chcore::TFileTime& ftCreation, const chcore::TFileTime& ftLastAccess, const chcore::TFileTime& ftLastWrite,
			unsigned int uiFlags);

		// without base path
		void Init(const chcore::TSmartPath& rpathFile, DWORD dwAttributes, ULONGLONG uhFileSize, const chcore::TFileTime& ftCreation,
			const chcore::TFileTime& ftLastAccess, const chcore::TFileTime& ftLastWrite, unsigned int uiFlags);

		// unique object id
		serializer::object_id_t GetObjectID() const;
		void SetObjectID(serializer::object_id_t oidObjectID);

		// parent object
		TBasePathDataPtr GetBasePathData() const;
		void SetParentObject(const TBasePathDataPtr& spBasePathData);
		serializer::object_id_t GetSrcObjectID() const;

		// file path
		const chcore::TSmartPath& GetFilePath() const;	// returns path with m_pathFile (probably not full)
		chcore::TSmartPath GetFullFilePath() const;		// returns full path
		void SetFilePath(const chcore::TSmartPath& tPath);

		// dst file path
		const chcore::TSmartPath& GetDstRelativePath() const { return m_pathRelativeDstFile; }
		void SetDstRelativePath(const chcore::TSmartPath& tPath) { m_pathRelativeDstFile = tPath; }

		// file size
		ULONGLONG GetLength64() const;
		void SetLength64(ULONGLONG uhSize);

		// file times
		void SetFileTimes(const chcore::TFileTime& rCreation, const chcore::TFileTime& rLastAccess, const chcore::TFileTime& rLastWrite);
		const chcore::TFileTime& GetCreationTime() const;
		const chcore::TFileTime& GetLastAccessTime() const;
		const chcore::TFileTime& GetLastWriteTime() const;

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

		void Store(const serializer::ISerializerContainerPtr& spContainer) const;
		static void InitColumns(serializer::IColumnsDefinition& rColumns);
		void Load(const serializer::ISerializerRowReaderPtr& spRowReader, const TBasePathDataContainerPtr& spSrcContainer);

		void MarkAsAdded() { m_setModifications[eMod_Added] = true; }
		bool IsAdded() const { return m_setModifications[eMod_Added]; }

	private:
		enum EModifications
		{
			eMod_None = 0,
			eMod_Added,
			eMod_Path,
			eMod_DstRelativePath,
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

		serializer::object_id_t m_oidObjectID;

		serializer::TSharedModificationTracker<chcore::TSmartPath, Bitset, eMod_Path> m_pathFile;
		serializer::TSharedModificationTracker<chcore::TSmartPath, Bitset, eMod_DstRelativePath> m_pathRelativeDstFile;
		serializer::TSharedModificationTracker<TBasePathDataPtr, Bitset, eMod_BasePath> m_spBasePathData;
		serializer::TSharedModificationTracker<DWORD, Bitset, eMod_Attributes> m_dwAttributes;	// attributes
		serializer::TSharedModificationTracker<ULONGLONG, Bitset, eMod_FileSize> m_uhFileSize;
		serializer::TSharedModificationTracker<chcore::TFileTime, Bitset, eMod_TimeCreated> m_ftCreation;
		serializer::TSharedModificationTracker<chcore::TFileTime, Bitset, eMod_TimeLastAccess> m_ftLastAccess;
		serializer::TSharedModificationTracker<chcore::TFileTime, Bitset, eMod_TimeLastWrite> m_ftLastWrite;
		serializer::TSharedModificationTracker<unsigned int, Bitset, eMod_Flags> m_uiFlags;
#pragma warning(pop)
	};

	typedef std::shared_ptr<TFileInfo> TFileInfoPtr;
}

#endif
