#include "stdafx.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../TSQLiteDatabase.h"

using namespace chcore::sqlite;

TEST(SQLiteDatabase, CreationWithVerification)
{
	TSQLiteDatabase db(_T(":memory:"));
	EXPECT_TRUE(db.GetHandle() != NULL);
	EXPECT_FALSE(db.GetInTransaction());
}
