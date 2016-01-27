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

namespace chcore
{
	TSQLiteColumnsDefinition::TSQLiteColumnsDefinition()
	{
	}

	TSQLiteColumnsDefinition::~TSQLiteColumnsDefinition()
	{
	}

	size_t TSQLiteColumnsDefinition::AddColumn(const TString& strColumnName, ETypes eColType)
	{
		m_vColumns.push_back(std::make_pair(strColumnName, eColType));
		return m_vColumns.size() - 1;
	}

	void TSQLiteColumnsDefinition::Clear()
	{
		m_vColumns.clear();
	}

	size_t TSQLiteColumnsDefinition::GetColumnIndex(const wchar_t* strColumnName)
	{
		size_t stPos = 0;
		for (VecColumns::const_iterator iterFnd = m_vColumns.begin(); iterFnd != m_vColumns.end(); ++iterFnd)
		{
			if (iterFnd->first == strColumnName)
				return stPos;
			++stPos;
		}

		throw TCoreException(eErr_BoundsExceeded, L"Column name not found", LOCATION);
	}

	const TString& TSQLiteColumnsDefinition::GetColumnName(size_t stIndex) const
	{
		return m_vColumns.at(stIndex).first;
	}

	size_t TSQLiteColumnsDefinition::GetCount() const
	{
		return m_vColumns.size();
	}

	bool TSQLiteColumnsDefinition::IsEmpty() const
	{
		return m_vColumns.empty();
	}

	TString TSQLiteColumnsDefinition::GetCommaSeparatedColumns() const
	{
		TString strColumns;
		VecColumns::value_type pairCol;
		BOOST_FOREACH(pairCol, m_vColumns)
		{
			strColumns += pairCol.first + _T(",");
		}

		strColumns.TrimRightSelf(_T(","));
		return strColumns;
	}

	IColumnsDefinition::ETypes TSQLiteColumnsDefinition::GetColumnType(size_t stIndex) const
	{
		return m_vColumns.at(stIndex).second;
	}
}
