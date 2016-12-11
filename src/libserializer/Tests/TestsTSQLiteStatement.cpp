#include "stdafx.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../TSQLiteDatabase.h"
#include "../TSQLiteStatement.h"
#include "../TSQLiteException.h"

using namespace serializer;
using namespace sqlite;
using namespace chcore;
using namespace string;

TEST(SQLiteStatement, CorrectPrepare)
{
	TSQLiteDatabasePtr spDB(new TSQLiteDatabase(PathFromString(_T(":memory:"))));
	TSQLiteStatement tStatement(spDB);

	tStatement.Prepare(_T("CREATE TABLE test(col1 INTEGER, col2 VARCHAR(40))"));
	EXPECT_EQ(TSQLiteStatement::eStep_Finished, tStatement.Step());
}

TEST(SQLiteStatement, IncorrectPrepare)
{
	TSQLiteDatabasePtr spDB(new TSQLiteDatabase(PathFromString(_T(":memory:"))));
	TSQLiteStatement tStatement(spDB);

	EXPECT_THROW(tStatement.Prepare(_T("CREATE incorrect TABLE test(col1 INTEGER, col2 VARCHAR(40))")), TSQLiteException);
}

TEST(SQLiteStatement, PreparedStep)
{
	TSQLiteDatabasePtr spDB(new TSQLiteDatabase(PathFromString(_T(":memory:"))));
	TSQLiteStatement tStatement(spDB);

	tStatement.Prepare(_T("CREATE TABLE test(col1 INTEGER, col2 VARCHAR(40))"));
	EXPECT_EQ(tStatement.Step(), TSQLiteStatement::eStep_Finished);
}

TEST(SQLiteStatement, UnpreparedStep)
{
	TSQLiteDatabasePtr spDB(new TSQLiteDatabase(PathFromString(_T(":memory:"))));
	TSQLiteStatement tStatement(spDB);

	EXPECT_THROW(tStatement.Step(), TSQLiteException);
}

TEST(SQLiteStatement, UnpreparedBind)
{
	TSQLiteDatabasePtr spDB(new TSQLiteDatabase(PathFromString(_T(":memory:"))));
	TSQLiteStatement tStatement(spDB);

	// insert data
	EXPECT_THROW(tStatement.BindValue(1, 54), TSQLiteException);
}

TEST(SQLiteStatement, InsertAndRetrieveData)
{
	TSQLiteDatabasePtr spDB(new TSQLiteDatabase(PathFromString(_T(":memory:"))));
	TSQLiteStatement tStatement(spDB);

	// create schema
	tStatement.Prepare(_T("CREATE TABLE test(col1 INTEGER, col2 VARCHAR(40))"));
	EXPECT_EQ(TSQLiteStatement::eStep_Finished, tStatement.Step());

	// insert data
	tStatement.Prepare(_T("INSERT INTO test(col1, col2) VALUES(?1, ?2)"));
	tStatement.BindValue(1, 54);
	tStatement.BindValue(2, _T("Some Value"));
	EXPECT_EQ(TSQLiteStatement::eStep_Finished, tStatement.Step());

	// retrieve data
	tStatement.Prepare(_T("SELECT col2, col1 FROM test"));
	EXPECT_EQ(TSQLiteStatement::eStep_HasRow, tStatement.Step());
	EXPECT_EQ(54, tStatement.GetInt(1));
	EXPECT_EQ(TString(_T("Some Value")), tStatement.GetText(0));
}

TEST(SQLiteStatement, ClearBindings)
{
	TSQLiteDatabasePtr spDB(new TSQLiteDatabase(PathFromString(_T(":memory:"))));
	TSQLiteStatement tStatement(spDB);

	// create schema
	tStatement.Prepare(_T("CREATE TABLE test(col1 INTEGER, col2 VARCHAR(40))"));
	EXPECT_EQ(TSQLiteStatement::eStep_Finished, tStatement.Step());

	// insert data
	tStatement.Prepare(_T("INSERT INTO test(col1, col2) VALUES(?1, ?2)"));

	tStatement.BindValue(1, 54);
	tStatement.BindValue(2, _T("Some Value"));
	EXPECT_EQ(TSQLiteStatement::eStep_Finished, tStatement.Step());

	tStatement.BindValue(1, 32);
	tStatement.BindValue(2, _T("???"));
	EXPECT_EQ(TSQLiteStatement::eStep_Finished, tStatement.Step());

	// retrieve data
	tStatement.Prepare(_T("SELECT col2, col1 FROM test ORDER BY col1"));
	EXPECT_EQ(TSQLiteStatement::eStep_HasRow, tStatement.Step());

	EXPECT_EQ(32, tStatement.GetInt(1));
	EXPECT_EQ(TString(_T("???")), tStatement.GetText(0));

	EXPECT_EQ(TSQLiteStatement::eStep_HasRow, tStatement.Step());

	EXPECT_EQ(54, tStatement.GetInt(1));
	EXPECT_EQ(TString(_T("Some Value")), tStatement.GetText(0));

	EXPECT_EQ(TSQLiteStatement::eStep_Finished, tStatement.Step());
}
