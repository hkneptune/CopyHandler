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
#include "stdafx.h"
#include "TFileFiltersArray.h"
#include "TFileInfo.h"
#include "TConfig.h"
#include "TBinarySerializer.h"
#include "SerializationHelpers.h"

BEGIN_CHCORE_NAMESPACE

TFileFiltersArray& TFileFiltersArray::operator=(const TFileFiltersArray& rSrc)
{
	if(this != &rSrc)
	{
		m_vFilters = rSrc.m_vFilters;
	}

	return *this;
}

bool TFileFiltersArray::Match(const TFileInfoPtr& spInfo) const
{
	if(m_vFilters.empty())
		return true;

	// if only one of the filters matches - return true
	for(std::vector<TFileFilter>::const_iterator iterFilter = m_vFilters.begin(); iterFilter != m_vFilters.end(); iterFilter++)
	{
		if((*iterFilter).Match(spInfo))
			return true;
	}

	return false;
}

void TFileFiltersArray::StoreInConfig(TConfig& rConfig, PCTSTR pszNodeName) const
{
	rConfig.DeleteNode(pszNodeName);
	BOOST_FOREACH(const TFileFilter& rFilter, m_vFilters)
	{
		TConfig cfgNode;
		rFilter.StoreInConfig(cfgNode);
		rConfig.AddSubConfig(TString(pszNodeName) + _T(".FilterDefinition"), cfgNode);
	}
}

bool TFileFiltersArray::ReadFromConfig(const TConfig& rConfig, PCTSTR pszNodeName)
{
	m_vFilters.clear();

	TConfigArray vConfigs;
	if(!rConfig.ExtractMultiSubConfigs(pszNodeName, vConfigs))
		return false;

	for(size_t stIndex = 0; stIndex < vConfigs.GetCount(); ++stIndex)
	{
		const TConfig& rCfg = vConfigs.GetAt(stIndex);
		TFileFilter tFilter;
		tFilter.ReadFromConfig(rCfg);

		m_vFilters.push_back(tFilter);
	}
	return true;
}

void TFileFiltersArray::Serialize(TReadBinarySerializer& rSerializer)
{
	using Serializers::Serialize;
	Serialize(rSerializer, m_vFilters);
}

void TFileFiltersArray::Serialize(TWriteBinarySerializer& rSerializer) const
{
	using Serializers::Serialize;
	Serialize(rSerializer, m_vFilters);
}

bool TFileFiltersArray::IsEmpty() const
{
	return m_vFilters.empty();
}

void TFileFiltersArray::Add(const TFileFilter& rFilter)
{
	m_vFilters.push_back(rFilter);
}

bool TFileFiltersArray::SetAt(size_t stIndex, const TFileFilter& rNewFilter)
{
	BOOST_ASSERT(stIndex < m_vFilters.size());
	if(stIndex < m_vFilters.size())
	{
		TFileFilter& rFilter = m_vFilters.at(stIndex);
		rFilter = rNewFilter;
		return true;
	}
	else
		return false;
}

const TFileFilter* TFileFiltersArray::GetAt(size_t stIndex) const
{
	BOOST_ASSERT(stIndex < m_vFilters.size());
	if(stIndex < m_vFilters.size())
		return &m_vFilters.at(stIndex);
	else
		return NULL;
}

bool TFileFiltersArray::RemoveAt(size_t stIndex)
{
	BOOST_ASSERT(stIndex < m_vFilters.size());
	if(stIndex < m_vFilters.size())
	{
		m_vFilters.erase(m_vFilters.begin() + stIndex);
		return true;
	}
	else
		return false;
}

size_t TFileFiltersArray::GetSize() const
{
	return m_vFilters.size();
}

void TFileFiltersArray::Clear()
{
	m_vFilters.clear();
}

END_CHCORE_NAMESPACE
