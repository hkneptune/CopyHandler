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
#include "TSQLiteSerializerRowWriter.h"
#include "ErrorCodes.h"
#include "TCoreException.h"
#include <boost/format.hpp>
#include "TSQLiteStatement.h"
#include "TSQLiteSerializerRowReader.h"

BEGIN_CHCORE_NAMESPACE

TSQLiteSerializerContainer::TSQLiteSerializerContainer(const TString& strName, const sqlite::TSQLiteDatabasePtr& spDB) :
	m_spColumns(new TSQLiteColumnsDefinition),
	m_strName(strName),
	m_spDB(spDB)
{
}

TSQLiteSerializerContainer::TSQLiteSerializerContainer(const TString& strName, size_t stParentID, const sqlite::TSQLiteDatabasePtr& spDB) :
	m_stParentID(stParentID),
	m_spColumns(new TSQLiteColumnsDefinition),
	m_strName(strName),
	m_spDB(spDB)
{
}

TSQLiteSerializerContainer::~TSQLiteSerializerContainer()
{
}

chcore::ISerializerRowWriterPtr TSQLiteSerializerContainer::AddRow(size_t stRowID)
{
	RowMap::iterator iterInsert = m_mapRows.insert(
			std::make_pair(stRowID, TSQLiteSerializerRowWriterPtr(new TSQLiteSerializerRowWriter(stRowID, m_spColumns, true)))
		).first;
	return (*iterInsert).second;
}

ISerializerRowWriterPtr TSQLiteSerializerContainer::GetRow(size_t stRowID)
{
	RowMap::iterator iterFnd = m_mapRows.find(stRowID);
	if(iterFnd == m_mapRows.end())
		iterFnd = m_mapRows.insert(std::make_pair(stRowID, TSQLiteSerializerRowWriterPtr(new TSQLiteSerializerRowWriter(stRowID, m_spColumns, false)))).first;

	return (*iterFnd).second;
}

void TSQLiteSerializerContainer::DeleteRow(size_t stRowID)
{
	RowMap::iterator iterFnd = m_mapRows.find(stRowID);
	if(iterFnd != m_mapRows.end())
	{
		m_mapRows.erase(iterFnd);
		m_setDeleteItems.insert(stRowID);
	}
	else
		THROW_CORE_EXCEPTION(eErr_SerializeStoreError);
}

ISerializerRowReaderPtr TSQLiteSerializerContainer::GetRowReader()
{
	TSQLiteSerializerRowReaderPtr spRowReader(new TSQLiteSerializerRowReader(m_spDB, m_spColumns, m_strName));
	return spRowReader;
}

chcore::IColumnsDefinitionPtr TSQLiteSerializerContainer::GetColumnsDefinition() const
{
	return m_spColumns;
}

END_CHCORE_NAMESPACE
