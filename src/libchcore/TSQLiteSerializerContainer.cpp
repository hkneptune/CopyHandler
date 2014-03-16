// ============================================================================
//  Copyright (C) 2001-2013 by Jozef Starosczyk
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
#include "TSQLiteSerializerContainer.h"
#include "TSQLiteSerializerRow.h"

BEGIN_CHCORE_NAMESPACE

TSQLiteSerializerContainer::TSQLiteSerializerContainer() :
	m_spColumns(new TSQLiteColumnDefinition)
{
}

TSQLiteSerializerContainer::TSQLiteSerializerContainer(size_t stParentID) :
	m_stParentID(stParentID),
	m_spColumns(new TSQLiteColumnDefinition)
{
}

TSQLiteSerializerContainer::~TSQLiteSerializerContainer()
{
}

chcore::ISerializerRowPtr TSQLiteSerializerContainer::GetNewRow()
{
	size_t stNewIndex = 0;

	if(m_mapRows.rbegin() != m_mapRows.rend())
	{
		stNewIndex = m_mapRows.rbegin()->first + 1;
	}

	std::map<size_t, ISerializerRowPtr>::iterator iterInsert = m_mapRows.insert(std::make_pair(stNewIndex, TSQLiteSerializerRowPtr(new TSQLiteSerializerRow(stNewIndex, m_spColumns)))).first;
	return (*iterInsert).second;
}

ISerializerRowPtr TSQLiteSerializerContainer::GetRow(size_t stRowID)
{
	std::map<size_t, ISerializerRowPtr>::iterator iterFnd = m_mapRows.find(stRowID);
	if(iterFnd == m_mapRows.end())
		iterFnd = m_mapRows.insert(std::make_pair(stRowID, ISerializerRowPtr(new TSQLiteSerializerRow(stRowID, m_spColumns)))).first;

	return (*iterFnd).second;
}

END_CHCORE_NAMESPACE
