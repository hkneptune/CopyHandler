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
#include "TSQLiteSerializerRowData.h"
#include "TSQLiteStatement.h"
#include <boost/format.hpp>
#include <atltrace.h>

BEGIN_CHCORE_NAMESPACE

namespace
{
	struct SQLiteBindValueVisitor : public boost::static_visitor<>
	{
	private:
		SQLiteBindValueVisitor& operator=(const SQLiteBindValueVisitor&);

	public:
		SQLiteBindValueVisitor(sqlite::TSQLiteStatement& rStatement, int& rColumn) : m_rStatement(rStatement), m_rColumn(rColumn) {}

		void operator()(bool value) const
		{
			m_rStatement.BindValue(m_rColumn++, value);
		}

		void operator()(short value) const
		{
			m_rStatement.BindValue(m_rColumn++, value);
		}

		void operator()(unsigned short value) const
		{
			m_rStatement.BindValue(m_rColumn++, value);
		}

		void operator()(int value) const
		{
			m_rStatement.BindValue(m_rColumn++, value);
		}

		void operator()(unsigned int value) const
		{
			m_rStatement.BindValue(m_rColumn++, value);
		}

		void operator()(long value) const
		{
			m_rStatement.BindValue(m_rColumn++, value);
		}

		void operator()(unsigned long value) const
		{
			m_rStatement.BindValue(m_rColumn++, value);
		}

		void operator()(long long value) const
		{
			m_rStatement.BindValue(m_rColumn++, value);
		}

		void operator()(unsigned long long value) const
		{
			m_rStatement.BindValue(m_rColumn++, value);
		}

		void operator()(double value) const
		{
			m_rStatement.BindValue(m_rColumn++, value);
		}

		void operator()(const TString& value) const
		{
			m_rStatement.BindValue(m_rColumn++, value);
		}

		void operator()(const TSmartPath& value) const
		{
			m_rStatement.BindValue(m_rColumn++, value);
		}

		int& m_rColumn;
		sqlite::TSQLiteStatement& m_rStatement;
	};
}

TSQLiteSerializerRowData::TSQLiteSerializerRowData(size_t stRowID, const TSQLiteColumnDefinitionPtr& spColumnDefinition, bool bAdded) :
	m_stRowID(stRowID),
	m_spColumns(spColumnDefinition),
	m_bAdded(bAdded)
{
}

TSQLiteSerializerRowData::~TSQLiteSerializerRowData()
{
}

ISerializerRowData& TSQLiteSerializerRowData::operator%(const TRowData& rData)
{
	size_t stColumn = m_spColumns->GetColumnIndex(rData.m_strColName);
	std::map<size_t, TRowData::InternalVariant>::iterator iterFnd = m_mapValues.find(stColumn);
	if(iterFnd == m_mapValues.end())
		m_mapValues.insert(std::make_pair(stColumn, rData.m_varValue));
	else
		(*iterFnd).second = rData.m_varValue;

	return *this;
}

ISerializerRowData& TSQLiteSerializerRowData::SetValue(const TRowData& rData)
{
	size_t stColumn = m_spColumns->GetColumnIndex(rData.m_strColName);
	std::map<size_t, TRowData::InternalVariant>::iterator iterFnd = m_mapValues.find(stColumn);
	if(iterFnd == m_mapValues.end())
		m_mapValues.insert(std::make_pair(stColumn, rData.m_varValue));
	else
		(*iterFnd).second = rData.m_varValue;

	return *this;
}

void TSQLiteSerializerRowData::Flush(const sqlite::TSQLiteDatabasePtr& spDatabase, const TString& strContainerName)
{
	using namespace sqlite;

	TSQLiteStatement tStatement(spDatabase);

	if(m_bAdded)
	{
		// prepare insert query
		TString strQuery = boost::str(boost::wformat(L"INSERT INTO %1%(id,") % strContainerName).c_str();
		TString strParams;

		for(MapVariants::iterator iterVariant = m_mapValues.begin(); iterVariant != m_mapValues.end(); ++iterVariant)
		{
			strQuery += boost::str(boost::wformat(_T("%1%,")) % m_spColumns->GetColumnName(iterVariant->first)).c_str();
			strParams += _T("?,");
		}

		strQuery.TrimRightSelf(_T(","));
		strQuery += _T(") VALUES(?,");

		strParams.TrimRightSelf(_T(","));
		strQuery += strParams;
		strQuery += _T(")");

		// exec query
		int iColumn = 1;
		tStatement.Prepare(strQuery);
		tStatement.BindValue(iColumn++, m_stRowID);
		for(MapVariants::iterator iterVariant = m_mapValues.begin(); iterVariant != m_mapValues.end(); ++iterVariant)
		{
			boost::apply_visitor(SQLiteBindValueVisitor(tStatement, iColumn), iterVariant->second);
		}

		ATLTRACE(_T("Executing query: %s\n"), (PCTSTR)strQuery);
		tStatement.Step();
	}
	else
	{
		// prepare update query
		TString strQuery = boost::str(boost::wformat(L"UPDATE %1% SET ") % strContainerName).c_str();

		for(MapVariants::iterator iterVariant = m_mapValues.begin(); iterVariant != m_mapValues.end(); ++iterVariant)
		{
			strQuery += boost::str(boost::wformat(_T("%1%=?,")) % m_spColumns->GetColumnName(iterVariant->first)).c_str();
		}

		strQuery.TrimRightSelf(_T(","));
		strQuery += _T(" WHERE id=?");

		int iColumn = 1;
		tStatement.Prepare(strQuery);
		for(MapVariants::iterator iterVariant = m_mapValues.begin(); iterVariant != m_mapValues.end(); ++iterVariant)
		{
			boost::apply_visitor(SQLiteBindValueVisitor(tStatement, iColumn), iterVariant->second);
		}
		tStatement.BindValue(iColumn++, m_stRowID);

		ATLTRACE(_T("Executing query: %s\n"), (PCTSTR)strQuery);
		tStatement.Step();
	}
}

END_CHCORE_NAMESPACE
