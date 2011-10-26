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

BEGIN_CHCORE_NAMESPACE

//////////////////////////////////////////////////////////////////////////////
// TBasePathData

TBasePathData::TBasePathData() :
	m_bMove(true)
{
}

TBasePathData::TBasePathData(const TBasePathData& rEntry) :
	m_bMove(rEntry.m_bMove),
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
		Serializers::Serialize(rSerializer, m_bMove);
	else
		Serializers::Serialize(rSerializer, m_pathDst);
}

void TBasePathData::Serialize(TWriteBinarySerializer& rSerializer, bool bData)
{
	if(bData)
		Serializers::Serialize(rSerializer, m_bMove);
	else
		Serializers::Serialize(rSerializer, m_pathDst);
}

//////////////////////////////////////////////////////////////////////////////
// TBasePathDataContainer

TBasePathDataContainer::TBasePathDataContainer(const TPathContainer& tBasePaths) :
	m_tBasePaths(tBasePaths)
{
}

TBasePathDataContainer::~TBasePathDataContainer()
{
	Clear();
}

TBasePathDataPtr TBasePathDataContainer::GetAt(size_t stPos) const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	
	if(stPos >= m_vEntries.size())
		THROW_CORE_EXCEPTION(eErr_BoundsExceeded);

	return m_vEntries.at(stPos);
}

void TBasePathDataContainer::SetAt(size_t stIndex, const TBasePathDataPtr& spEntry)
{
	if(!spEntry)
		THROW_CORE_EXCEPTION(eErr_InvalidArgument);

	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	
	if(stIndex >= m_vEntries.size())
		THROW_CORE_EXCEPTION(eErr_BoundsExceeded);
	
	m_vEntries[stIndex] = spEntry;
}

void TBasePathDataContainer::Add(const TBasePathDataPtr& spEntry)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_vEntries.push_back(spEntry);
}

void TBasePathDataContainer::RemoveAt(size_t nIndex, size_t nCount)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_vEntries.erase(m_vEntries.begin() + nIndex, m_vEntries.begin() + nIndex + nCount);
}

void TBasePathDataContainer::Clear()
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_vEntries.clear();
}

void TBasePathDataContainer::SetCount(size_t stCount)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	if(stCount > m_vEntries.size())
	{
		size_t stCountToAdd = stCount - m_vEntries.size();
		while(stCountToAdd--)
		{
			TBasePathDataPtr spData(new TBasePathData);
			m_vEntries.push_back(spData);
		}
	}
}

size_t TBasePathDataContainer::GetCount() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_vEntries.size();
}

void TBasePathDataContainer::Serialize(TReadBinarySerializer& rSerializer, bool bData)
{
	using Serializers::Serialize;

	size_t stCount;
	Serialize(rSerializer, stCount);

	boost::unique_lock<boost::shared_mutex> lock(m_lock);

	if(!bData && m_vEntries.size() != stCount)
		THROW_CORE_EXCEPTION(eErr_InternalProblem);

	if(bData)
	{
		m_vEntries.clear();
		m_vEntries.reserve(stCount);
	}

	TBasePathDataPtr spEntry;
	for(size_t stIndex = 0; stIndex < stCount; ++stIndex)
	{
		if(bData)
			spEntry.reset(new TBasePathData);
		else
			spEntry = m_vEntries.at(stIndex);
		spEntry->Serialize(rSerializer, bData);

		if(bData)
			m_vEntries.push_back(spEntry);
	}
}

void TBasePathDataContainer::Serialize(TWriteBinarySerializer& rSerializer, bool bData)
{
	using Serializers::Serialize;

	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	// write data
	size_t stCount = m_vEntries.size();
	Serialize(rSerializer, stCount);

	BOOST_FOREACH(const TBasePathDataPtr& spEntry, m_vEntries)
	{
		spEntry->Serialize(rSerializer, bData);
	}
}

END_CHCORE_NAMESPACE
