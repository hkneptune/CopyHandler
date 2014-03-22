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
#include "TSQLiteSerializerRowWriter.h"
#include "TSQLiteSerializerContainer.h"

BEGIN_CHCORE_NAMESPACE

TSQLiteSerializerRowWriter::TSQLiteSerializerRowWriter(size_t stRowID, const TSQLiteColumnDefinitionPtr& spColumnDefinition, bool bAdded) :
	m_stRowID(stRowID),
	m_spColumns(spColumnDefinition),
	m_bAdded(bAdded)
{
}

TSQLiteSerializerRowWriter::~TSQLiteSerializerRowWriter()
{
}

ISerializerRowWriter& TSQLiteSerializerRowWriter::operator%(const TRowData& rData)
{
	size_t stColumn = m_spColumns->GetColumnIndex(rData.m_strColName);
	std::map<size_t, TRowData::InternalVariant>::iterator iterFnd = m_mapValues.find(stColumn);
	if(iterFnd == m_mapValues.end())
		m_mapValues.insert(std::make_pair(stColumn, rData.m_varValue));
	else
		(*iterFnd).second = rData.m_varValue;

	return *this;
}

ISerializerRowWriter& TSQLiteSerializerRowWriter::SetValue(const TRowData& rData)
{
	size_t stColumn = m_spColumns->GetColumnIndex(rData.m_strColName);
	std::map<size_t, TRowData::InternalVariant>::iterator iterFnd = m_mapValues.find(stColumn);
	if(iterFnd == m_mapValues.end())
		m_mapValues.insert(std::make_pair(stColumn, rData.m_varValue));
	else
		(*iterFnd).second = rData.m_varValue;

	return *this;
}

END_CHCORE_NAMESPACE
