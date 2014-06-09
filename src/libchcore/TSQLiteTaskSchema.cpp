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

		tStatement.Prepare(_T("CREATE TABLE subtask_fastmove(id BIGINT UNIQUE, current_index INT NOT NULL, is_running boolean NOT NULL, total_size BIGINT NOT NULL, processed_size BIGINT NOT NULL, size_speed varchar(1024) NOT NULL, ")
							_T("total_count BIGINT NOT NULL, processed_count BIGINT NOT NULL, count_speed varchar(1024) NOT NULL, ci_processed_size BIGINT NOT NULL, ci_total_size BIGINT NOT NULL, timer BIGINT NOT NULL, ")
							_T("buffer_index INT NOT NULL, current_path varchar(32768) NOT NULL, suboperation_type INT NOT NULL)"));
		tStatement.Step();

		tStatement.Prepare(_T("CREATE TABLE subtask_delete(id BIGINT UNIQUE, current_index INT NOT NULL, is_running boolean NOT NULL, total_size BIGINT NOT NULL, processed_size BIGINT NOT NULL, size_speed varchar(1024) NOT NULL, ")
							_T("total_count BIGINT NOT NULL, processed_count BIGINT NOT NULL, count_speed varchar(1024) NOT NULL, ci_processed_size BIGINT NOT NULL, ci_total_size BIGINT NOT NULL, timer BIGINT NOT NULL, ")
							_T("buffer_index INT NOT NULL, current_path varchar(32768) NOT NULL, suboperation_type INT NOT NULL)"));
		tStatement.Step();

		tStatement.Prepare(_T("CREATE TABLE subtask_copymove(id BIGINT UNIQUE, current_index INT NOT NULL, cf_processed_size BIGINT NOT NULL, is_running boolean NOT NULL, total_size BIGINT NOT NULL, processed_size BIGINT NOT NULL, size_speed varchar(1024) NOT NULL, ")
							_T("total_count BIGINT NOT NULL, processed_count BIGINT NOT NULL, count_speed varchar(1024) NOT NULL, ci_processed_size BIGINT NOT NULL, ci_total_size BIGINT NOT NULL, timer BIGINT NOT NULL, ")
							_T("buffer_index INT NOT NULL, current_path varchar(32768) NOT NULL, suboperation_type INT NOT NULL)"));
		tStatement.Step();

		// and finally set the database version to current one
		tVersion.SetVersion(1);
	}

	tTransaction.Commit();
}

END_CHCORE_NAMESPACE
