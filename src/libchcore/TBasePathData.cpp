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

BEGIN_CHCORE_NAMESPACE

//////////////////////////////////////////////////////////////////////////////
// TBasePathData

TBasePathData::TBasePathData() :
	m_bSkipFurtherProcessing(false)
{
}

TBasePathData::TBasePathData(const TBasePathData& rEntry) :
	m_pathDst(rEntry.m_pathDst)
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

void TBasePathData::Serialize(TReadBinarySerializer& rSerializer, bool bData)
{
	if(bData)
		Serializers::Serialize(rSerializer, m_bSkipFurtherProcessing);
	else
		Serializers::Serialize(rSerializer, m_pathDst);
}

void TBasePathData::Serialize(TWriteBinarySerializer& rSerializer, bool bData)
{
	if(bData)
		Serializers::Serialize(rSerializer, m_bSkipFurtherProcessing);
	else
		Serializers::Serialize(rSerializer, m_pathDst);
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
}

void TBasePathDataContainer::Clear()
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
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

END_CHCORE_NAMESPACE
