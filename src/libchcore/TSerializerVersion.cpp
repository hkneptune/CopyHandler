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
#include "TSerializerVersion.h"
#include "TSQLiteTransaction.h"
#include "ErrorCodes.h"
#include "TSerializerException.h"
#include "TSQLiteStatement.h"
#include "TSQLiteException.h"
#include "sqlite3\sqlite3.h"

namespace chcore
{
	using namespace sqlite;

	TSerializerVersion::TSerializerVersion(const TSQLiteDatabasePtr& spDatabase) :
		m_spDatabase(spDatabase),
		m_bSetupExecuted(false)
	{
		if (!spDatabase)
			throw TSerializerException(eErr_InvalidArgument, _T("No database provided"), LOCATION);
	}

	TSerializerVersion::~TSerializerVersion()
	{
	}

	void TSerializerVersion::Setup()
	{
		if (m_bSetupExecuted)
			return;

		TSQLiteTransaction tTransaction(m_spDatabase);
		TSQLiteStatement tStatement(m_spDatabase);

		tStatement.Prepare(_T("SELECT count(*) FROM sqlite_master WHERE type=?1 AND name=?2"));
		tStatement.BindValue(1, _T("table"));
		tStatement.BindValue(2, _T("version"));
		if (tStatement.Step() != TSQLiteStatement::eStep_HasRow)
			throw TSQLiteException(eErr_InternalProblem, SQLITE_ERROR, _T("Problem accessing sqlite_master table"), LOCATION);

		int iVersionCount = tStatement.GetInt(0);
		if (iVersionCount == 0)
		{
			// create table
			tStatement.Prepare(_T("CREATE TABLE IF NOT EXISTS version(db_version INT NOT NULL)"));
			tStatement.Step();

			tStatement.Prepare(_T("INSERT INTO version(db_version) VALUES(?1)"));
			tStatement.BindValue(1, 0);
			tStatement.Step();
		}

		tTransaction.Commit();

		m_bSetupExecuted = true;
	}

	int TSerializerVersion::GetVersion()
	{
		Setup();

		TSQLiteTransaction tTransaction(m_spDatabase);
		TSQLiteStatement tStatement(m_spDatabase);

		// when table does not exist the sqlite error is just SQLITE_ERROR when preparing statement
		tStatement.Prepare(_T("SELECT db_version FROM version"));
		if (tStatement.Step() == TSQLiteStatement::eStep_HasRow)
		{
			tTransaction.Commit();
			return tStatement.GetInt(0);
		}

		tTransaction.Commit();
		return 0;
	}

	void TSerializerVersion::SetVersion(int iNewVersion)
	{
		Setup();

		TSQLiteTransaction tTransaction(m_spDatabase);
		TSQLiteStatement tStatement(m_spDatabase);

		// create table
		tStatement.Prepare(_T("UPDATE version SET db_version=?1"));
		tStatement.BindValue(1, iNewVersion);
		tStatement.Step();

		tTransaction.Commit();
	}
}
