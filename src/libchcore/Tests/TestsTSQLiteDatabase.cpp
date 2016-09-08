#include "stdafx.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../TSQLiteDatabase.h"

using namespace chcore;
using namespace sqlite;

TEST(SQLiteDatabase, CreationWithVerification)
{
	TSQLiteDatabase db(PathFromString(_T(":memory:")));
	EXPECT_TRUE(db.GetHandle() != NULL);
	EXPECT_FALSE(db.GetInTransaction());
}
