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
#include "TTaskManagerSerializer.h"
#include "TSQLiteDatabase.h"
#include "TSQLiteTransaction.h"
#include "TSQLiteException.h"
#include "TSerializerVersion.h"
#include "TSQLiteStatement.h"
#include "TTaskSerializer.h"
#include "TTaskInfo.h"
#include <boost/numeric/conversion/cast.hpp>

BEGIN_CHCORE_NAMESPACE

using namespace sqlite;

TTaskManagerSerializer::TTaskManagerSerializer(const TSmartPath& pathDB, const TSmartPath& pathTasksDir) :
	m_bSetupExecuted(false),
	m_pathDB(pathDB),
	m_pathTasksDir(pathTasksDir)
{
}

TTaskManagerSerializer::~TTaskManagerSerializer()
{
}

void TTaskManagerSerializer::Setup()
{
	TSQLiteDatabasePtr spDatabase = GetDatabase();
	TSQLiteTransaction tTransaction(spDatabase);

	// check version of the database
	TSerializerVersion tVersion(spDatabase);
	tVersion.Setup();

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

	m_bSetupExecuted = true;
}

void TTaskManagerSerializer::Store(const TTaskInfoContainer& tTasksInfo)
{
	if(!m_bSetupExecuted)
		Setup();

	TSQLiteDatabasePtr spDatabase(GetDatabase());
	TSQLiteTransaction tTransaction(spDatabase);

	TSQLiteStatement tStatement(spDatabase);

	// first delete existing items
	if(tTasksInfo.HasDeletions())
	{
		tStatement.Prepare(_T("DELETE FROM tasks WHERE task_id = ?1"));

		for(size_t stIndex = 0; stIndex < tTasksInfo.GetDeletedCount(); ++stIndex)
		{
			taskid_t tTaskID = tTasksInfo.GetDeletedAt(stIndex);

			tStatement.BindValue(1, tTaskID);
			tStatement.Step();
		}
	}

	if(tTasksInfo.HasAdditions())
	{
		tStatement.Prepare(_T("INSERT INTO tasks(task_id, task_order, path) VALUES(?1, ?2, ?3)"));

		for(size_t stIndex = 0; stIndex < tTasksInfo.GetCount(); ++stIndex)
		{
			const TTaskInfoEntry& rEntry = tTasksInfo.GetAt(stIndex);
			if(rEntry.IsAdded())
			{
				tStatement.BindValue(1, rEntry.GetTaskID());
				tStatement.BindValue(2, rEntry.GetOrder());
				tStatement.BindValue(3, rEntry.GetTaskPath().ToString());
				tStatement.Step();
			}
		}
	}

	if(tTasksInfo.HasModifications())
	{
		// right now both order and path are updated regardless if only one of them changed.
		// might be optimized in the future (not optimizing now as expected traffic is very low).
		tStatement.Prepare(_T("UPDATE tasks SET task_order=?2, path=?3 WHERE task_id=?1"));

		for(size_t stIndex = 0; stIndex < tTasksInfo.GetCount(); ++stIndex)
		{
			const TTaskInfoEntry& rEntry = tTasksInfo.GetAt(stIndex);
			if(rEntry.IsModified())
			{
				tStatement.BindValue(1, rEntry.GetTaskID());
				tStatement.BindValue(2, rEntry.GetOrder());
				tStatement.BindValue(3, rEntry.GetTaskPath().ToString());
				tStatement.Step();
			}
		}
	}

	tTransaction.Commit();
}

void TTaskManagerSerializer::Load(TTaskInfoContainer& tTasksInfo)
{
	if(!m_bSetupExecuted)
		Setup();

	tTasksInfo.Clear();

	TSQLiteDatabasePtr spDatabase(GetDatabase());
	TSQLiteTransaction tTransaction(spDatabase);

	TSQLiteStatement tStatement(spDatabase);

	tStatement.Prepare(_T("SELECT task_id, task_order, path FROM tasks"));
	while(tStatement.Step() == TSQLiteStatement::eStep_HasRow)
	{
		taskid_t tTaskID = boost::numeric_cast<taskid_t>(tStatement.GetUInt64(0));
		int iOrder = tStatement.GetInt(1);
		TString strPath = tStatement.GetText(2);

		tTasksInfo.Add(tTaskID, PathFromWString(strPath), iOrder, TTaskPtr());
	}
}

sqlite::TSQLiteDatabasePtr TTaskManagerSerializer::GetDatabase()
{
	if(!m_spDatabase)
		m_spDatabase.reset(new TSQLiteDatabase(m_pathDB.ToString()));

	return m_spDatabase;
}

chcore::ITaskSerializerPtr TTaskManagerSerializer::CreateTaskSerializer(const TSmartPath& pathSerialize)
{
	TTaskSerializerPtr spTaskSerializer(new TTaskSerializer(pathSerialize));
	return spTaskSerializer;
}

END_CHCORE_NAMESPACE
