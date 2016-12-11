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
#include "TSQLiteTaskManagerSchema.h"
#include "TSQLiteTransaction.h"
#include "TSerializerVersion.h"
#include "TSQLiteStatement.h"

namespace serializer
{
	using namespace sqlite;

	TSQLiteTaskManagerSchema::TSQLiteTaskManagerSchema()
	{
	}

	TSQLiteTaskManagerSchema::~TSQLiteTaskManagerSchema()
	{
	}

	void TSQLiteTaskManagerSchema::Setup(const sqlite::TSQLiteDatabasePtr& spDatabase)
	{
		TSQLiteTransaction tTransaction(spDatabase);

		// check version of the database
		TSerializerVersion tVersion(spDatabase);

		// if version is 0, then this is the fresh database with (almost) no tables inside
		if (tVersion.GetVersion() == 0)
		{
			TSQLiteStatement tStatement(spDatabase);

			tStatement.Prepare(_T("CREATE TABLE tasks(id BIGINT UNIQUE PRIMARY KEY, task_order INT NOT NULL, path VARCHAR(32768) NOT NULL, logpath VARCHAR(32768) NOT NULL)"));
			tStatement.Step();

			// and finally set the database version to current one
			tVersion.SetVersion(1);
		}
		if (tVersion.GetVersion() == 1)
		{
			TSQLiteStatement tStatement(spDatabase);

			tStatement.Prepare(_T("CREATE TABLE obsolete_tasks(id BIGINT UNIQUE PRIMARY KEY, path VARCHAR(32768) NOT NULL)"));
			tStatement.Step();

			// and finally set the database version to current one
			tVersion.SetVersion(2);
		}

		if(tVersion.GetVersion() == 2)
		{
			TSQLiteStatement tStatement(spDatabase);

			tStatement.Prepare(_T("ALTER TABLE tasks RENAME TO tasks_old"));
			tStatement.Step();

			tStatement.Prepare(_T("CREATE TABLE tasks(id BIGINT UNIQUE PRIMARY KEY, task_order INT NOT NULL, path VARCHAR(32768) NOT NULL, logpath VARCHAR(32768) NOT NULL)"));
			tStatement.Step();


			tStatement.Prepare(_T("INSERT INTO tasks(id, task_order, path, logpath) SELECT id, task_order, path, replace(path, '.sqlite', '.log') FROM tasks_old"));
			tStatement.Step();

			tStatement.Prepare(_T("DROP TABLE tasks_old"));
			tStatement.Step();

			tVersion.SetVersion(3);
		}

		tTransaction.Commit();
	}
}
