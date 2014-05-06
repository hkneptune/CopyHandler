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
#include "TSQLiteStatement.h"
#include "sqlite3/sqlite3.h"
#include "ErrorCodes.h"
#include "TSQLiteException.h"
#include <boost/numeric/conversion/cast.hpp>

BEGIN_CHCORE_NAMESPACE

namespace sqlite
{
	TSQLiteStatement::TSQLiteStatement(const TSQLiteDatabasePtr& spDatabase) :
		m_pStatement(NULL),
		m_spDatabase(spDatabase),
		m_bHasRow(false)
	{
		if(!m_spDatabase)
			THROW_SQLITE_EXCEPTION(eErr_InvalidArgument, 0, _T("Invalid database provided"));
	}

	TSQLiteStatement::~TSQLiteStatement()
	{
		int iResult = sqlite3_finalize(m_pStatement);
		iResult;
		_ASSERTE(iResult == SQLITE_OK);
	}

	void TSQLiteStatement::Close()
	{
		if(m_pStatement != NULL)
		{
			int iResult = sqlite3_finalize(m_pStatement);
			if(iResult != SQLITE_OK)
				THROW_SQLITE_EXCEPTION(eErr_SQLiteFinalizeError, iResult, _T("Cannot finalize statement"));
			m_pStatement = NULL;
		}
		m_bHasRow = false;
	}

	void TSQLiteStatement::Prepare(PCWSTR pszQuery)
	{
		Close();

		int iResult = sqlite3_prepare16_v2((sqlite3*)m_spDatabase->GetHandle(), pszQuery, -1, &m_pStatement, NULL);
		if(iResult != SQLITE_OK)
			THROW_SQLITE_EXCEPTION(eErr_SQLitePrepareError, iResult, (PCTSTR)sqlite3_errmsg16((sqlite3*)m_spDatabase->GetHandle()));
	}

	TSQLiteStatement::EStepResult TSQLiteStatement::Step()
	{
		m_bHasRow = false;

		if(!m_pStatement)
			THROW_SQLITE_EXCEPTION(eErr_SQLiteStatementNotPrepared, 0, _T("Tried to step on unprepared statement"));

		int iResult = sqlite3_step(m_pStatement);
		switch(iResult)
		{
		case SQLITE_ROW:
			m_bHasRow = true;
			return eStep_HasRow;
		case SQLITE_OK:
		case SQLITE_DONE:
			Reset();
			return eStep_Finished;
		default:
			THROW_SQLITE_EXCEPTION(eErr_SQLiteStepError, iResult, _T("Cannot perform step on the statement"));
		}
	}

	int TSQLiteStatement::Changes()
	{
		return sqlite3_changes((sqlite3*)m_spDatabase->GetHandle());
	}

	void TSQLiteStatement::BindValue(int iColumn, bool bValue)
	{
		BindValue(iColumn, bValue ? 1 : 0);
	}

	void TSQLiteStatement::BindValue(int iColumn, short siValue)
	{
		BindValue(iColumn, (int)siValue);
	}

	void TSQLiteStatement::BindValue(int iColumn, unsigned short usiValue)
	{
		BindValue(iColumn, (unsigned int)usiValue);
	}

	void TSQLiteStatement::BindValue(int iColumn, int iValue)
	{
		if(!m_pStatement)
			THROW_SQLITE_EXCEPTION(eErr_SQLiteStatementNotPrepared, 0, _T("Tried to step on unprepared statement"));

		int iResult = sqlite3_bind_int(m_pStatement, iColumn, iValue);
		if(iResult != SQLITE_OK)
			THROW_SQLITE_EXCEPTION(eErr_SQLiteBindError, iResult, _T("Cannot bind a parameter"));
	}

	void TSQLiteStatement::BindValue(int iColumn, unsigned int uiValue)
	{
		BindValue(iColumn, *(int*)&uiValue);
	}

	void TSQLiteStatement::BindValue(int iColumn, long lValue)
	{
		BindValue(iColumn, boost::numeric_cast<int>(lValue));
	}

	void TSQLiteStatement::BindValue(int iColumn, unsigned long ulValue)
	{
		BindValue(iColumn, boost::numeric_cast<unsigned int>(ulValue));
	}

	void TSQLiteStatement::BindValue(int iColumn, long long llValue)
	{
		if(!m_pStatement)
			THROW_SQLITE_EXCEPTION(eErr_SQLiteStatementNotPrepared, 0, _T("Tried to step on unprepared statement"));

		int iResult = sqlite3_bind_int64(m_pStatement, iColumn, llValue);
		if(iResult != SQLITE_OK)
			THROW_SQLITE_EXCEPTION(eErr_SQLiteBindError, iResult, _T("Cannot bind a parameter"));
	}

	void TSQLiteStatement::BindValue(int iColumn, unsigned long long ullValue)
	{
		BindValue(iColumn, *(long long*)&ullValue);
	}

	void TSQLiteStatement::BindValue(int iColumn, double dValue)
	{
		if(!m_pStatement)
			THROW_SQLITE_EXCEPTION(eErr_SQLiteStatementNotPrepared, 0, _T("Tried to step on unprepared statement"));

		int iResult = sqlite3_bind_double(m_pStatement, iColumn, dValue);
		if(iResult != SQLITE_OK)
			THROW_SQLITE_EXCEPTION(eErr_SQLiteBindError, iResult, _T("Cannot bind a parameter"));
	}

	void TSQLiteStatement::BindValue(int iColumn, PCTSTR pszText)
	{
		if(!m_pStatement)
			THROW_SQLITE_EXCEPTION(eErr_SQLiteStatementNotPrepared, 0, _T("Tried to step on unprepared statement"));

		int iResult = sqlite3_bind_text16(m_pStatement, iColumn, pszText, -1, SQLITE_TRANSIENT);
		if(iResult != SQLITE_OK)
			THROW_SQLITE_EXCEPTION(eErr_SQLiteBindError, iResult, _T("Cannot bind a parameter"));
	}

	void TSQLiteStatement::BindValue(int iColumn, const TString& strText)
	{
		BindValue(iColumn, (PCTSTR)strText);
	}

	void TSQLiteStatement::BindValue(int iColumn, const TSmartPath& path)
	{
		BindValue(iColumn, path.ToString());
	}


	bool TSQLiteStatement::GetBool(int iCol)
	{
		return GetInt(iCol) != 0;
	}

	short TSQLiteStatement::GetShort(int iCol)
	{
		return boost::numeric_cast<short>(GetInt(iCol));
	}

	unsigned short TSQLiteStatement::GetUShort(int iCol)
	{
		return boost::numeric_cast<unsigned short>(GetUInt(iCol));
	}

	int TSQLiteStatement::GetInt(int iCol)
	{
		if(!m_pStatement)
			THROW_SQLITE_EXCEPTION(eErr_SQLiteStatementNotPrepared, 0, _T("Tried to step on unprepared statement"));
		if(!m_bHasRow)
			THROW_SQLITE_EXCEPTION(eErr_SQLiteNoRowAvailable, 0, _T("No row available"));

		return sqlite3_column_int(m_pStatement, iCol);
	}

	unsigned int TSQLiteStatement::GetUInt(int iCol)
	{
		int iVal = GetInt(iCol);
		return *(unsigned int*)&iVal;
	}

	long TSQLiteStatement::GetLong(int iCol)
	{
		return boost::numeric_cast<long>(GetInt(iCol));
	}

	unsigned long TSQLiteStatement::GetULong(int iCol)
	{
		return boost::numeric_cast<unsigned long>(GetUInt(iCol));
	}

	long long TSQLiteStatement::GetInt64(int iCol)
	{
		if(!m_pStatement)
			THROW_SQLITE_EXCEPTION(eErr_SQLiteStatementNotPrepared, 0, _T("Tried to step on unprepared statement"));
		if(!m_bHasRow)
			THROW_SQLITE_EXCEPTION(eErr_SQLiteNoRowAvailable, 0, _T("No row available"));

		return sqlite3_column_int64(m_pStatement, iCol);
	}

	unsigned long long TSQLiteStatement::GetUInt64(int iCol)
	{
		long long llVal = GetInt64(iCol);
		return *(unsigned long long*)&llVal;
	}

	double TSQLiteStatement::GetDouble(int iCol)
	{
		if(!m_pStatement)
			THROW_SQLITE_EXCEPTION(eErr_SQLiteStatementNotPrepared, 0, _T("Tried to step on unprepared statement"));
		if(!m_bHasRow)
			THROW_SQLITE_EXCEPTION(eErr_SQLiteNoRowAvailable, 0, _T("No row available"));

		return sqlite3_column_double(m_pStatement, iCol);
	}

	TString TSQLiteStatement::GetText(int iCol)
	{
		if(!m_pStatement)
			THROW_SQLITE_EXCEPTION(eErr_SQLiteStatementNotPrepared, 0, _T("Tried to step on unprepared statement"));
		if(!m_bHasRow)
			THROW_SQLITE_EXCEPTION(eErr_SQLiteNoRowAvailable, 0, _T("No row available"));

		return TString((const wchar_t*)sqlite3_column_text16(m_pStatement, iCol));
	}

	TSmartPath TSQLiteStatement::GetPath(int iCol)
	{
		return PathFromWString(GetText(iCol));
	}

	void TSQLiteStatement::ClearBindings()
	{
		if(!m_pStatement)
			THROW_SQLITE_EXCEPTION(eErr_SQLiteStatementNotPrepared, 0, _T("Tried to step on unprepared statement"));

		int iResult = sqlite3_clear_bindings(m_pStatement);
		if(iResult != SQLITE_OK)
			THROW_SQLITE_EXCEPTION(eErr_SQLiteBindError, iResult, _T("Cannot clear bindings"));
	}

	void TSQLiteStatement::Reset()
	{
		if(!m_pStatement)
			THROW_SQLITE_EXCEPTION(eErr_SQLiteStatementNotPrepared, 0, _T("Tried to step on unprepared statement"));

		int iResult = sqlite3_reset(m_pStatement);
		if(iResult != SQLITE_OK)
			THROW_SQLITE_EXCEPTION(eErr_SQLiteBindError, iResult, _T("Cannot reset statement"));
	}

	void TSQLiteStatement::GetValue(int iCol, bool& bValue)
	{
		bValue = GetBool(iCol);
	}

	void TSQLiteStatement::GetValue(int iCol, short& iValue)
	{
		iValue = GetShort(iCol);
	}

	void TSQLiteStatement::GetValue(int iCol, unsigned short& uiValue)
	{
		uiValue = GetUShort(iCol);
	}

	void TSQLiteStatement::GetValue(int iCol, int& iValue)
	{
		iValue = GetInt(iCol);
	}

	void TSQLiteStatement::GetValue(int iCol, unsigned int& uiValue)
	{
		uiValue = GetUInt(iCol);
	}

	void TSQLiteStatement::GetValue(int iCol, long& lValue)
	{
		lValue = GetLong(iCol);
	}

	void TSQLiteStatement::GetValue(int iCol, unsigned long& ulValue)
	{
		ulValue = GetULong(iCol);
	}

	void TSQLiteStatement::GetValue(int iCol, long long& llValue)
	{
		llValue = GetInt64(iCol);
	}

	void TSQLiteStatement::GetValue(int iCol, unsigned long long& ullValue)
	{
		ullValue = GetUInt64(iCol);
	}

	void TSQLiteStatement::GetValue(int iCol, double& dValue)
	{
		dValue = GetDouble(iCol);
	}

	void TSQLiteStatement::GetValue(int iCol, TString& strValue)
	{
		strValue = GetText(iCol);
	}

	void TSQLiteStatement::GetValue(int iCol, TSmartPath& pathValue)
	{
		pathValue = GetPath(iCol);
	}
}

END_CHCORE_NAMESPACE
