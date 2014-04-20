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
#include "TRowData.h"
#include "ISerializerRowData.h"

BEGIN_CHCORE_NAMESPACE

//////////////////////////////////////////////////////////////////////////////
// TBasePathData

TBasePathData::TBasePathData() :
	m_bSkipFurtherProcessing(m_setModifications, false),
	m_pathDst(m_setModifications)
{
	m_setModifications[eMod_Added] = true;
}

TBasePathData::TBasePathData(const TBasePathData& rEntry) :
	m_pathDst(rEntry.m_pathDst),
	m_bSkipFurtherProcessing(rEntry.m_bSkipFurtherProcessing),
	m_setModifications(rEntry.m_setModifications)
{
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

void TBasePathData::Store(const ISerializerContainerPtr& spContainer, size_t stObjectID) const
{
	if(!spContainer)
		THROW_CORE_EXCEPTION(eErr_InvalidPointer);

	ISerializerRowDataPtr spRow;

	bool bAdded = m_setModifications.at(eMod_Added);
	if(bAdded)
		spRow = spContainer->AddRow(stObjectID);
	else if(m_setModifications.any())
		spRow = spContainer->GetRow(stObjectID);

	if(bAdded || m_setModifications.at(eMod_SkipProcessing))
		*spRow % TRowData(_T("skip_processing"), m_bSkipFurtherProcessing);
	if(bAdded || m_setModifications.at(eMod_DstPath))
		*spRow % TRowData(_T("dst_path"), m_pathDst);

	m_setModifications.reset();
}

void TBasePathData::InitLoader(const IColumnsDefinitionPtr& spColumnDefs)
{
	if(!spColumnDefs)
		THROW_CORE_EXCEPTION(eErr_InvalidPointer);

	*spColumnDefs % _T("id") % _T("skip_processing") % _T("dst_path");
}

void TBasePathData::Load(const ISerializerRowReaderPtr& spRowReader, size_t& stObjectID)
{
	spRowReader->GetValue(_T("id"), stObjectID);
	spRowReader->GetValue(_T("skip_processing"), m_bSkipFurtherProcessing.Modify());
	spRowReader->GetValue(_T("dst_path"), m_pathDst.Modify());
	m_setModifications.reset();
}

//////////////////////////////////////////////////////////////////////////////
// TBasePathDataContainer

TBasePathDataContainer::TBasePathDataContainer()
{
}

TBasePathDataContainer::~TBasePathDataContainer()
{
	// clear works with critical section to avoid destruction while item in use
	Clear();
}

bool TBasePathDataContainer::Exists(size_t stObjectID) const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);

	bool bResult = m_mapEntries.find(stObjectID) != m_mapEntries.end();
	return bResult;
}

TBasePathDataPtr TBasePathDataContainer::GetExisting(size_t stObjectID) const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	
	MapEntries::const_iterator iter = m_mapEntries.find(stObjectID);
	if(iter == m_mapEntries.end())
		THROW_CORE_EXCEPTION(eErr_BoundsExceeded);

	return iter->second;
}

chcore::TBasePathDataPtr TBasePathDataContainer::Get(size_t stObjectID)
{
	boost::upgrade_lock<boost::shared_mutex> lock(m_lock);

	MapEntries::iterator iter = m_mapEntries.find(stObjectID);
	if(iter == m_mapEntries.end())
	{
		boost::upgrade_to_unique_lock<boost::shared_mutex> upgraded_lock(lock);
		iter = m_mapEntries.insert(std::make_pair(stObjectID, TBasePathDataPtr(new TBasePathData))).first;
	}

	return iter->second;
}

void TBasePathDataContainer::Remove(size_t stObjectID)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_mapEntries.erase(stObjectID);
	m_setRemovedObjects.Add(stObjectID);
}

void TBasePathDataContainer::Clear()
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	
	BOOST_FOREACH(const MapEntries::value_type& rItem, m_mapEntries)
	{
		m_setRemovedObjects.Add(rItem.first);
	}

	m_mapEntries.clear();
}

bool TBasePathDataContainer::GetSkipFurtherProcessing(size_t stObjectID) const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);

	MapEntries::const_iterator iter = m_mapEntries.find(stObjectID);
	if(iter == m_mapEntries.end())
		return false;

	return iter->second->GetSkipFurtherProcessing();
}

chcore::TSmartPath TBasePathDataContainer::GetDestinationPath(size_t stObjectID) const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);

	MapEntries::const_iterator iter = m_mapEntries.find(stObjectID);
	if(iter == m_mapEntries.end())
		return TSmartPath();

	return iter->second->GetDestinationPath();
}

bool TBasePathDataContainer::IsDestinationPathSet(size_t stObjectID) const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);

	MapEntries::const_iterator iter = m_mapEntries.find(stObjectID);
	if(iter == m_mapEntries.end())
		return false;

	return iter->second->IsDestinationPathSet();
}

void TBasePathDataContainer::Store(const ISerializerContainerPtr& spContainer) const
{
	if(!spContainer)
		THROW_CORE_EXCEPTION(eErr_InvalidPointer);

	boost::shared_lock<boost::shared_mutex> lock(m_lock);

	spContainer->DeleteRows(m_setRemovedObjects);
	m_setRemovedObjects.Clear();

	BOOST_FOREACH(const MapEntries::value_type& rPair, m_mapEntries)
	{
		rPair.second->Store(spContainer, rPair.first);
	}
}

void TBasePathDataContainer::Load(const ISerializerContainerPtr& spContainer)
{
	if(!spContainer)
		THROW_CORE_EXCEPTION(eErr_InvalidPointer);

	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_setRemovedObjects.Clear();
	m_mapEntries.clear();

	ISerializerRowReaderPtr spRowReader = spContainer->GetRowReader();
	IColumnsDefinitionPtr spColumns = spRowReader->GetColumnsDefinitions();
	if(spColumns->IsEmpty())
		TBasePathData::InitLoader(spRowReader->GetColumnsDefinitions());

	while(spRowReader->Next())
	{
		TBasePathDataPtr spPathData(new TBasePathData);
		size_t stObjectID = 0;

		spPathData->Load(spRowReader, stObjectID);

		m_mapEntries.insert(std::make_pair(stObjectID, spPathData));
	}
}

END_CHCORE_NAMESPACE
