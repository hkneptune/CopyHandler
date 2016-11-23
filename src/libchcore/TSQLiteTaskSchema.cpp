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
#include <tchar.h>

namespace chcore
{
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
		// that's why the initialization always creates the latest version of database, whithout going
		// through all the intermediate migrations
		if (tVersion.GetVersion() == 0)
			CreateNewDatabase(spDatabase, tVersion);
		else
		{
			// migrate current database to the newest version
			if (tVersion.GetVersion() == 1)
				Migrate_001_002(spDatabase, tVersion);

			if (tVersion.GetVersion() == 2)
				Migrate_002_003(spDatabase, tVersion);

			if (tVersion.GetVersion() == 3)
				Migrate_003_004(spDatabase, tVersion);

			if(tVersion.GetVersion() == 4)
				Migrate_004_005(spDatabase, tVersion);
		}

		tTransaction.Commit();
	}

	void TSQLiteTaskSchema::CreateNewDatabase(const sqlite::TSQLiteDatabasePtr& spDatabase, TSerializerVersion &tVersion)
	{
		sqlite::TSQLiteStatement tStatement(spDatabase);
		tStatement.Prepare(_T("CREATE TABLE task(id BIGINT UNIQUE, name varchar(256) NOT NULL, log_path VARCHAR(32768), current_state INT NOT NULL, destination_path varchar(32768) NOT NULL)"));
		tStatement.Step();

		tStatement.Prepare(_T("CREATE TABLE base_paths(id BIGINT UNIQUE, src_path varchar(32768) NOT NULL, skip_processing boolean NOT NULL, dst_path varchar(32768) NOT NULL)"));
		tStatement.Step();

		tStatement.Prepare(_T("CREATE TABLE scanned_files(id BIGINT UNIQUE, rel_path varchar(32768) NOT NULL, base_path_id BIGINT NOT NULL, attr INT NOT NULL, size BIGINT NOT NULL, time_created BIGINT NOT NULL, time_last_write BIGINT NOT NULL, time_last_access BIGINT NOT NULL, flags INT NOT NULL)"));
		tStatement.Step();

		tStatement.Prepare(_T("CREATE TABLE task_config(id BIGINT UNIQUE, name varchar(256) NOT NULL, node_order INT NOT NULL, value varchar(32768) NOT NULL)"));
		tStatement.Step();

		tStatement.Prepare(_T("CREATE TABLE filters(id BIGINT UNIQUE, use_mask INT NOT NULL, mask varchar(32768) NOT NULL, ")
			_T("use_exclude_mask INT NOT NULL, exclude_mask varchar(32768) NOT NULL, ")
			_T("use_size_1 INT NOT NULL, compare_type_1 INT NOT NULL, size_1 BIGINT NOT NULL, ")
			_T("use_size_2 INT NOT NULL, compare_type_2 INT NOT NULL, size_2 BIGINT NOT NULL, ")
			_T("date_type INT NOT NULL,")
			_T("use_date_time_1 INT NOT NULL, date_compare_type_1 INT NOT NULL, use_date_1 INT NOT NULL, use_time_1 INT NOT NULL, datetime_1 varchar(64) NOT NULL, ")
			_T("use_date_time_2 INT NOT NULL, date_compare_type_2 INT NOT NULL, use_date_2 INT NOT NULL, use_time_2 INT NOT NULL, datetime_2 varchar(64) NOT NULL, ")
			_T("use_attributes INT NOT NULL, attr_archive INT NOT NULL, attr_ro INT NOT NULL, attr_hidden INT NOT NULL, attr_system INT NOT NULL, attr_directory INT NOT NULL")
			_T(")"));
		tStatement.Step();

		tStatement.Prepare(_T("CREATE TABLE local_stats(id BIGINT UNIQUE, elapsed_time BIGINT NOT NULL)"));
		tStatement.Step();

		tStatement.Prepare(_T("CREATE TABLE subtasks(id BIGINT UNIQUE, type INT NOT NULL, is_current boolean NOT NULL, is_estimation boolean NOT NULL)"));
		tStatement.Step();

		tStatement.Prepare(_T("CREATE TABLE subtasks_info(id BIGINT UNIQUE, operation INT NOT NULL)"));
		tStatement.Step();

		tStatement.Prepare(_T("CREATE TABLE subtask_fastmove(id BIGINT UNIQUE, current_index INT NOT NULL, is_running boolean NOT NULL, is_initialized boolean NOT NULL, total_size BIGINT NOT NULL, processed_size BIGINT NOT NULL, size_speed varchar(1024) NOT NULL, ")
			_T("total_count BIGINT NOT NULL, processed_count BIGINT NOT NULL, count_speed varchar(1024) NOT NULL, ci_processed_size BIGINT NOT NULL, ci_total_size BIGINT NOT NULL, timer BIGINT NOT NULL, ")
			_T("buffer_index INT NOT NULL, current_path varchar(32768) NOT NULL, ci_silent_resume boolean NOT NULL)"));
		tStatement.Step();

		tStatement.Prepare(_T("CREATE TABLE subtask_scan(id BIGINT UNIQUE, current_index INT NOT NULL, is_running boolean NOT NULL, is_initialized boolean NOT NULL, total_size BIGINT NOT NULL, processed_size BIGINT NOT NULL, size_speed varchar(1024) NOT NULL, ")
			_T("total_count BIGINT NOT NULL, processed_count BIGINT NOT NULL, count_speed varchar(1024) NOT NULL, ci_processed_size BIGINT NOT NULL, ci_total_size BIGINT NOT NULL, timer BIGINT NOT NULL, ")
			_T("buffer_index INT NOT NULL, current_path varchar(32768) NOT NULL, ci_silent_resume boolean NOT NULL)"));
		tStatement.Step();

		tStatement.Prepare(_T("CREATE TABLE subtask_delete(id BIGINT UNIQUE, current_index INT NOT NULL, is_running boolean NOT NULL, is_initialized boolean NOT NULL, total_size BIGINT NOT NULL, processed_size BIGINT NOT NULL, size_speed varchar(1024) NOT NULL, ")
			_T("total_count BIGINT NOT NULL, processed_count BIGINT NOT NULL, count_speed varchar(1024) NOT NULL, ci_processed_size BIGINT NOT NULL, ci_total_size BIGINT NOT NULL, timer BIGINT NOT NULL, ")
			_T("buffer_index INT NOT NULL, current_path varchar(32768) NOT NULL, ci_silent_resume boolean NOT NULL)"));
		tStatement.Step();

		tStatement.Prepare(_T("CREATE TABLE subtask_copymove(id BIGINT UNIQUE, current_index INT NOT NULL, is_running boolean NOT NULL, is_initialized boolean NOT NULL, total_size BIGINT NOT NULL, processed_size BIGINT NOT NULL, size_speed varchar(1024) NOT NULL, ")
			_T("total_count BIGINT NOT NULL, processed_count BIGINT NOT NULL, count_speed varchar(1024) NOT NULL, ci_processed_size BIGINT NOT NULL, ci_total_size BIGINT NOT NULL, timer BIGINT NOT NULL, ")
			_T("buffer_index INT NOT NULL, current_path varchar(32768) NOT NULL, ci_silent_resume boolean NOT NULL)"));
		tStatement.Step();

		tStatement.Prepare(_T("CREATE TABLE feedback(id BIGINT UNIQUE, file_error INT NOT NULL, file_already_exists INT NOT NULL, not_enough_space INT NOT NULL, operation_finished INT NOT NULL, operation_error INT NOT NULL)"));
		tStatement.Step();

		// and finally set the database version to current one
		tVersion.SetVersion(5);
	}

	void TSQLiteTaskSchema::Migrate_001_002(const sqlite::TSQLiteDatabasePtr& spDatabase, TSerializerVersion &tVersion)
	{
		sqlite::TSQLiteStatement tStatement(spDatabase);

		tStatement.Prepare(_T("ALTER TABLE subtask_fastmove ADD COLUMN ci_silent_resume boolean NOT NULL DEFAULT false"));
		tStatement.Step();
		tStatement.Prepare(_T("ALTER TABLE subtask_delete ADD COLUMN ci_silent_resume boolean NOT NULL DEFAULT false"));
		tStatement.Step();
		tStatement.Prepare(_T("ALTER TABLE subtask_copymove ADD COLUMN ci_silent_resume boolean NOT NULL DEFAULT false"));
		tStatement.Step();

		tVersion.SetVersion(2);
	}

	void TSQLiteTaskSchema::Migrate_002_003(const sqlite::TSQLiteDatabasePtr& spDatabase, TSerializerVersion &tVersion)
	{
		sqlite::TSQLiteStatement tStatement(spDatabase);

		// drop column from fastmove
		tStatement.Prepare(_T("CREATE TABLE subtask_fastmove_new(id BIGINT UNIQUE, current_index INT NOT NULL, is_running boolean NOT NULL, is_initialized boolean NOT NULL, total_size BIGINT NOT NULL, processed_size BIGINT NOT NULL, size_speed varchar(1024) NOT NULL, ")
			_T("total_count BIGINT NOT NULL, processed_count BIGINT NOT NULL, count_speed varchar(1024) NOT NULL, ci_processed_size BIGINT NOT NULL, ci_total_size BIGINT NOT NULL, timer BIGINT NOT NULL, ")
			_T("buffer_index INT NOT NULL, current_path varchar(32768) NOT NULL, ci_silent_resume boolean NOT NULL)"));
		tStatement.Step();

		tStatement.Prepare(_T("INSERT INTO subtask_fastmove_new(id, current_index, is_running, is_initialized, total_size, processed_size, size_speed, ")
			_T("total_count, processed_count, count_speed, ci_processed_size, ci_total_size, timer, buffer_index, current_path, ci_silent_resume)")
			_T("SELECT id, current_index, is_running, is_initialized, total_size, processed_size, size_speed, ")
			_T("total_count, processed_count, count_speed, ci_processed_size, ci_total_size, timer, buffer_index, current_path, ci_silent_resume ")
			_T("FROM subtask_fastmove"));
		tStatement.Step();

		tStatement.Prepare(_T("DROP TABLE subtask_fastmove"));
		tStatement.Step();

		tStatement.Prepare(_T("ALTER TABLE subtask_fastmove_new RENAME TO subtask_fastmove"));
		tStatement.Step();

		// drop column from subtask delete
		tStatement.Prepare(_T("CREATE TABLE subtask_delete_new(id BIGINT UNIQUE, current_index INT NOT NULL, is_running boolean NOT NULL, is_initialized boolean NOT NULL, total_size BIGINT NOT NULL, processed_size BIGINT NOT NULL, size_speed varchar(1024) NOT NULL, ")
			_T("total_count BIGINT NOT NULL, processed_count BIGINT NOT NULL, count_speed varchar(1024) NOT NULL, ci_processed_size BIGINT NOT NULL, ci_total_size BIGINT NOT NULL, timer BIGINT NOT NULL, ")
			_T("buffer_index INT NOT NULL, current_path varchar(32768) NOT NULL, ci_silent_resume boolean NOT NULL)"));
		tStatement.Step();

		//id, current_index, is_running, is_initialized, total_size, processed_size, size_speed, total_count, processed_count, count_speed, ci_processed_size, ci_total_size, timer, buffer_index, current_path, ci_silent_resume
		tStatement.Prepare(_T("INSERT INTO subtask_delete_new(id, current_index, is_running, is_initialized, total_size, processed_size, size_speed, total_count, processed_count, count_speed, ci_processed_size, ci_total_size, timer, buffer_index, current_path, ci_silent_resume)")
			_T("SELECT id, current_index, is_running, is_initialized, total_size, processed_size, size_speed, total_count, processed_count, count_speed, ci_processed_size, ci_total_size, timer, buffer_index, current_path, ci_silent_resume ")
			_T("FROM subtask_delete"));
		tStatement.Step();

		tStatement.Prepare(_T("DROP TABLE subtask_delete"));
		tStatement.Step();

		tStatement.Prepare(_T("ALTER TABLE subtask_delete_new RENAME TO subtask_delete"));
		tStatement.Step();

		// drop column from subtask copymove
		tStatement.Prepare(_T("CREATE TABLE subtask_copymove_new(id BIGINT UNIQUE, current_index INT NOT NULL, is_running boolean NOT NULL, is_initialized boolean NOT NULL, total_size BIGINT NOT NULL, processed_size BIGINT NOT NULL, size_speed varchar(1024) NOT NULL, ")
			_T("total_count BIGINT NOT NULL, processed_count BIGINT NOT NULL, count_speed varchar(1024) NOT NULL, ci_processed_size BIGINT NOT NULL, ci_total_size BIGINT NOT NULL, timer BIGINT NOT NULL, ")
			_T("buffer_index INT NOT NULL, current_path varchar(32768) NOT NULL, ci_silent_resume boolean NOT NULL)"));
		tStatement.Step();

		//id, current_index, is_running, is_initialized, total_size, processed_size, size_speed, total_count, processed_count, count_speed, ci_processed_size, ci_total_size, timer, buffer_index, current_path, ci_silent_resume
		tStatement.Prepare(_T("INSERT INTO subtask_copymove_new(id, current_index, is_running, is_initialized, total_size, processed_size, size_speed, total_count, processed_count, count_speed, ci_processed_size, ci_total_size, timer, buffer_index, current_path, ci_silent_resume)")
			_T("SELECT id, current_index, is_running, is_initialized, total_size, processed_size, size_speed, total_count, processed_count, count_speed, ci_processed_size, ci_total_size, timer, buffer_index, current_path, ci_silent_resume ")
			_T("FROM subtask_copymove"));
		tStatement.Step();

		tStatement.Prepare(_T("DROP TABLE subtask_copymove"));
		tStatement.Step();

		tStatement.Prepare(_T("ALTER TABLE subtask_copymove_new RENAME TO subtask_copymove"));
		tStatement.Step();

		tVersion.SetVersion(3);
	}

	void TSQLiteTaskSchema::Migrate_003_004(const sqlite::TSQLiteDatabasePtr& spDatabase, TSerializerVersion &tVersion)
	{
		sqlite::TSQLiteStatement tStatement(spDatabase);

		tStatement.Prepare(_T("CREATE TABLE feedback(id BIGINT UNIQUE, file_error INT NOT NULL, file_already_exists INT NOT NULL, not_enough_space INT NOT NULL, operation_finished INT NOT NULL, operation_error INT NOT NULL)"));
		tStatement.Step();

		// for tasks being migrated we have to insert some defaults for feedback
		tStatement.Prepare(_T("INSERT INTO feedback(id, file_error, file_already_exists, not_enough_space, operation_finished, operation_error) VALUES(?1, ?2, ?3, ?4, ?5, ?6)"));
		tStatement.BindValue(1, 0);
		tStatement.BindValue(2, 0);
		tStatement.BindValue(3, 0);
		tStatement.BindValue(4, 0);
		tStatement.BindValue(5, 0);
		tStatement.BindValue(6, 0);

		tStatement.Step();

		tVersion.SetVersion(4);
	}

	void TSQLiteTaskSchema::Migrate_004_005(const sqlite::TSQLiteDatabasePtr& spDatabase, TSerializerVersion &tVersion)
	{
		sqlite::TSQLiteStatement tStatement(spDatabase);

		// new table for scan stats
		tStatement.Prepare(_T("CREATE TABLE subtask_scan(id BIGINT UNIQUE, current_index INT NOT NULL, is_running boolean NOT NULL, is_initialized boolean NOT NULL, total_size BIGINT NOT NULL, processed_size BIGINT NOT NULL, size_speed varchar(1024) NOT NULL, ")
			_T("total_count BIGINT NOT NULL, processed_count BIGINT NOT NULL, count_speed varchar(1024) NOT NULL, ci_processed_size BIGINT NOT NULL, ci_total_size BIGINT NOT NULL, timer BIGINT NOT NULL, ")
			_T("buffer_index INT NOT NULL, current_path varchar(32768) NOT NULL, ci_silent_resume boolean NOT NULL)"));
		tStatement.Step();

		tVersion.SetVersion(5);
	}
}
