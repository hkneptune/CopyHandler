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
#include "TSQLiteColumnDefinition.h"
#include "ErrorCodes.h"
#include "TCoreException.h"

BEGIN_CHCORE_NAMESPACE

TSQLiteColumnsDefinition::TSQLiteColumnsDefinition()
{
}

TSQLiteColumnsDefinition::~TSQLiteColumnsDefinition()
{
}

size_t TSQLiteColumnsDefinition::AddColumn(const TString& strColumnName)
{
	m_vColumns.push_back(strColumnName);
	return m_vColumns.size() - 1;
}

void TSQLiteColumnsDefinition::Clear()
{
	m_vColumns.clear();
}

size_t TSQLiteColumnsDefinition::GetColumnIndex(const TString& strColumnName, bool bAdd)
{
	std::vector<TString>::const_iterator iterFnd = std::find(m_vColumns.begin(), m_vColumns.end(), strColumnName);
	if(iterFnd == m_vColumns.end())
	{
		if(bAdd)
			return AddColumn(strColumnName);

		THROW_CORE_EXCEPTION(eErr_InvalidData);
	}

	std::vector<TString>::const_iterator iterBegin = m_vColumns.begin();
	return std::distance(iterBegin, iterFnd);
}

TString TSQLiteColumnsDefinition::GetColumnName(size_t stIndex) const
{
	return m_vColumns.at(stIndex);
}

size_t TSQLiteColumnsDefinition::GetCount() const
{
	return m_vColumns.size();
}

bool TSQLiteColumnsDefinition::IsEmpty() const
{
	return m_vColumns.empty();
}

IColumnsDefinition& TSQLiteColumnsDefinition::operator%(const TString& strColName)
{
	AddColumn(strColName);
	return *this;
}

chcore::TString TSQLiteColumnsDefinition::GetCommaSeparatedColumns() const
{
	TString strColumns;
	BOOST_FOREACH(const TString& strName, m_vColumns)
	{
		strColumns += strName + _T(",");
	}

	strColumns.TrimRightSelf(_T(","));
	return strColumns;
}

END_CHCORE_NAMESPACE
