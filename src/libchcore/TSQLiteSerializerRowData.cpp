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
#include "TPlainStringPool.h"

namespace chcore
{
	///////////////////////////////////////////////////////////////////////////
	TSQLiteSerializerRowData::TSQLiteSerializerRowData(object_id_t oidRowID, TSQLiteColumnsDefinition& rColumnDefinition, bool bAdded, unsigned long long* pPoolMemory, size_t stPoolMemorySizeInBytes, TPlainStringPool& poolStrings) :
		m_rColumns(rColumnDefinition),
		m_pPoolMemory(pPoolMemory),
		m_poolStrings(poolStrings)
	{
		if (!m_pPoolMemory)
			throw TSerializerException(eErr_InvalidArgument, _T("Null memory provided"), LOCATION);
		if (rColumnDefinition.GetCount() > 63)
			throw TSerializerException(eErr_InternalProblem, _T("Serializer supports up to 63 columns. If more is needed the block header needs to be increased."), LOCATION);

		// initialize memory
		memset((void*)pPoolMemory, 0, stPoolMemorySizeInBytes);

		// set id
		size_t stIDIndex = rColumnDefinition.GetColumnIndex(_T("id"));
		InternalSetValue(stIDIndex, oidRowID);

		if (bAdded)
			MarkAsAdded();
	}

	TSQLiteSerializerRowData::TSQLiteSerializerRowData(const TSQLiteSerializerRowData& rSrc) :
		m_rColumns(rSrc.m_rColumns),
		m_pPoolMemory(rSrc.m_pPoolMemory),
		m_poolStrings(rSrc.m_poolStrings)
	{
	}

	TSQLiteSerializerRowData::~TSQLiteSerializerRowData()
	{
	}

	ISerializerRowData& TSQLiteSerializerRowData::SetValue(size_t stColIndex, bool bValue)
	{
		if (m_rColumns.GetColumnType(stColIndex) != IColumnsDefinition::eType_bool)
			throw TSerializerException(eErr_InvalidArgument, _T("Invalid argument type provided"), LOCATION);

		ModifyColumnData(stColIndex) = (bValue ? 1ULL : 0ULL);
		return *this;
	}

	ISerializerRowData& TSQLiteSerializerRowData::SetValue(size_t stColIndex, short siValue)
	{
		if (m_rColumns.GetColumnType(stColIndex) != IColumnsDefinition::eType_short)
			throw TSerializerException(eErr_InvalidArgument, _T("Invalid argument type provided"), LOCATION);

		ModifyColumnData(stColIndex) = (unsigned long long)*(unsigned short*)&siValue;
		return *this;
	}

	ISerializerRowData& TSQLiteSerializerRowData::SetValue(size_t stColIndex, unsigned short usiValue)
	{
		if (m_rColumns.GetColumnType(stColIndex) != IColumnsDefinition::eType_ushort)
			throw TSerializerException(eErr_InvalidArgument, _T("Invalid argument type provided"), LOCATION);

		ModifyColumnData(stColIndex) = (unsigned long long)usiValue;
		return *this;
	}

	ISerializerRowData& TSQLiteSerializerRowData::SetValue(size_t stColIndex, int iValue)
	{
		if (m_rColumns.GetColumnType(stColIndex) != IColumnsDefinition::eType_int)
			throw TSerializerException(eErr_InvalidArgument, _T("Invalid argument type provided"), LOCATION);

		ModifyColumnData(stColIndex) = (unsigned long long)*(unsigned int*)&iValue;
		return *this;
	}

	ISerializerRowData& TSQLiteSerializerRowData::SetValue(size_t stColIndex, unsigned int uiValue)
	{
		if (m_rColumns.GetColumnType(stColIndex) != IColumnsDefinition::eType_uint)
			throw TSerializerException(eErr_InvalidArgument, _T("Invalid argument type provided"), LOCATION);

		ModifyColumnData(stColIndex) = (unsigned long long)uiValue;
		return *this;
	}

	ISerializerRowData& TSQLiteSerializerRowData::SetValue(size_t stColIndex, long lValue)
	{
		if (m_rColumns.GetColumnType(stColIndex) != IColumnsDefinition::eType_long)
			throw TSerializerException(eErr_InvalidArgument, _T("Invalid argument type provided"), LOCATION);

		ModifyColumnData(stColIndex) = (unsigned long long)*(unsigned long*)&lValue;
		return *this;
	}

	ISerializerRowData& TSQLiteSerializerRowData::InternalSetValue(size_t stColIndex, unsigned long ulValue)
	{
		if(m_rColumns.GetColumnType(stColIndex) != IColumnsDefinition::eType_ulong)
			throw TSerializerException(eErr_InvalidArgument, _T("Invalid argument type provided"), LOCATION);

		ModifyColumnData(stColIndex) = (unsigned long long)ulValue;
		return *this;
	}

	ISerializerRowData& TSQLiteSerializerRowData::SetValue(size_t stColIndex, unsigned long ulValue)
	{
		InternalSetValue(stColIndex, ulValue);
		return *this;
	}

	ISerializerRowData& TSQLiteSerializerRowData::SetValue(size_t stColIndex, long long llValue)
	{
		if (m_rColumns.GetColumnType(stColIndex) != IColumnsDefinition::eType_longlong)
			throw TSerializerException(eErr_InvalidArgument, _T("Invalid argument type provided"), LOCATION);

		ModifyColumnData(stColIndex) = *(unsigned long long*)&llValue;
		return *this;
	}

	ISerializerRowData& TSQLiteSerializerRowData::SetValue(size_t stColIndex, unsigned long long ullValue)
	{
		if (m_rColumns.GetColumnType(stColIndex) != IColumnsDefinition::eType_ulonglong)
			throw TSerializerException(eErr_InvalidArgument, _T("Invalid argument type provided"), LOCATION);

		ModifyColumnData(stColIndex) = ullValue;
		return *this;
	}

	ISerializerRowData& TSQLiteSerializerRowData::SetValue(size_t stColIndex, double dValue)
	{
		if (m_rColumns.GetColumnType(stColIndex) != IColumnsDefinition::eType_double)
			throw TSerializerException(eErr_InvalidArgument, _T("Invalid argument type provided"), LOCATION);

		BOOST_STATIC_ASSERT(sizeof(double) == sizeof(unsigned long long));
		// cppcheck-suppress invalidPointerCast
		ModifyColumnData(stColIndex) = *(unsigned long long*)&dValue;
		return *this;
	}

	ISerializerRowData& TSQLiteSerializerRowData::SetValue(size_t stColIndex, const TString& strValue)
	{
		if (m_rColumns.GetColumnType(stColIndex) != IColumnsDefinition::eType_string)
			throw TSerializerException(eErr_InvalidArgument, _T("Invalid argument type provided"), LOCATION);

		if (strValue.IsEmpty())
			ModifyColumnData(stColIndex) = (unsigned long long)0;
		else
		{
			wchar_t* pszBuffer = m_poolStrings.AllocForString(strValue.c_str());
			ModifyColumnData(stColIndex) = (unsigned long long)(void*)pszBuffer;
		}

		return *this;
	}

	ISerializerRowData& TSQLiteSerializerRowData::SetValue(size_t stColIndex, const TSmartPath& pathValue)
	{
		if (m_rColumns.GetColumnType(stColIndex) != IColumnsDefinition::eType_path)
			throw TSerializerException(eErr_InvalidArgument, _T("Invalid argument type provided"), LOCATION);

		if (pathValue.IsEmpty())
			ModifyColumnData(stColIndex) = (unsigned long long)0;
		else
		{
			wchar_t* pszBuffer = m_poolStrings.AllocForString(pathValue.ToString());
			ModifyColumnData(stColIndex) = (unsigned long long)(void*)pszBuffer;
		}

		return *this;
	}

	ISerializerRowData& TSQLiteSerializerRowData::SetValue(const wchar_t* strColumnName, bool bValue)
	{
		return SetValue(m_rColumns.GetColumnIndex(strColumnName), bValue);
	}

	ISerializerRowData& TSQLiteSerializerRowData::SetValue(const wchar_t* strColumnName, short iValue)
	{
		return SetValue(m_rColumns.GetColumnIndex(strColumnName), iValue);
	}

	ISerializerRowData& TSQLiteSerializerRowData::SetValue(const wchar_t* strColumnName, unsigned short uiValue)
	{
		return SetValue(m_rColumns.GetColumnIndex(strColumnName), uiValue);
	}

	ISerializerRowData& TSQLiteSerializerRowData::SetValue(const wchar_t* strColumnName, int iValue)
	{
		return SetValue(m_rColumns.GetColumnIndex(strColumnName), iValue);
	}

	ISerializerRowData& TSQLiteSerializerRowData::SetValue(const wchar_t* strColumnName, unsigned int uiValue)
	{
		return SetValue(m_rColumns.GetColumnIndex(strColumnName), uiValue);
	}

	ISerializerRowData& TSQLiteSerializerRowData::SetValue(const wchar_t* strColumnName, long lValue)
	{
		return SetValue(m_rColumns.GetColumnIndex(strColumnName), lValue);
	}

	ISerializerRowData& TSQLiteSerializerRowData::SetValue(const wchar_t* strColumnName, unsigned long ulValue)
	{
		return SetValue(m_rColumns.GetColumnIndex(strColumnName), ulValue);
	}

	ISerializerRowData& TSQLiteSerializerRowData::SetValue(const wchar_t* strColumnName, long long llValue)
	{
		return SetValue(m_rColumns.GetColumnIndex(strColumnName), llValue);
	}

	ISerializerRowData& TSQLiteSerializerRowData::SetValue(const wchar_t* strColumnName, unsigned long long llValue)
	{
		return SetValue(m_rColumns.GetColumnIndex(strColumnName), llValue);
	}

	ISerializerRowData& TSQLiteSerializerRowData::SetValue(const wchar_t* strColumnName, double dValue)
	{
		return SetValue(m_rColumns.GetColumnIndex(strColumnName), dValue);
	}

	ISerializerRowData& TSQLiteSerializerRowData::SetValue(const wchar_t* strColumnName, const TString& strValue)
	{
		return SetValue(m_rColumns.GetColumnIndex(strColumnName), strValue);
	}

	ISerializerRowData& TSQLiteSerializerRowData::SetValue(const wchar_t* strColumnName, const TSmartPath& pathValue)
	{
		return SetValue(m_rColumns.GetColumnIndex(strColumnName), pathValue);
	}

	void TSQLiteSerializerRowData::BindParamsAndExec(sqlite::TSQLiteStatement& tStatement)
	{
		using namespace sqlite;

		if (IsAdded())
		{
			// exec query
			int iColumn = 1;
			BindParams(tStatement, iColumn);

			tStatement.Step();
		}
		else if (HasAnyData())
		{
			int iColumn = 1;

			size_t stIDColumnIndex = m_rColumns.GetColumnIndex(_T("id"));

			BindParams(tStatement, iColumn, stIDColumnIndex);

			// bind id as the last argument
			tStatement.BindValue(iColumn++, GetDataForColumn(stIDColumnIndex));
			tStatement.Step();

			int iChanges = tStatement.Changes();
			_ASSERTE(iChanges == 1);
			if (iChanges != 1)
				throw TSerializerException(eErr_InvalidData, _T("Update query did not update record in the database"), LOCATION);
		}
	}

	TString TSQLiteSerializerRowData::GetQuery(const TString& strContainerName) const
	{
		if (IsAdded())
		{
			// prepare insert query
			TString strQuery = boost::str(boost::wformat(L"INSERT INTO %1%(") % strContainerName).c_str();
			TString strParams;

			size_t stCount = m_rColumns.GetCount();
			for (size_t stIndex = 0; stIndex < stCount; ++stIndex)
			{
				strQuery += boost::str(boost::wformat(_T("%1%,")) % m_rColumns.GetColumnName(stIndex)).c_str();
				strParams += _T("?,");
			}

			strQuery.TrimRightSelf(_T(","));
			strQuery += _T(") VALUES(");

			strParams.TrimRightSelf(_T(","));
			strQuery += strParams;
			strQuery += _T(")");

			return strQuery;
		}
		else if (HasAnyData())
		{
			// prepare update query
			TString strQuery = boost::str(boost::wformat(L"UPDATE %1% SET ") % strContainerName).c_str();

			size_t stCountOfAssignments = 0;
			size_t stIDColumnIndex = m_rColumns.GetColumnIndex(_T("id"));
			size_t stCount = m_rColumns.GetCount();
			for (size_t stIndex = 0; stIndex < stCount; ++stIndex)
			{
				if (stIndex != stIDColumnIndex && HasData(stIndex))
				{
					strQuery += boost::str(boost::wformat(_T("%1%=?,")) % m_rColumns.GetColumnName(stIndex)).c_str();
					++stCountOfAssignments;
				}
			}

			if (stCountOfAssignments == 0)
				return TString();

			strQuery.TrimRightSelf(_T(","));
			strQuery += _T(" WHERE id=?");

			return strQuery;
		}
		else
			return TString();
	}

	unsigned long long TSQLiteSerializerRowData::GetChangeIdentification() const
	{
		return GetDataHeader();
	}

	void TSQLiteSerializerRowData::MarkAsAdded()
	{
		// first bit is always the "added" bit
		m_pPoolMemory[0] |= AddedBit;
	}

	const unsigned long long& TSQLiteSerializerRowData::GetDataForColumn(size_t stColIndex) const
	{
		return (m_pPoolMemory[stColIndex + 1]);
	}

	unsigned long long& TSQLiteSerializerRowData::ModifyColumnData(size_t stColIndex)
	{
		FreeColumnData(stColIndex);

		MarkColumnUsage(stColIndex, true);
		return (m_pPoolMemory[stColIndex + 1]);
	}

	void TSQLiteSerializerRowData::MarkColumnUsage(size_t stIndex, bool bUsed)
	{
		if (stIndex >= m_rColumns.GetCount())
			throw TSerializerException(eErr_BoundsExceeded, _T("Wrong column provided"), LOCATION);

		unsigned long long ullMask = 2ULL << stIndex;
		if (bUsed)
			m_pPoolMemory[0] |= ullMask;
		else
			m_pPoolMemory[0] &= ~ullMask;
	}

	bool TSQLiteSerializerRowData::IsAdded() const
	{
		return (m_pPoolMemory[0] & AddedBit) != 0;
	}

	bool TSQLiteSerializerRowData::HasAnyData() const
	{
		return GetDataHeader() != 0;
	}

	bool TSQLiteSerializerRowData::HasData(size_t stColumnIndex) const
	{
		return (GetDataHeader() & (2ULL << stColumnIndex)) != 0;
	}

	void TSQLiteSerializerRowData::BindParams(sqlite::TSQLiteStatement &tStatement, int& iSQLiteColumnNumber, size_t stSkipColumn)
	{
		size_t stCount = m_rColumns.GetCount();
		for (size_t stColumn = 0; stColumn < stCount; ++stColumn)
		{
			if (stColumn == stSkipColumn)
				continue;

			if (HasData(stColumn))
			{
				switch (m_rColumns.GetColumnType(stColumn))
				{
				case IColumnsDefinition::eType_bool:
				{
					bool bValue = GetDataForColumn(stColumn) != 0 ? true : false;
					DBTRACE1_D(_T("- param(bool): %ld\n"), bValue ? 1l : 0l);
					tStatement.BindValue(iSQLiteColumnNumber++, bValue);
					break;
				}

				case IColumnsDefinition::eType_short:
				{
					short siValue = *(short*)(unsigned short*)&GetDataForColumn(stColumn);
					DBTRACE1_D(_T("- param(short): %d\n"), siValue);
					tStatement.BindValue(iSQLiteColumnNumber++, siValue);
					break;
				}

				case IColumnsDefinition::eType_ushort:
				{
					unsigned short usiValue = (unsigned short)GetDataForColumn(stColumn);
					DBTRACE1_D(_T("- param(ushort): %u\n"), usiValue);
					tStatement.BindValue(iSQLiteColumnNumber++, usiValue);
					break;
				}

				case IColumnsDefinition::eType_int:
				{
					int iValue = *(int*)(unsigned int*)&GetDataForColumn(stColumn);
					DBTRACE1_D(_T("- param(int): %ld\n"), iValue);
					tStatement.BindValue(iSQLiteColumnNumber++, iValue);
					break;
				}

				case IColumnsDefinition::eType_uint:
				{
					unsigned int uiValue = (unsigned int)GetDataForColumn(stColumn);
					DBTRACE1_D(_T("- param(uint): %lu\n"), uiValue);
					tStatement.BindValue(iSQLiteColumnNumber++, uiValue);
					break;
				}

				case IColumnsDefinition::eType_long:
				{
					long lValue = *(long*)(unsigned long*)&GetDataForColumn(stColumn);
					DBTRACE1_D(_T("- param(long): %ld\n"), lValue);
					tStatement.BindValue(iSQLiteColumnNumber++, lValue);
					break;
				}

				case IColumnsDefinition::eType_ulong:
				{
					unsigned long ulValue = (unsigned long)GetDataForColumn(stColumn);
					DBTRACE1_D(_T("- param(ulong): %lu\n"), ulValue);
					tStatement.BindValue(iSQLiteColumnNumber++, ulValue);
					break;
				}

				case IColumnsDefinition::eType_longlong:
				{
					long long llValue = *(long long*)(unsigned long long*)&GetDataForColumn(stColumn);
					DBTRACE1_D(_T("- param(llong): %I64d\n"), llValue);
					tStatement.BindValue(iSQLiteColumnNumber++, llValue);
					break;
				}

				case IColumnsDefinition::eType_ulonglong:
				{
					unsigned long long ullValue = GetDataForColumn(stColumn);
					DBTRACE1_D(_T("- param(ullong): %I64u\n"), ullValue);
					tStatement.BindValue(iSQLiteColumnNumber++, ullValue);
					break;
				}

				case IColumnsDefinition::eType_double:
				{
					// cppcheck-suppress invalidPointerCast
					double dValue = *(double*)(unsigned long long*)&GetDataForColumn(stColumn);
					DBTRACE1_D(_T("- param(double): %f\n"), dValue);
					tStatement.BindValue(iSQLiteColumnNumber++, dValue);
					break;
				}

				case IColumnsDefinition::eType_string:
				{
					const wchar_t* pszValue = (const wchar_t*)(unsigned long long*)GetDataForColumn(stColumn);
					DBTRACE1_D(_T("- param(string): %s\n"), pszValue ? pszValue : _T(""));
					tStatement.BindValue(iSQLiteColumnNumber++, pszValue ? pszValue : _T(""));
					break;
				}

				case IColumnsDefinition::eType_path:
				{
					const wchar_t* pszValue = (const wchar_t*)(unsigned long long*)GetDataForColumn(stColumn);
					DBTRACE1_D(_T("- param(path): %s\n"), pszValue ? pszValue : _T(""));
					tStatement.BindValue(iSQLiteColumnNumber++, pszValue ? PathFromString(pszValue) : TSmartPath());
					break;
				}

				default:
					throw TSerializerException(eErr_InvalidArgument, _T("Invalid type"), LOCATION);
				}
			}
		}
	}

	unsigned long long TSQLiteSerializerRowData::GetDataHeader() const
	{
		return m_pPoolMemory[0];
	}

	void TSQLiteSerializerRowData::FreeColumnData(size_t stColumnID)
	{
		if (!HasData(stColumnID))
			return;

		switch (m_rColumns.GetColumnType(stColumnID))
		{
		case IColumnsDefinition::eType_path:
		case IColumnsDefinition::eType_string:
		{
			unsigned long long& ullColumnData = m_pPoolMemory[stColumnID + 1];
			ullColumnData = 0ULL;

			break;
		}
		}
	}

	void TSQLiteSerializerRowData::FreeAllColumnData()
	{
		size_t stCount = m_rColumns.GetCount();
		for (size_t stColumn = 0; stColumn < stCount; ++stColumn)
		{
			FreeColumnData(stColumn);
		}
	}

	TSQLiteSerializerRowData& TSQLiteSerializerRowData::operator=(const TSQLiteSerializerRowData& rSrc)
	{
		m_pPoolMemory = rSrc.m_pPoolMemory;

		return *this;
	}
}
