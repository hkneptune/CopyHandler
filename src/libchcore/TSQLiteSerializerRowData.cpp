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
#include "TSerializerException.h"
#include "ErrorCodes.h"
#include "SerializerTrace.h"

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
			DBTRACE1_D(_T("- param(bool): %ld\n"), value ? 1l : 0l);
			m_rStatement.BindValue(m_rColumn++, value);
		}

		void operator()(short value) const
		{
			DBTRACE1_D(_T("- param(short): %d\n"), value);
			m_rStatement.BindValue(m_rColumn++, value);
		}

		void operator()(unsigned short value) const
		{
			DBTRACE1_D(_T("- param(ushort): %u\n"), value);
			m_rStatement.BindValue(m_rColumn++, value);
		}

		void operator()(int value) const
		{
			DBTRACE1_D(_T("- param(int): %ld\n"), value);
			m_rStatement.BindValue(m_rColumn++, value);
		}

		void operator()(unsigned int value) const
		{
			DBTRACE1_D(_T("- param(uint): %lu\n"), value);
			m_rStatement.BindValue(m_rColumn++, value);
		}

		void operator()(long value) const
		{
			DBTRACE1_D(_T("- param(long): %ld\n"), value);
			m_rStatement.BindValue(m_rColumn++, value);
		}

		void operator()(unsigned long value) const
		{
			DBTRACE1_D(_T("- param(ulong): %lu\n"), value);
			m_rStatement.BindValue(m_rColumn++, value);
		}

		void operator()(long long value) const
		{
			DBTRACE1_D(_T("- param(longlong): %I64d\n"), value);
			m_rStatement.BindValue(m_rColumn++, value);
		}

		void operator()(unsigned long long value) const
		{
			DBTRACE1_D(_T("- param(ulonglong): %I64u\n"), value);
			m_rStatement.BindValue(m_rColumn++, value);
		}

		void operator()(double value) const
		{
			DBTRACE1_D(_T("- param(double): %f\n"), value);
			m_rStatement.BindValue(m_rColumn++, value);
		}

		void operator()(const TString& value) const
		{
			DBTRACE1_D(_T("- param(string): '%s'\n"), (PCTSTR)value);
			m_rStatement.BindValue(m_rColumn++, value);
		}

		void operator()(const TSmartPath& value) const
		{
			DBTRACE1_D(_T("- param(path): %s\n"), value.ToString());
			m_rStatement.BindValue(m_rColumn++, value);
		}

		int& m_rColumn;
		sqlite::TSQLiteStatement& m_rStatement;
	};

}


///////////////////////////////////////////////////////////////////////////
TRowID::TRowID(const TSQLiteColumnsDefinition& rColumnDefinition) :
	m_bitset(rColumnDefinition.GetCount() + 1)	// count of columns and a bit for add/modify
{
}

TRowID::~TRowID()
{
}

void TRowID::SetAddedBit(bool bAdded)
{
	m_bitset[0] = bAdded;
}

void TRowID::SetColumnBit(size_t stIndex, bool bColumnExists)
{
	++stIndex;

	if(stIndex >= m_bitset.size())
		THROW_SERIALIZER_EXCEPTION(eErr_BoundsExceeded, _T("Row identification info cannot be generated"));

	m_bitset[stIndex] = bColumnExists;
}

bool TRowID::operator==(const TRowID rSrc) const
{
	return m_bitset == rSrc.m_bitset;
}

bool TRowID::operator<(const TRowID rSrc) const
{
	return m_bitset < rSrc.m_bitset;
}

void TRowID::Clear()
{
	m_bitset.reset();
}

bool TRowID::HasAny() const
{
	return m_bitset.any();
}

///////////////////////////////////////////////////////////////////////////
TSQLiteSerializerRowData::TSQLiteSerializerRowData(size_t stRowID, TSQLiteColumnsDefinition& rColumnDefinition, bool bAdded) :
	m_stRowID(stRowID),
	m_rColumns(rColumnDefinition),
	m_bAdded(bAdded)
{
}

TSQLiteSerializerRowData::~TSQLiteSerializerRowData()
{
}

ISerializerRowData& TSQLiteSerializerRowData::operator%(const TRowData& rData)
{
	size_t stColumn = m_rColumns.GetColumnIndex(rData.m_strColName);
	std::map<size_t, TRowData::InternalVariant>::iterator iterFnd = m_mapValues.find(stColumn);
	if(iterFnd == m_mapValues.end())
		m_mapValues.insert(std::make_pair(stColumn, rData.m_varValue));
	else
		(*iterFnd).second = rData.m_varValue;

	return *this;
}

ISerializerRowData& TSQLiteSerializerRowData::SetValue(const TRowData& rData)
{
	size_t stColumn = m_rColumns.GetColumnIndex(rData.m_strColName);
	std::map<size_t, TRowData::InternalVariant>::iterator iterFnd = m_mapValues.find(stColumn);
	if(iterFnd == m_mapValues.end())
		m_mapValues.insert(std::make_pair(stColumn, rData.m_varValue));
	else
		(*iterFnd).second = rData.m_varValue;

	return *this;
}

void TSQLiteSerializerRowData::BindParamsAndExec(sqlite::TSQLiteStatement& tStatement)
{
	using namespace sqlite;

	if(m_bAdded)
	{
		// exec query
		int iColumn = 1;
		tStatement.BindValue(iColumn++, m_stRowID);
		for(MapVariants::iterator iterVariant = m_mapValues.begin(); iterVariant != m_mapValues.end(); ++iterVariant)
		{
			boost::apply_visitor(SQLiteBindValueVisitor(tStatement, iColumn), iterVariant->second);
		}

		tStatement.Step();
	}
	else if(!m_mapValues.empty())
	{
		int iColumn = 1;
		for(MapVariants::iterator iterVariant = m_mapValues.begin(); iterVariant != m_mapValues.end(); ++iterVariant)
		{
			boost::apply_visitor(SQLiteBindValueVisitor(tStatement, iColumn), iterVariant->second);
		}
		tStatement.BindValue(iColumn++, m_stRowID);

		tStatement.Step();

		int iChanges = tStatement.Changes();
		_ASSERTE(iChanges == 1);
		if(iChanges != 1)
			THROW_SERIALIZER_EXCEPTION(eErr_InvalidData, _T("Update query did not update record in the database"));
	}
}

TString TSQLiteSerializerRowData::GetQuery(const TString& strContainerName) const
{
	if(m_bAdded)
	{
		// prepare insert query
		TString strQuery = boost::str(boost::wformat(L"INSERT INTO %1%(id,") % strContainerName).c_str();
		TString strParams;

		for(MapVariants::const_iterator iterVariant = m_mapValues.begin(); iterVariant != m_mapValues.end(); ++iterVariant)
		{
			strQuery += boost::str(boost::wformat(_T("%1%,")) % m_rColumns.GetColumnName(iterVariant->first)).c_str();
			strParams += _T("?,");
		}

		strQuery.TrimRightSelf(_T(","));
		strQuery += _T(") VALUES(?,");

		strParams.TrimRightSelf(_T(","));
		strQuery += strParams;
		strQuery += _T(")");

		return strQuery;
	}
	else if(!m_mapValues.empty())
	{
		// prepare update query
		TString strQuery = boost::str(boost::wformat(L"UPDATE %1% SET ") % strContainerName).c_str();

		for(MapVariants::const_iterator iterVariant = m_mapValues.begin(); iterVariant != m_mapValues.end(); ++iterVariant)
		{
			strQuery += boost::str(boost::wformat(_T("%1%=?,")) % m_rColumns.GetColumnName(iterVariant->first)).c_str();
		}

		strQuery.TrimRightSelf(_T(","));
		strQuery += _T(" WHERE id=?");

		return strQuery;
	}
	else
		return TString();
}

TRowID TSQLiteSerializerRowData::GetChangeIdentification() const
{
	TRowID rowID(m_rColumns);
	rowID.SetAddedBit(m_bAdded);
	for(MapVariants::const_iterator iterVariant = m_mapValues.begin(); iterVariant != m_mapValues.end(); ++iterVariant)
	{
		rowID.SetColumnBit(iterVariant->first, true);
	}

	return rowID;
}

void TSQLiteSerializerRowData::MarkAsAdded()
{
	m_bAdded = true;
}

END_CHCORE_NAMESPACE
