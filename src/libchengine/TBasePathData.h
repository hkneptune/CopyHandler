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

#include "../libchcore/TPath.h"
#include <bitset>
#include "../libserializer/TRemovedObjects.h"
#include "../libserializer/ISerializerRowData.h"
#include "../libserializer/ISerializerRowReader.h"
#include "CommonDataTypes.h"
#include "libchengine.h"
#include "../libserializer/TSharedModificationTracker.h"

namespace serializer
{
	class IColumnsDefinition;
}

namespace chengine
{
	/////////////////////////////////////////////////////////////////////////////
	// TBasePathData
	class LIBCHENGINE_API TBasePathData
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
		TBasePathData(serializer::object_id_t oidObjectID, const chcore::TSmartPath& spSrcPath);
		TBasePathData(const TBasePathData& rEntry);

		serializer::object_id_t GetObjectID() const;
		void SetObjectID(serializer::object_id_t oidObjectID);

		chcore::TSmartPath GetSrcPath() const;
		void SetSrcPath(const chcore::TSmartPath& pathSrc);

		bool GetSkipFurtherProcessing() const;
		void SetSkipFurtherProcessing(bool bSkipFurtherProcessing);

		void SetDestinationPath(const chcore::TSmartPath& strPath);
		chcore::TSmartPath GetDestinationPath() const;
		bool IsDestinationPathSet() const;

		void Store(const serializer::ISerializerContainerPtr& spContainer) const;
		static void InitColumns(serializer::IColumnsDefinition& rColumnDefs);
		void Load(const serializer::ISerializerRowReaderPtr& spRowReader);

	private:
#pragma warning(push)
#pragma warning(disable: 4251)
		// modification management
		typedef std::bitset<eMod_Last> BitSet;
		mutable BitSet m_setModifications;

		// attributes
		serializer::object_id_t m_oidObjectID;
		serializer::TSharedModificationTracker<chcore::TSmartPath, BitSet, eMod_SrcPath> m_pathSrc;
		serializer::TSharedModificationTracker<bool, BitSet, eMod_SkipProcessing> m_bSkipFurtherProcessing;		// specifies if the path should be (or not) processed further
		serializer::TSharedModificationTracker<chcore::TSmartPath, BitSet, eMod_DstPath> m_pathDst;
#pragma warning(pop)
	};

	typedef std::shared_ptr<TBasePathData> TBasePathDataPtr;

	//////////////////////////////////////////////////////////////////////////
	// TBasePathDataContainer

	class LIBCHENGINE_API TBasePathDataContainer
	{
	public:
		// constructors/destructor
		TBasePathDataContainer();
		TBasePathDataContainer(const TBasePathDataContainer& rSrc) = delete;
		~TBasePathDataContainer();

		TBasePathDataContainer& operator=(const TBasePathDataContainer& rSrc) = delete;

		TBasePathDataContainer& operator=(const chcore::TPathContainer& tPaths);

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

		void Store(const serializer::ISerializerContainerPtr& spContainer) const;
		void Load(const serializer::ISerializerContainerPtr& spContainer);

		void InitColumns(const serializer::ISerializerContainerPtr& spContainer) const;

	private:
		void ClearNL();

	protected:
#pragma warning(push)
#pragma warning(disable: 4251)
		typedef std::vector<TBasePathDataPtr> VecEntries;
		VecEntries m_vEntries;
		mutable serializer::TRemovedObjects m_setRemovedObjects;

		mutable boost::shared_mutex m_lock;
#pragma warning(pop)
		serializer::object_id_t m_oidLastObjectID;
	};

	typedef std::shared_ptr<TBasePathDataContainer> TBasePathDataContainerPtr;
}

#endif // __TBASEPATHDATA_H__
