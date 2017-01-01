// ============================================================================
//  Copyright (C) 2001-2017 by Jozef Starosczyk
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
#ifndef __TBASEPATHDATACONTAINER_H__
#define __TBASEPATHDATACONTAINER_H__

#include "libchengine.h"
#include "TBasePathData.h"
#include <vector>
#include <boost/thread/shared_mutex.hpp>

namespace chcore {
	class TPathContainer;
}

namespace chengine
{
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

#endif
