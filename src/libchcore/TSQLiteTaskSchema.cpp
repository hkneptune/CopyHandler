// ============================================================================
//  Copyright (C) 2001-2014 by Jozef Starosczyk
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
#include "TSQLiteTaskSchema.h"
#include "TSQLiteTransaction.h"
#include "TSerializerVersion.h"
#include "TSQLiteStatement.h"

BEGIN_CHCORE_NAMESPACE

TSQLiteTaskSchema::TSQLiteTaskSchema()
{
}

TSQLiteTaskSchema::~TSQLiteTaskSchema()
{
}

void TSQLiteTaskSchema::Setup(const sqlite::TSQLiteDatabasePtr& spDatabase)
{
	sqlite::TSQLiteTransaction tTransaction(spDatabase);

	// check version of the database
	TSerializerVersion tVersion(spDatabase);

	// if version is 0, then this is the fresh database with (almost) no tables inside
	if(tVersion.GetVersion() == 0)
	{
		sqlite::TSQLiteStatement tStatement(spDatabase);
		tStatement.Prepare(_T("CREATE TABLE task(id BIGINT UNIQUE, name varchar(256) NOT NULL, log_path VARCHAR(32768), current_state INT NOT NULL, destination_path varchar(32768) NOT NULL)"));
		tStatement.Step();

		tStatement.Prepare(_T("CREATE TABLE base_paths(id BIGINT UNIQUE, src_path varchar(32768) NOT NULL, skip_processing boolean NOT NULL, dst_path varchar(32768) NOT NULL)"));
		tStatement.Step();

		tStatement.Prepare(_T("CREATE TABLE scanned_files(id BIGINT UNIQUE, rel_path varchar(32768) NOT NULL, base_path_id BIGINT NOT NULL, attr INT NOT NULL, size BIGINT NOT NULL, time_created BIGINT NOT NULL, time_last_write BIGINT NOT NULL, time_last_access BIGINT NOT NULL, flags INT NOT NULL)"));
		tStatement.Step();

		// and finally set the database version to current one
		tVersion.SetVersion(1);
	}

	tTransaction.Commit();
}

END_CHCORE_NAMESPACE
