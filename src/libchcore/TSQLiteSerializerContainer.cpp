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
#include <boost/pool/pool.hpp>
#include "TSerializerException.h"

namespace chcore
{
	using namespace sqlite;

	TSQLiteSerializerContainer::TSQLiteSerializerContainer(const TString& strName, const sqlite::TSQLiteDatabasePtr& spDB, TPlainStringPool& poolStrings) :
		m_strName(strName),
		m_spDB(spDB),
		m_pPoolRows(NULL),
		m_poolStrings(poolStrings)
	{
	}

	TSQLiteSerializerContainer::~TSQLiteSerializerContainer()
	{
		// get rid of all rows first
		m_mapRows.clear();

		// now get rid of memory pool
		delete m_pPoolRows;
	}

	ISerializerRowData& TSQLiteSerializerContainer::GetRow(object_id_t oidRowID, bool bMarkAsAdded)
	{
		RowMap::iterator iterFnd = m_mapRows.find(oidRowID);
		if (iterFnd == m_mapRows.end())
		{
			void* pMemoryBlock = GetPool().malloc();
			if (!pMemoryBlock)
				throw TSerializerException(eErr_InternalProblem, _T("Cannot allocate memory"), LOCATION);

			iterFnd = m_mapRows.insert(std::make_pair(oidRowID, TSQLiteSerializerRowData(oidRowID, m_tColumns, bMarkAsAdded, (unsigned long long*)pMemoryBlock, GetPool().get_requested_size(), m_poolStrings))).first;
		}
		else if (bMarkAsAdded)
			iterFnd->second.MarkAsAdded();

		return (*iterFnd).second;
	}

	void TSQLiteSerializerContainer::DeleteRow(object_id_t oidRowID)
	{
		RowMap::iterator iterFnd = m_mapRows.find(oidRowID);
		if (iterFnd != m_mapRows.end())
			m_mapRows.erase(iterFnd);

		m_setDeleteItems.insert(oidRowID);
	}

	void TSQLiteSerializerContainer::DeleteRows(const TRemovedObjects& setObjects)
	{
		size_t stCount = setObjects.GetCount();
		while (stCount-- != 0)
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
		std::map<unsigned long long, std::vector<TSQLiteSerializerRowData*>> mapGroups;
		std::map<unsigned long long, std::vector<TSQLiteSerializerRowData*>>::iterator iterMapGroups;

		for (RowMap::iterator iterRows = m_mapRows.begin(); iterRows != m_mapRows.end(); ++iterRows)
		{
			unsigned long long rowID = iterRows->second.GetChangeIdentification();
			iterMapGroups = mapGroups.find(rowID);
			if (iterMapGroups == mapGroups.end())
				iterMapGroups = mapGroups.insert(std::make_pair(rowID, std::vector<TSQLiteSerializerRowData*>())).first;

			iterMapGroups->second.push_back(&iterRows->second);
		}

		TSQLiteStatement tStatement(m_spDB);

		for (iterMapGroups = mapGroups.begin(); iterMapGroups != mapGroups.end(); ++iterMapGroups)
		{
			if (iterMapGroups->first != 0)
			{
				std::vector<TSQLiteSerializerRowData*>& rGroupRows = iterMapGroups->second;

				// query is generated from the first item in a group
				TString strQuery = rGroupRows.front()->GetQuery(m_strName);
				if (!strQuery.IsEmpty())
				{
					DBTRACE2(_T("Preparing query for %lu records: %s\n"), (unsigned long)iterMapGroups->second.size(), strQuery.c_str());

					tStatement.Prepare(strQuery.c_str());

					for (std::vector<TSQLiteSerializerRowData*>::iterator iterRow = iterMapGroups->second.begin(); iterRow != iterMapGroups->second.end(); ++iterRow)
					{
						(*iterRow)->BindParamsAndExec(tStatement);
						tStatement.ClearBindings();
					}

					tStatement.Close();
				}
			}
		}
	}

	void TSQLiteSerializerContainer::FlushDeletions()
	{
		// delete from m_strName WHERE id IN (???)
		TSQLiteStatement tStatement(m_spDB);

		const size_t stMaxToRemoveAtOnce = 10;

		// delete items in chunks
		std::set<object_id_t>::const_iterator iterToDelete = m_setDeleteItems.begin();
		while (iterToDelete != m_setDeleteItems.end())
		{
			TString strItemsToRemove;
			size_t stToRemove = stMaxToRemoveAtOnce;
			while (iterToDelete != m_setDeleteItems.end() && (--stToRemove) != 0)
			{
				strItemsToRemove += boost::str(boost::wformat(L"%1%,") % *iterToDelete).c_str();
				++iterToDelete;
			}
			strItemsToRemove.TrimRightSelf(_T(","));

			TString strQuery = boost::str(boost::wformat(L"DELETE FROM %1% WHERE id IN (%2%)") % m_strName % strItemsToRemove).c_str();
			tStatement.Prepare(strQuery.c_str());

			DBTRACE1_D(_T("Executing query: %s\n"), strQuery.c_str());
			tStatement.Step();
		}
	}

	boost::pool<>& TSQLiteSerializerContainer::GetPool()
	{
		if (!m_pPoolRows)
			m_pPoolRows = new boost::pool<>(CalculateRowMemorySize());
		else
		{
			if (m_pPoolRows->get_requested_size() != CalculateRowMemorySize())
				throw TSerializerException(eErr_InternalProblem, _T("Column count changed after first use"), LOCATION);
		}

		return *m_pPoolRows;
	}

	size_t TSQLiteSerializerContainer::CalculateRowMemorySize() const
	{
		// assume 64bit column mask (8 bytes)
		const size_t stMaskSize = 8;

		// and additionally 64bit for each column (either for storing numbers directly or for allocating string/double)
		const size_t stFieldSize = 8;

		return stMaskSize + m_tColumns.GetCount() * stFieldSize;
	}
}
