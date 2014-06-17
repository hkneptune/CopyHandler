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
#include "TSQLiteSerializerRowData.h"
#include "ErrorCodes.h"
#include "TCoreException.h"
#include <boost/format.hpp>
#include "TSQLiteStatement.h"
#include "TSQLiteSerializerRowReader.h"
#include "TRemovedObjects.h"
#include "SerializerTrace.h"

BEGIN_CHCORE_NAMESPACE

using namespace sqlite;

TSQLiteSerializerContainer::TSQLiteSerializerContainer(const TString& strName, const sqlite::TSQLiteDatabasePtr& spDB) :
	m_strName(strName),
	m_spDB(spDB)
{
}

TSQLiteSerializerContainer::TSQLiteSerializerContainer(const TString& strName, size_t stParentID, const sqlite::TSQLiteDatabasePtr& spDB) :
	m_stParentID(stParentID),
	m_strName(strName),
	m_spDB(spDB)
{
}

TSQLiteSerializerContainer::~TSQLiteSerializerContainer()
{
}

ISerializerRowDataPtr TSQLiteSerializerContainer::GetRow(size_t stRowID, bool bMarkAsAdded)
{
	RowMap::iterator iterFnd = m_mapRows.find(stRowID);
	if(iterFnd == m_mapRows.end())
		iterFnd = m_mapRows.insert(std::make_pair(stRowID, TSQLiteSerializerRowDataPtr(new TSQLiteSerializerRowData(stRowID, m_tColumns, bMarkAsAdded)))).first;
	else if(bMarkAsAdded)
		iterFnd->second->MarkAsAdded();

	return (*iterFnd).second;
}

void TSQLiteSerializerContainer::DeleteRow(size_t stRowID)
{
	RowMap::iterator iterFnd = m_mapRows.find(stRowID);
	if(iterFnd != m_mapRows.end())
		m_mapRows.erase(iterFnd);

	m_setDeleteItems.insert(stRowID);
}

void TSQLiteSerializerContainer::DeleteRows(const TRemovedObjects& setObjects)
{
	size_t stCount = setObjects.GetCount();
	while(stCount-- != 0)
	{
		DeleteRow(setObjects.GetAt(stCount));
	}
}

ISerializerRowReaderPtr TSQLiteSerializerContainer::GetRowReader()
{
	TSQLiteSerializerRowReaderPtr spRowReader(new TSQLiteSerializerRowReader(m_spDB, m_tColumns, m_strName));
	return spRowReader;
}

IColumnsDefinition& TSQLiteSerializerContainer::GetColumnsDefinition()
{
	return m_tColumns;
}

void TSQLiteSerializerContainer::Flush()
{
	FlushDeletions();

	// group rows that can be executed with one preparation
	std::map<TRowID, std::vector<TSQLiteSerializerRowDataPtr>> mapGroups;
	std::map<TRowID, std::vector<TSQLiteSerializerRowDataPtr>>::iterator iterMapGroups;

	for(RowMap::iterator iterRows = m_mapRows.begin(); iterRows != m_mapRows.end(); ++iterRows)
	{
		TRowID rowID = iterRows->second->GetChangeIdentification();
		iterMapGroups = mapGroups.find(rowID);
		if(iterMapGroups == mapGroups.end())
			iterMapGroups = mapGroups.insert(std::make_pair(rowID, std::vector<TSQLiteSerializerRowDataPtr>())).first;

		iterMapGroups->second.push_back(iterRows->second);
	}

	TSQLiteStatement tStatement(m_spDB);

	for(iterMapGroups = mapGroups.begin(); iterMapGroups != mapGroups.end(); ++iterMapGroups)
	{
		if(iterMapGroups->first.HasAny())
		{
			std::vector<TSQLiteSerializerRowDataPtr>& rGroupRows = iterMapGroups->second;

			// query is generated from the first item in a group
			TString strQuery = rGroupRows.front()->GetQuery(m_strName);

			DBTRACE2(_T("Preparing query for %lu records: %s\n"), (unsigned long)iterMapGroups->second.size(), (PCTSTR)strQuery);

			tStatement.Prepare(strQuery);

			for(std::vector<TSQLiteSerializerRowDataPtr>::iterator iterRow = iterMapGroups->second.begin(); iterRow != iterMapGroups->second.end(); ++iterRow)
			{
				(*iterRow)->BindParamsAndExec(tStatement);
				tStatement.ClearBindings();
			}

			tStatement.Close();
		}
	}
}

void TSQLiteSerializerContainer::FlushDeletions()
{
	// delete from m_strName WHERE id IN (???)
	TSQLiteStatement tStatement(m_spDB);

	const size_t stMaxToRemoveAtOnce = 10;

	// delete items in chunks
	std::set<size_t>::const_iterator iterToDelete = m_setDeleteItems.begin();
	while(iterToDelete != m_setDeleteItems.end())
	{
		TString strItemsToRemove;
		size_t stToRemove = stMaxToRemoveAtOnce;
		while(iterToDelete != m_setDeleteItems.end() && (--stToRemove) != 0)
		{
			strItemsToRemove += boost::str(boost::wformat(L"%1%,") % *iterToDelete).c_str();
			++iterToDelete;
		}
		strItemsToRemove.TrimRightSelf(_T(","));

		TString strQuery = boost::str(boost::wformat(L"DELETE FROM %1% WHERE id IN (%2%)") % m_strName % strItemsToRemove).c_str();
		tStatement.Prepare(strQuery);

		DBTRACE1_D(_T("Executing query: %s\n"), (PCTSTR)strQuery);
		tStatement.Step();
	}
}

END_CHCORE_NAMESPACE
