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
#include "TSQLiteTransaction.h"
#include "TSQLiteException.h"
#include "sqlite3/sqlite3.h"

using namespace chcore;

namespace chengine
{
	namespace sqlite
	{
		TSQLiteTransaction::TSQLiteTransaction(const TSQLiteDatabasePtr& spDatabase) :
			m_spDatabase(spDatabase),
			m_bTransactionStarted(false)
		{
			if (!m_spDatabase)
				throw TSQLiteException(eErr_InvalidArgument, 0, _T("Invalid database provided"), LOCATION);
			Begin();
		}

		TSQLiteTransaction::~TSQLiteTransaction()
		{
			// try to rollback the transaction; this is the last resort
			if (m_bTransactionStarted && m_spDatabase->GetInTransaction())
			{
				int iResult = sqlite3_exec((sqlite3*)m_spDatabase->GetHandle(), "ROLLBACK TRANSACTION;", nullptr, nullptr, nullptr);
				iResult;
				_ASSERTE(iResult == SQLITE_OK);
				m_spDatabase->SetInTransaction(false);
			}
		}

		void TSQLiteTransaction::Begin()
		{
			if (m_spDatabase->GetInTransaction())
				return;

			if (m_bTransactionStarted)
				throw TSQLiteException(eErr_SQLiteCannotBeginTransaction, 0, _T("Transaction already started"), LOCATION);

			int iResult = sqlite3_exec((sqlite3*)m_spDatabase->GetHandle(), "BEGIN TRANSACTION", nullptr, nullptr, nullptr);
			if (iResult != SQLITE_OK)
				throw TSQLiteException(eErr_SQLiteCannotBeginTransaction, iResult, _T("Cannot begin transaction"), LOCATION);

			m_spDatabase->SetInTransaction(true);
			m_bTransactionStarted = true;
		}

		void TSQLiteTransaction::Rollback()
		{
			// no transactions whatsoever (even on database)
			if (!m_bTransactionStarted && !m_spDatabase->GetInTransaction())
				throw TSQLiteException(eErr_SQLiteCannotRollbackTransaction, 0, _T("Transaction not started"), LOCATION);

			// database has transaction started, but not by this object
			if (!m_bTransactionStarted)
				return;

			int iResult = sqlite3_exec((sqlite3*)m_spDatabase->GetHandle(), "ROLLBACK TRANSACTION;", nullptr, nullptr, nullptr);
			if (iResult != SQLITE_OK)
				throw TSQLiteException(eErr_SQLiteCannotRollbackTransaction, iResult, _T("Cannot rollback transaction"), LOCATION);
			m_spDatabase->SetInTransaction(false);
			m_bTransactionStarted = false;
		}

		void TSQLiteTransaction::Commit()
		{
			// no transactions whatsoever (even on database)
			if (!m_bTransactionStarted && !m_spDatabase->GetInTransaction())
				throw TSQLiteException(eErr_SQLiteCannotRollbackTransaction, 0, _T("Transaction not started"), LOCATION);

			// database has transaction started, but not by this object
			if (!m_bTransactionStarted)
				return;

			int iResult = sqlite3_exec((sqlite3*)m_spDatabase->GetHandle(), "COMMIT TRANSACTION;", nullptr, nullptr, nullptr);
			if (iResult != SQLITE_OK)
				throw TSQLiteException(eErr_SQLiteCannotCommitTransaction, iResult, _T("Cannot commit transaction"), LOCATION);
			m_spDatabase->SetInTransaction(false);
			m_bTransactionStarted = false;
		}
	}
}
