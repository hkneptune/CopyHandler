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
/// @file  TBasePathData.h
/// @date  2010/10/13
/// @brief Contains declarations of classes related to keeping additional path data.
// ============================================================================
#ifndef __TBASEPATHDATA_H__
#define __TBASEPATHDATA_H__

#include "libchcore.h"
#include "TPath.h"
#include <bitset>
#include "TSharedModificationTracker.h"
#include "TRemovedObjects.h"
#include "ISerializerRowData.h"
#include "IColumnsDefinition.h"
#include "ISerializerRowReader.h"
#include "CommonDataTypes.h"

namespace chcore
{
	class TPathContainer;

	/////////////////////////////////////////////////////////////////////////////
	// TBasePathData
	class LIBCHCORE_API TBasePathData
	{
	private:
		enum EModifications
		{
			eMod_Added,
			eMod_SrcPath,
			eMod_SkipProcessing,
			eMod_DstPath,

			eMod_Last
		};

	public:
		TBasePathData();
		TBasePathData(object_id_t oidObjectID, const TSmartPath& spSrcPath);
		TBasePathData(const TBasePathData& rEntry);

		object_id_t GetObjectID() const;
		void SetObjectID(object_id_t oidObjectID);

		TSmartPath GetSrcPath() const;
		void SetSrcPath(const TSmartPath& pathSrc);

		bool GetSkipFurtherProcessing() const;
		void SetSkipFurtherProcessing(bool bSkipFurtherProcessing);

		void SetDestinationPath(const TSmartPath& strPath);
		TSmartPath GetDestinationPath() const;
		bool IsDestinationPathSet() const;

		void Store(const ISerializerContainerPtr& spContainer) const;
		static void InitColumns(IColumnsDefinition& rColumnDefs);
		void Load(const ISerializerRowReaderPtr& spRowReader);

	private:
#pragma warning(push)
#pragma warning(disable: 4251)
		// modification management
		typedef std::bitset<eMod_Last> BitSet;
		mutable BitSet m_setModifications;

		// attributes
		object_id_t m_oidObjectID;
		TSharedModificationTracker<TSmartPath, BitSet, eMod_SrcPath> m_pathSrc;
		TSharedModificationTracker<bool, BitSet, eMod_SkipProcessing> m_bSkipFurtherProcessing;		// specifies if the path should be (or not) processed further
		TSharedModificationTracker<TSmartPath, BitSet, eMod_DstPath> m_pathDst;
#pragma warning(pop)
	};

	typedef boost::shared_ptr<TBasePathData> TBasePathDataPtr;

	//////////////////////////////////////////////////////////////////////////
	// TBasePathDataContainer

	class LIBCHCORE_API TBasePathDataContainer
	{
	public:
		// constructors/destructor
		TBasePathDataContainer();
		~TBasePathDataContainer();

		TBasePathDataContainer& operator=(const TPathContainer& tPaths);

		// standard access to data
		void Add(const TBasePathDataPtr& spEntry);
		void RemoveAt(file_count_t fcIndex);
		TBasePathDataPtr GetAt(file_count_t fcIndex) const;
		TBasePathDataPtr FindByID(size_t fcObjectID) const;

		void Clear();

		bool IsEmpty() const;
		file_count_t GetCount() const;

		// processing flags
		bool AllMarkedAsSkipFurtherProcessing() const;
		void ResetProcessingFlags();

		void Store(const ISerializerContainerPtr& spContainer) const;
		void Load(const ISerializerContainerPtr& spContainer);

		void InitColumns(const ISerializerContainerPtr& spContainer) const;

	private:
		TBasePathDataContainer(const TBasePathDataContainer& rSrc);
		TBasePathDataContainer& operator=(const TBasePathDataContainer& rSrc);

		void ClearNL();

	protected:
#pragma warning(push)
#pragma warning(disable: 4251)
		typedef std::vector<TBasePathDataPtr> VecEntries;
		VecEntries m_vEntries;
		mutable TRemovedObjects m_setRemovedObjects;

		mutable boost::shared_mutex m_lock;
#pragma warning(pop)
		object_id_t m_oidLastObjectID;
	};

	typedef boost::shared_ptr<TBasePathDataContainer> TBasePathDataContainerPtr;
}

#endif // __TBASEPATHDATA_H__
