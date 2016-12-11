#include "stdafx.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../TSQLiteDatabase.h"

using namespace chengine;
using namespace sqlite;

TEST(SQLiteDatabase, CreationWithVerification)
{
	TSQLiteDatabase db(PathFromString(_T(":memory:")));
	EXPECT_TRUE(db.GetHandle() != nullptr);
	EXPECT_FALSE(db.GetInTransaction());
}
