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
/// @file  TBasePathData.cpp
/// @date  2010/10/13
/// @brief Contains implementations of classes related to keeping path data.
// ============================================================================
#include "stdafx.h"
#include "TBasePathData.h"
#include "TCoreException.h"
#include "ErrorCodes.h"
#include "ISerializerContainer.h"
#include "ISerializerRowData.h"
#include <boost/make_shared.hpp>
#include "TPathContainer.h"
#include <boost/numeric/conversion/cast.hpp>

namespace chcore
{
	//////////////////////////////////////////////////////////////////////////////
	// TBasePathData

	TBasePathData::TBasePathData() :
		m_oidObjectID(0),
		m_pathSrc(m_setModifications),
		m_bSkipFurtherProcessing(m_setModifications, false),
		m_pathDst(m_setModifications)
	{
		m_setModifications[eMod_Added] = true;
	}

	TBasePathData::TBasePathData(const TBasePathData& rEntry) :
		m_oidObjectID(rEntry.m_oidObjectID),
		m_pathSrc(m_setModifications, rEntry.m_pathSrc),
		m_pathDst(m_setModifications, rEntry.m_pathDst),
		m_bSkipFurtherProcessing(m_setModifications, rEntry.m_bSkipFurtherProcessing),
		m_setModifications(rEntry.m_setModifications)
	{
	}

	TBasePathData::TBasePathData(object_id_t oidObjectID, const TSmartPath& spSrcPath) :
		m_oidObjectID(oidObjectID),
		m_pathSrc(m_setModifications, spSrcPath),
		m_bSkipFurtherProcessing(m_setModifications, false),
		m_pathDst(m_setModifications)
	{
		m_pathSrc.Modify().StripSeparatorAtEnd();
		m_setModifications[eMod_Added] = true;
	}

	void TBasePathData::SetDestinationPath(const TSmartPath& tPath)
	{
		m_pathDst = tPath;
		m_pathDst.Modify().StripSeparatorAtEnd();
	}

	TSmartPath TBasePathData::GetDestinationPath() const
	{
		return m_pathDst;
	}

	bool TBasePathData::GetSkipFurtherProcessing() const
	{
		return m_bSkipFurtherProcessing;
	}

	void TBasePathData::SetSkipFurtherProcessing(bool bSkipFurtherProcessing)
	{
		m_bSkipFurtherProcessing = bSkipFurtherProcessing;
	}

	bool TBasePathData::IsDestinationPathSet() const
	{
		return !m_pathDst.Get().IsEmpty();
	}

	void TBasePathData::Store(const ISerializerContainerPtr& spContainer) const
	{
		if (!spContainer)
			throw TCoreException(eErr_InvalidPointer, L"spContainer", LOCATION);

		bool bAdded = m_setModifications[eMod_Added];
		if (m_setModifications.any())
		{
			ISerializerRowData& rRow = spContainer->GetRow(m_oidObjectID, bAdded);
			if (bAdded || m_setModifications[eMod_SrcPath])
				rRow.SetValue(_T("src_path"), m_pathSrc);
			if (bAdded || m_setModifications[eMod_SkipProcessing])
				rRow.SetValue(_T("skip_processing"), m_bSkipFurtherProcessing);
			if (bAdded || m_setModifications[eMod_DstPath])
				rRow.SetValue(_T("dst_path"), m_pathDst);

			m_setModifications.reset();
		}
	}

	void TBasePathData::InitColumns(IColumnsDefinition& rColumns)
	{
		rColumns.AddColumn(_T("id"), ColumnType<object_id_t>::value);
		rColumns.AddColumn(_T("src_path"), IColumnsDefinition::eType_path);
		rColumns.AddColumn(_T("skip_processing"), IColumnsDefinition::eType_bool);
		rColumns.AddColumn(_T("dst_path"), IColumnsDefinition::eType_path);
	}

	void TBasePathData::Load(const ISerializerRowReaderPtr& spRowReader)
	{
		spRowReader->GetValue(_T("id"), m_oidObjectID);
		spRowReader->GetValue(_T("src_path"), m_pathSrc.Modify());
		spRowReader->GetValue(_T("skip_processing"), m_bSkipFurtherProcessing.Modify());
		spRowReader->GetValue(_T("dst_path"), m_pathDst.Modify());

		m_pathSrc.Modify().StripSeparatorAtEnd();
		m_pathDst.Modify().StripSeparatorAtEnd();

		m_setModifications.reset();
	}

	TSmartPath TBasePathData::GetSrcPath() const
	{
		return m_pathSrc;
	}

	void TBasePathData::SetSrcPath(const TSmartPath& pathSrc)
	{
		m_pathSrc = pathSrc;
		m_pathSrc.Modify().StripSeparatorAtEnd();
	}

	object_id_t TBasePathData::GetObjectID() const
	{
		return m_oidObjectID;
	}

	void TBasePathData::SetObjectID(object_id_t oidObjectID)
	{
		m_oidObjectID = oidObjectID;
	}

	//////////////////////////////////////////////////////////////////////////////
	// TBasePathDataContainer

	TBasePathDataContainer::TBasePathDataContainer() :
		m_oidLastObjectID(0)
	{
	}

	TBasePathDataContainer::~TBasePathDataContainer()
	{
		// clear works with critical section to avoid destruction while item in use
		Clear();
	}

	void TBasePathDataContainer::Store(const ISerializerContainerPtr& spContainer) const
	{
		if (!spContainer)
			throw TCoreException(eErr_InvalidPointer, L"spContainer", LOCATION);

		boost::shared_lock<boost::shared_mutex> lock(m_lock);

		InitColumns(spContainer);

		spContainer->DeleteRows(m_setRemovedObjects);
		m_setRemovedObjects.Clear();

		BOOST_FOREACH(const TBasePathDataPtr& spEntry, m_vEntries)
		{
			spEntry->Store(spContainer);
		}
	}

	void TBasePathDataContainer::Load(const ISerializerContainerPtr& spContainer)
	{
		if (!spContainer)
			throw TCoreException(eErr_InvalidPointer, L"spContainer", LOCATION);

		boost::unique_lock<boost::shared_mutex> lock(m_lock);
		m_setRemovedObjects.Clear();
		m_vEntries.clear();

		InitColumns(spContainer);

		ISerializerRowReaderPtr spRowReader = spContainer->GetRowReader();

		while (spRowReader->Next())
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
		if (fcIndex >= m_vEntries.size())
			throw TCoreException(eErr_BoundsExceeded, L"fcIndex", LOCATION);

		m_setRemovedObjects.Add(m_vEntries[boost::numeric_cast<size_t>(fcIndex)]->GetObjectID());
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
		BOOST_FOREACH(const TBasePathDataPtr& spItem, m_vEntries)
		{
			if (spItem->GetObjectID() == stObjectID)
				return spItem;
		}

		throw TCoreException(eErr_InvalidArgument, L"Object id does not exist", LOCATION);
	}

	void TBasePathDataContainer::ClearNL()
	{
		BOOST_FOREACH(const TBasePathDataPtr& spItem, m_vEntries)
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

		for (const TBasePathDataPtr& spBasePath : m_vEntries)
		{
			if (!spBasePath->GetSkipFurtherProcessing())
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

		for (size_t stIndex = 0; stIndex < tPaths.GetCount(); ++stIndex)
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
		if (rColumns.IsEmpty())
			TBasePathData::InitColumns(rColumns);
	}
}
