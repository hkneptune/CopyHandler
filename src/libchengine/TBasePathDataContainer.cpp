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
#include "stdafx.h"
#include "TBasePathDataContainer.h"
#include "../libchcore/TCoreException.h"
#include "../libserializer/ISerializerContainer.h"
#include <boost/thread/locks.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include "../libchcore/TPathContainer.h"

using namespace serializer;
using namespace chcore;

namespace chengine
{
	TBasePathDataContainer::TBasePathDataContainer() :
		m_oidLastObjectID(0)
	{
	}

	TBasePathDataContainer::~TBasePathDataContainer()
	{
		try
		{
			// clear works with critical section to avoid destruction while item in use
			Clear();
		}
		catch (const std::exception& e)
		{
		}
	}

	void TBasePathDataContainer::Store(const ISerializerContainerPtr& spContainer) const
	{
		if(!spContainer)
			throw TCoreException(eErr_InvalidPointer, L"spContainer", LOCATION);

		boost::shared_lock<boost::shared_mutex> lock(m_lock);

		InitColumns(spContainer);

		spContainer->DeleteRows(m_setRemovedObjects);
		m_setRemovedObjects.Clear();

		for(const TBasePathDataPtr& spEntry : m_vEntries)
		{
			spEntry->Store(spContainer);
		}
	}

	void TBasePathDataContainer::Load(const ISerializerContainerPtr& spContainer)
	{
		if(!spContainer)
			throw TCoreException(eErr_InvalidPointer, L"spContainer", LOCATION);

		boost::unique_lock<boost::shared_mutex> lock(m_lock);
		m_setRemovedObjects.Clear();
		m_vEntries.clear();

		InitColumns(spContainer);

		ISerializerRowReaderPtr spRowReader = spContainer->GetRowReader();

		while(spRowReader->Next())
		{
			TBasePathDataPtr spPathData(new TBasePathData);

			spPathData->Load(spRowReader);

			m_vEntries.push_back(spPathData);
		}
	}

	void TBasePathDataContainer::Add(const TBasePathDataPtr& spEntry)
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);
		spEntry->SetObjectID(++m_oidLastObjectID);
		m_vEntries.push_back(spEntry);
	}

	void TBasePathDataContainer::RemoveAt(file_count_t fcIndex)
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);
		if(fcIndex >= m_vEntries.size())
			throw TCoreException(eErr_BoundsExceeded, L"fcIndex", LOCATION);

		m_setRemovedObjects.Add(m_vEntries[ boost::numeric_cast<size_t>(fcIndex) ]->GetObjectID());
		m_vEntries.erase(m_vEntries.begin() + boost::numeric_cast<size_t>(fcIndex));
	}

	TBasePathDataPtr TBasePathDataContainer::GetAt(file_count_t fcIndex) const
	{
		boost::shared_lock<boost::shared_mutex> lock(m_lock);
		return m_vEntries.at(boost::numeric_cast<size_t>(fcIndex));
	}

	TBasePathDataPtr TBasePathDataContainer::FindByID(size_t stObjectID) const
	{
		boost::shared_lock<boost::shared_mutex> lock(m_lock);
		for(const TBasePathDataPtr& spItem : m_vEntries)
		{
			if(spItem->GetObjectID() == stObjectID)
				return spItem;
		}

		throw TCoreException(eErr_InvalidArgument, L"Object id does not exist", LOCATION);
	}

	void TBasePathDataContainer::ClearNL()
	{
		for(const TBasePathDataPtr& spItem : m_vEntries)
		{
			m_setRemovedObjects.Add(spItem->GetObjectID());
		}

		m_vEntries.clear();
	}

	void TBasePathDataContainer::Clear()
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);
		ClearNL();
	}

	bool TBasePathDataContainer::IsEmpty() const
	{
		boost::shared_lock<boost::shared_mutex> lock(m_lock);

		return m_vEntries.empty();
	}

	file_count_t TBasePathDataContainer::GetCount() const
	{
		boost::shared_lock<boost::shared_mutex> lock(m_lock);
		return boost::numeric_cast<file_count_t>(m_vEntries.size());
	}

	bool TBasePathDataContainer::AllMarkedAsSkipFurtherProcessing() const
	{
		boost::shared_lock<boost::shared_mutex> lock(m_lock);

		for(const TBasePathDataPtr& spBasePath : m_vEntries)
		{
			if(!spBasePath->GetSkipFurtherProcessing())
				return false;
		}

		return true;
	}

	void TBasePathDataContainer::ResetProcessingFlags()
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);

		for(const TBasePathDataPtr& spBasePath : m_vEntries)
		{
			spBasePath->SetSkipFurtherProcessing(false);
		}
	}

	TBasePathDataContainer& TBasePathDataContainer::operator=(const TPathContainer& tPaths)
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);
		ClearNL();

		for(size_t stIndex = 0; stIndex < tPaths.GetCount(); ++stIndex)
		{
			TSmartPath path = tPaths.GetAt(stIndex);
			path.StripSeparatorAtEnd();

			TBasePathDataPtr spPathData = std::make_shared<TBasePathData>(++m_oidLastObjectID, path);
			m_vEntries.push_back(spPathData);
		}

		return *this;
	}

	void TBasePathDataContainer::InitColumns(const ISerializerContainerPtr& spContainer) const
	{
		IColumnsDefinition& rColumns = spContainer->GetColumnsDefinition();
		if(rColumns.IsEmpty())
			TBasePathData::InitColumns(rColumns);
	}
}
