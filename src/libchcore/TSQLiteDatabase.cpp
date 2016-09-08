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
#include "TSQLiteDatabase.h"
#include "sqlite3/sqlite3.h"
#include "ErrorCodes.h"
#include "TSQLiteException.h"

namespace chcore
{
	namespace sqlite
	{
		TSQLiteDatabase::TSQLiteDatabase(const TSmartPath& pathDatabase) :
			m_pDBHandle(nullptr),
			m_bInTransaction(false),
			m_pathDatabase(pathDatabase)
		{
			int iResult = sqlite3_open16(m_pathDatabase.ToString(), &m_pDBHandle);
			if (iResult != SQLITE_OK)
			{
				const wchar_t* pszMsg = (const wchar_t*)sqlite3_errmsg16(m_pDBHandle);
				throw TSQLiteException(eErr_SQLiteCannotOpenDatabase, iResult, pszMsg, LOCATION);
			}
		}

		TSQLiteDatabase::~TSQLiteDatabase()
		{
			int iResult = sqlite3_close_v2(m_pDBHandle);	// handles properly the nullptr DB Handle
			iResult;
			_ASSERTE(iResult == SQLITE_OK);
		}

		HANDLE TSQLiteDatabase::GetHandle()
		{
			return m_pDBHandle;
		}

		bool TSQLiteDatabase::GetInTransaction() const
		{
			return m_bInTransaction;
		}

		void TSQLiteDatabase::SetInTransaction(bool bInTransaction)
		{
			m_bInTransaction = bInTransaction;
		}

		TSmartPath TSQLiteDatabase::GetLocation() const
		{
			return m_pathDatabase;
		}
	}
}
