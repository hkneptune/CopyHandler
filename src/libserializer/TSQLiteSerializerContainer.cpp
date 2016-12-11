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
#include "../libchcore/ErrorCodes.h"
#include <boost/format.hpp>
#include "TSQLiteStatement.h"
#include "TSQLiteSerializerRowReader.h"
#include <boost/pool/pool.hpp>
#include "TSerializerException.h"
#include "TRemovedObjects.h"

using namespace string;
using namespace chcore;

namespace serializer
{
	using namespace sqlite;

	TSQLiteSerializerContainer::TSQLiteSerializerContainer(const TString& strName, const sqlite::TSQLiteDatabasePtr& spDB, TPlainStringPool& poolStrings, const logger::TLogFileDataPtr& spLogFileData) :
		m_pPoolRows(nullptr),
		m_strName(strName),
		m_spDB(spDB),
		m_poolStrings(poolStrings),
		m_spLog(logger::MakeLogger(spLogFileData, L"Serializer-Container"))
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

			iterFnd = m_mapRows.insert(std::make_pair(oidRowID, std::make_unique<TSQLiteSerializerRowData>(oidRowID, m_tColumns, bMarkAsAdded, (unsigned long long*)pMemoryBlock, GetPool().get_requested_size(), m_poolStrings, m_spLog->GetLogFileData()))).first;
		}
		else if (bMarkAsAdded)
			iterFnd->second->MarkAsAdded();

		return *(*iterFnd).second.get();
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

		auto iterRows = m_mapRows.begin();
		size_t stInputIndex = 0;
		while(stInputIndex < stCount && iterRows != m_mapRows.end())
		{
			object_id_t idInput = setObjects.GetAt(stInputIndex);
			object_id_t idRows = iterRows->first;

			if(idInput < idRows)
				++stInputIndex;
			else if(idInput > idRows)
				++iterRows;
			else
			{
				// equals
				iterRows = m_mapRows.erase(iterRows);
				++stInputIndex;
			}
		}

		while(stCount > 0)
		{
			m_setDeleteItems.insert(setObjects.GetAt(--stCount));
		}
	}

	ISerializerRowReaderPtr TSQLiteSerializerContainer::GetRowReader()
	{
		TSQLiteSerializerRowReaderPtr spRowReader(new TSQLiteSerializerRowReader(m_spDB, m_tColumns, m_strName, m_spLog->GetLogFileData()));
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
			unsigned long long rowID = iterRows->second->GetChangeIdentification();
			iterMapGroups = mapGroups.find(rowID);
			if (iterMapGroups == mapGroups.end())
				iterMapGroups = mapGroups.insert(std::make_pair(rowID, std::vector<TSQLiteSerializerRowData*>())).first;

			iterMapGroups->second.push_back(iterRows->second.get());
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
					LOG_DEBUG(m_spLog) << L"Preparing query for " << (unsigned long)iterMapGroups->second.size() << L" records: " << strQuery;

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
		size_t stCount = m_setDeleteItems.size();
		if(stCount == 0)
			return;

		TSQLiteStatement tStatement(m_spDB);

		TString strQuery = boost::str(boost::wformat(L"DELETE FROM %1% WHERE id=?1") % m_strName).c_str();
		tStatement.Prepare(strQuery.c_str());

		for (object_id_t idObj : m_setDeleteItems)
		{
			tStatement.ClearBindings();
			tStatement.BindValue(1, idObj);

			LOG_DEBUG(m_spLog) << L"Executing query: " << strQuery;
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
