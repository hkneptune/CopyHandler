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
#include "TTaskManagerSchema.h"
#include "TSQLiteTransaction.h"
#include "TSerializerVersion.h"
#include "TSQLiteStatement.h"

BEGIN_CHCORE_NAMESPACE

using namespace sqlite;

TTaskManagerSchema::TTaskManagerSchema()
{
}

TTaskManagerSchema::~TTaskManagerSchema()
{
}

void TTaskManagerSchema::Setup(const sqlite::TSQLiteDatabasePtr& spDatabase)
{
	TSQLiteTransaction tTransaction(spDatabase);

	// check version of the database
	TSerializerVersion tVersion(spDatabase);

	// if version is 0, then this is the fresh database with (almost) no tables inside
	if(tVersion.GetVersion() == 0)
	{
		TSQLiteStatement tStatement(spDatabase);
		tStatement.Prepare(_T("CREATE TABLE tasks(task_id BIGINT UNIQUE, task_order INT, path VARCHAR(32768))"));
		tStatement.Step();

		// and finally set the database version to current one
		tVersion.SetVersion(1);
	}

	tTransaction.Commit();
}

END_CHCORE_NAMESPACE
