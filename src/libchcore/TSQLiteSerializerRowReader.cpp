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
#include "TSQLiteSerializerRowReader.h"
#include <boost/numeric/conversion/cast.hpp>
#include "TCoreException.h"
#include "ErrorCodes.h"
#include <boost/format.hpp>
#include "TSQLiteStatement.h"

BEGIN_CHCORE_NAMESPACE

TSQLiteSerializerRowReader::TSQLiteSerializerRowReader(const sqlite::TSQLiteDatabasePtr& spDatabase, const TSQLiteColumnDefinitionPtr& spColumns, const TString& strContainerName) :
	m_spStatement(new sqlite::TSQLiteStatement(spDatabase)),
	m_spColumns(spColumns),
	m_bInitialized(false),
	m_strContainerName(strContainerName)
{
	if(!m_spColumns || m_strContainerName.IsEmpty())
		THROW_CORE_EXCEPTION(eErr_InvalidArgument);
}

TSQLiteSerializerRowReader::~TSQLiteSerializerRowReader()
{
}

bool TSQLiteSerializerRowReader::Next()
{
	if(m_spColumns->IsEmpty())
		THROW_CORE_EXCEPTION(eErr_SerializeLoadError);

	if(!m_bInitialized)
	{
		// generate query to retrieve data from db
		TString strQuery;
		strQuery = boost::str(boost::wformat(L"SELECT %1% FROM %2%") % (PCTSTR)m_spColumns->GetCommaSeparatedColumns() % (PCTSTR)m_strContainerName).c_str();

		m_spStatement->Prepare(strQuery);
		m_bInitialized = true;
	}

	return m_spStatement->Step() == sqlite::TSQLiteStatement::eStep_HasRow;
}

void TSQLiteSerializerRowReader::GetValue(const TString& strColName, bool& bValue)
{
	if(!m_bInitialized)
		THROW_CORE_EXCEPTION(eErr_SerializeLoadError);

	bValue = m_spStatement->GetInt(GetColumnIndex(strColName)) != 0;
}

void TSQLiteSerializerRowReader::GetValue(const TString& strColName, short& iValue)
{
	if(!m_bInitialized)
		THROW_CORE_EXCEPTION(eErr_SerializeLoadError);

	iValue = boost::numeric_cast<short>(m_spStatement->GetInt(GetColumnIndex(strColName)));
}

void TSQLiteSerializerRowReader::GetValue(const TString& strColName, unsigned short& uiValue)
{
	if(!m_bInitialized)
		THROW_CORE_EXCEPTION(eErr_SerializeLoadError);

	uiValue = boost::numeric_cast<unsigned short>(m_spStatement->GetUInt(GetColumnIndex(strColName)));
}

void TSQLiteSerializerRowReader::GetValue(const TString& strColName, int& iValue)
{
	if(!m_bInitialized)
		THROW_CORE_EXCEPTION(eErr_SerializeLoadError);

	iValue = m_spStatement->GetInt(GetColumnIndex(strColName));
}

void TSQLiteSerializerRowReader::GetValue(const TString& strColName, unsigned int& uiValue)
{
	if(!m_bInitialized)
		THROW_CORE_EXCEPTION(eErr_SerializeLoadError);

	uiValue = m_spStatement->GetUInt(GetColumnIndex(strColName));
}

void TSQLiteSerializerRowReader::GetValue(const TString& strColName, long long& llValue)
{
	if(!m_bInitialized)
		THROW_CORE_EXCEPTION(eErr_SerializeLoadError);

	llValue = m_spStatement->GetInt64(GetColumnIndex(strColName));
}

void TSQLiteSerializerRowReader::GetValue(const TString& strColName, unsigned long long& ullValue)
{
	if(!m_bInitialized)
		THROW_CORE_EXCEPTION(eErr_SerializeLoadError);

	ullValue = m_spStatement->GetUInt64(GetColumnIndex(strColName));
}

void TSQLiteSerializerRowReader::GetValue(const TString& strColName, double& dValue)
{
	if(!m_bInitialized)
		THROW_CORE_EXCEPTION(eErr_SerializeLoadError);

	dValue = m_spStatement->GetDouble(GetColumnIndex(strColName));
}

void TSQLiteSerializerRowReader::GetValue(const TString& strColName, TString& strValue)
{
	if(!m_bInitialized)
		THROW_CORE_EXCEPTION(eErr_SerializeLoadError);

	strValue = m_spStatement->GetText(GetColumnIndex(strColName));
}

void TSQLiteSerializerRowReader::GetValue(const TString& strColName, TSmartPath& pathValue)
{
	if(!m_bInitialized)
		THROW_CORE_EXCEPTION(eErr_SerializeLoadError);

	pathValue = PathFromString(m_spStatement->GetText(GetColumnIndex(strColName)));
}

int TSQLiteSerializerRowReader::GetColumnIndex(const TString& strColName) const
{
	if(!m_bInitialized)
		THROW_CORE_EXCEPTION(eErr_SerializeLoadError);

	size_t stColumn = m_spColumns->GetColumnIndex(strColName);
	return boost::numeric_cast<int>(stColumn);
}

IColumnsDefinitionPtr TSQLiteSerializerRowReader::GetColumnsDefinitions() const
{
	return m_spColumns;
}

END_CHCORE_NAMESPACE
