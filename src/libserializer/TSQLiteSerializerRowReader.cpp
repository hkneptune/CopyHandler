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
#include "../libchcore/ErrorCodes.h"
#include <boost/format.hpp>
#include "TSQLiteStatement.h"
#include "../libchcore/TCoreException.h"

using namespace string;
using namespace chcore;

namespace serializer
{
	TSQLiteSerializerRowReader::TSQLiteSerializerRowReader(const sqlite::TSQLiteDatabasePtr& spDatabase, TSQLiteColumnsDefinition& rColumns, const TString& strContainerName, const logger::TLogFileDataPtr& spLogFileData) :
		m_bInitialized(false),
		m_spStatement(new sqlite::TSQLiteStatement(spDatabase)),
		m_rColumns(rColumns),
		m_strContainerName(strContainerName),
		m_spLog(logger::MakeLogger(spLogFileData, L"Serializer-RowReader"))
	{
		if (m_strContainerName.IsEmpty())
			throw TCoreException(eErr_InvalidArgument, L"m_strContainerName", LOCATION);
	}

	TSQLiteSerializerRowReader::~TSQLiteSerializerRowReader()
	{
	}

	bool TSQLiteSerializerRowReader::Next()
	{
		if (m_rColumns.IsEmpty())
			throw TCoreException(eErr_SerializeLoadError, L"m_rColumns is empty", LOCATION);

		if (!m_bInitialized)
		{
			// generate query to retrieve data from db
			TString strQuery;
			strQuery = boost::str(boost::wformat(L"SELECT %1% FROM %2% ORDER BY id") % m_rColumns.GetCommaSeparatedColumns().c_str() % m_strContainerName.c_str()).c_str();

			LOG_TRACE(m_spLog) << L"Executing query: " << strQuery;
			m_spStatement->Prepare(strQuery.c_str());
			m_bInitialized = true;
		}

		return m_spStatement->Step() == sqlite::TSQLiteStatement::eStep_HasRow;
	}

	void TSQLiteSerializerRowReader::GetValue(const TString& strColName, bool& bValue)
	{
		if (!m_bInitialized)
			throw TCoreException(eErr_SerializeLoadError, L"Serializer not initialized", LOCATION);

		m_spStatement->GetValue(GetColumnIndex(strColName), bValue);
	}

	void TSQLiteSerializerRowReader::GetValue(const TString& strColName, short& iValue)
	{
		if (!m_bInitialized)
			throw TCoreException(eErr_SerializeLoadError, L"Serializer not initialized", LOCATION);

		m_spStatement->GetValue(GetColumnIndex(strColName), iValue);
	}

	void TSQLiteSerializerRowReader::GetValue(const TString& strColName, unsigned short& uiValue)
	{
		if (!m_bInitialized)
			throw TCoreException(eErr_SerializeLoadError, L"Serializer not initialized", LOCATION);

		m_spStatement->GetValue(GetColumnIndex(strColName), uiValue);
	}

	void TSQLiteSerializerRowReader::GetValue(const TString& strColName, int& iValue)
	{
		if (!m_bInitialized)
			throw TCoreException(eErr_SerializeLoadError, L"Serializer not initialized", LOCATION);

		m_spStatement->GetValue(GetColumnIndex(strColName), iValue);
	}

	void TSQLiteSerializerRowReader::GetValue(const TString& strColName, unsigned int& uiValue)
	{
		if (!m_bInitialized)
			throw TCoreException(eErr_SerializeLoadError, L"Serializer not initialized", LOCATION);

		m_spStatement->GetValue(GetColumnIndex(strColName), uiValue);
	}

	void TSQLiteSerializerRowReader::GetValue(const TString& strColName, long& lValue)
	{
		if (!m_bInitialized)
			throw TCoreException(eErr_SerializeLoadError, L"Serializer not initialized", LOCATION);

		m_spStatement->GetValue(GetColumnIndex(strColName), lValue);
	}

	void TSQLiteSerializerRowReader::GetValue(const TString& strColName, unsigned long& ulValue)
	{
		if (!m_bInitialized)
			throw TCoreException(eErr_SerializeLoadError, L"Serializer not initialized", LOCATION);

		m_spStatement->GetValue(GetColumnIndex(strColName), ulValue);
	}

	void TSQLiteSerializerRowReader::GetValue(const TString& strColName, long long& llValue)
	{
		if (!m_bInitialized)
			throw TCoreException(eErr_SerializeLoadError, L"Serializer not initialized", LOCATION);

		m_spStatement->GetValue(GetColumnIndex(strColName), llValue);
	}

	void TSQLiteSerializerRowReader::GetValue(const TString& strColName, unsigned long long& ullValue)
	{
		if (!m_bInitialized)
			throw TCoreException(eErr_SerializeLoadError, L"Serializer not initialized", LOCATION);

		m_spStatement->GetValue(GetColumnIndex(strColName), ullValue);
	}

	void TSQLiteSerializerRowReader::GetValue(const TString& strColName, double& dValue)
	{
		if (!m_bInitialized)
			throw TCoreException(eErr_SerializeLoadError, L"Serializer not initialized", LOCATION);

		m_spStatement->GetValue(GetColumnIndex(strColName), dValue);
	}

	void TSQLiteSerializerRowReader::GetValue(const TString& strColName, TString& strValue)
	{
		if (!m_bInitialized)
			throw TCoreException(eErr_SerializeLoadError, L"Serializer not initialized", LOCATION);

		m_spStatement->GetValue(GetColumnIndex(strColName), strValue);
	}

	void TSQLiteSerializerRowReader::GetValue(const TString& strColName, TSmartPath& pathValue)
	{
		if (!m_bInitialized)
			throw TCoreException(eErr_SerializeLoadError, L"Serializer not initialized", LOCATION);

		m_spStatement->GetValue(GetColumnIndex(strColName), pathValue);
	}

	int TSQLiteSerializerRowReader::GetColumnIndex(const TString& strColName) const
	{
		if (!m_bInitialized)
			throw TCoreException(eErr_SerializeLoadError, L"Serializer not initialized", LOCATION);

		size_t stColumn = m_rColumns.GetColumnIndex(strColName.c_str());
		return boost::numeric_cast<int>(stColumn);
	}
}
