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
#include "TBinarySerializer.h"
#include "SerializationHelpers.h"
#include "TCoreException.h"
#include "ErrorCodes.h"
#include "ISerializerContainer.h"
#include "ISerializerRowData.h"
#include <boost/make_shared.hpp>
#include "TPathContainer.h"

BEGIN_CHCORE_NAMESPACE

//////////////////////////////////////////////////////////////////////////////
// TBasePathData

TBasePathData::TBasePathData() :
	m_stObjectID(0),
	m_pathSrc(m_setModifications),
	m_bSkipFurtherProcessing(m_setModifications, false),
	m_pathDst(m_setModifications)
{
	m_setModifications[eMod_Added] = true;
}

TBasePathData::TBasePathData(const TBasePathData& rEntry) :
	m_stObjectID(rEntry.m_stObjectID),
	m_pathSrc(rEntry.m_pathSrc),
	m_pathDst(rEntry.m_pathDst),
	m_bSkipFurtherProcessing(rEntry.m_bSkipFurtherProcessing),
	m_setModifications(rEntry.m_setModifications)
{
}

TBasePathData::TBasePathData(size_t stObjectID, const TSmartPath& spSrcPath) :
	m_stObjectID(stObjectID),
	m_pathSrc(m_setModifications, spSrcPath),
	m_bSkipFurtherProcessing(m_setModifications, false),
	m_pathDst(m_setModifications)
{
	m_setModifications[eMod_Added] = true;
}

void TBasePathData::SetDestinationPath(const TSmartPath& tPath)
{
	m_pathDst = tPath;
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
	if(!spContainer)
		THROW_CORE_EXCEPTION(eErr_InvalidPointer);

	bool bAdded = m_setModifications[eMod_Added];
	if(m_setModifications.any())
	{
		ISerializerRowData& rRow = spContainer->GetRow(m_stObjectID, bAdded);
		if(bAdded || m_setModifications[eMod_SrcPath])
			rRow.SetValue(_T("src_path"), m_pathSrc);
		if(bAdded || m_setModifications[eMod_SkipProcessing])
			rRow.SetValue(_T("skip_processing"), m_bSkipFurtherProcessing);
		if(bAdded || m_setModifications[eMod_DstPath])
			rRow.SetValue(_T("dst_path"), m_pathDst);

		m_setModifications.reset();
	}
}

void TBasePathData::InitColumns(IColumnsDefinition& rColumns)
{
	rColumns.AddColumn(_T("id"), IColumnsDefinition::eType_ulonglong);
	rColumns.AddColumn(_T("src_path"), IColumnsDefinition::eType_path);
	rColumns.AddColumn(_T("skip_processing"), IColumnsDefinition::eType_bool);
	rColumns.AddColumn(_T("dst_path"), IColumnsDefinition::eType_path);
}

void TBasePathData::Load(const ISerializerRowReaderPtr& spRowReader)
{
	spRowReader->GetValue(_T("id"), m_stObjectID);
	spRowReader->GetValue(_T("src_path"), m_pathSrc.Modify());
	spRowReader->GetValue(_T("skip_processing"), m_bSkipFurtherProcessing.Modify());
	spRowReader->GetValue(_T("dst_path"), m_pathDst.Modify());
	m_setModifications.reset();
}

TSmartPath TBasePathData::GetSrcPath() const
{
	return m_pathSrc;
}

void TBasePathData::SetSrcPath(const TSmartPath& pathSrc)
{
	m_pathSrc = pathSrc;
}

size_t TBasePathData::GetObjectID() const
{
	return m_stObjectID;
}

void TBasePathData::SetObjectID(size_t stObjectID)
{
	m_stObjectID = stObjectID;
}

//////////////////////////////////////////////////////////////////////////////
// TBasePathDataContainer

TBasePathDataContainer::TBasePathDataContainer() :
	m_stLastObjectID(0)
{
}

TBasePathDataContainer::~TBasePathDataContainer()
{
	// clear works with critical section to avoid destruction while item in use
	Clear();
}

void TBasePathDataContainer::Store(const ISerializerContainerPtr& spContainer) const
{
	if(!spContainer)
		THROW_CORE_EXCEPTION(eErr_InvalidPointer);

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
	if(!spContainer)
		THROW_CORE_EXCEPTION(eErr_InvalidPointer);

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
	spEntry->SetObjectID(++m_stLastObjectID);
	m_vEntries.push_back(spEntry);
}

void TBasePathDataContainer::RemoveAt(size_t stIndex)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	if(stIndex >= m_vEntries.size())
		THROW_CORE_EXCEPTION(eErr_BoundsExceeded);

	m_setRemovedObjects.Add(m_vEntries[stIndex]->GetObjectID());
	m_vEntries.erase(m_vEntries.begin() + stIndex);
}

TBasePathDataPtr TBasePathDataContainer::GetAt(size_t stIndex) const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_vEntries.at(stIndex);
}


TBasePathDataPtr TBasePathDataContainer::FindByID(size_t stObjectID) const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	BOOST_FOREACH(const TBasePathDataPtr& spItem, m_vEntries)
	{
		if(spItem->GetObjectID() == stObjectID)
			return spItem;
	}

	THROW_CORE_EXCEPTION(eErr_InvalidArgument);
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

size_t TBasePathDataContainer::GetCount() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_vEntries.size();
}

TBasePathDataContainer& TBasePathDataContainer::operator=(const TPathContainer& tPaths)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	ClearNL();

	for(size_t stIndex = 0; stIndex < tPaths.GetCount(); ++stIndex)
	{
		TBasePathDataPtr spPathData = boost::make_shared<TBasePathData>(++m_stLastObjectID, tPaths.GetAt(stIndex));
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

END_CHCORE_NAMESPACE
