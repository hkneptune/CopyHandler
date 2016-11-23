#include "stdafx.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../TSQLiteTransaction.h"
#include "../TSQLiteDatabase.h"
#include "../TSQLiteStatement.h"
#include "../TSQLiteException.h"

using namespace chcore;
using namespace sqlite;

TEST(SQLiteTransaction, BeginTransactionWithDefaultRollback_Empty)
{
	TSQLiteDatabasePtr spDB(new TSQLiteDatabase(PathFromString(_T(":memory:"))));

	// separate scope for the transaction
	{
		TSQLiteTransaction tran(spDB);
		EXPECT_TRUE(spDB->GetInTransaction());
	}
	EXPECT_FALSE(spDB->GetInTransaction());
}

TEST(SQLiteTransaction, BeginTransactionWithDefaultRollback_WithData)
{
	TSQLiteDatabasePtr spDB(new TSQLiteDatabase(PathFromString(_T(":memory:"))));
	TSQLiteStatement tStatement(spDB);

	// separate scope for the transaction
	{
		TSQLiteTransaction tran(spDB);

		tStatement.Prepare(_T("CREATE TABLE test(col1 INTEGER, col2 VARCHAR(40))"));
		tStatement.Step();

		tStatement.Prepare(_T("INSERT INTO test(col1, col2) VALUES(?1, ?2)"));
		tStatement.BindValue(1, 54);
		tStatement.BindValue(2, _T("Some Value"));
		tStatement.Step();
	}

	// rollback seem to revert the schema changes, so this statement can't be processed due to missing table
	EXPECT_THROW(tStatement.Prepare(_T("SELECT count(*) FROM test")), TSQLiteException);
}

TEST(SQLiteTransaction, BeginTransactionWithCommit)
{
	TSQLiteDatabasePtr spDB(new TSQLiteDatabase(PathFromString(_T(":memory:"))));

	// separate scope for the transaction
	TSQLiteTransaction tran(spDB);

	tran.Commit();
	EXPECT_FALSE(spDB->GetInTransaction());
}

TEST(SQLiteTransaction, BeginTransactionWithCommit_WithData)
{
	TSQLiteDatabasePtr spDB(new TSQLiteDatabase(PathFromString(_T(":memory:"))));
	TSQLiteStatement tStatement(spDB);

	// separate scope for the transaction
	{
		TSQLiteTransaction tran(spDB);

		tStatement.Prepare(_T("CREATE TABLE test(col1 INTEGER, col2 VARCHAR(40))"));
		tStatement.Step();

		tStatement.Prepare(_T("INSERT INTO test(col1, col2) VALUES(?1, ?2)"));
		tStatement.BindValue(1, 54);
		tStatement.BindValue(2, _T("Some Value"));
		tStatement.Step();

		tran.Commit();
	}

	// rollback seem to revert the schema changes, so this statement can't be processed due to missing table
	tStatement.Prepare(_T("SELECT count(*) FROM test"));
	tStatement.Step();

	EXPECT_EQ(1, tStatement.GetInt(0));
}
