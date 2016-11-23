#include "stdafx.h"
#include "gtest/gtest.h"
#include "../TSparseRangeMap.h"

using namespace chcore;

TEST(TestsTSparseRangeMap, DefaultConstructor)
{
	TSparseRangeMap map;
	EXPECT_EQ(0, map.GetRangeCount());
	EXPECT_EQ(false, map.OverlapsRange(0, 0));
}

TEST(TestsTSparseRangeMap, SingleRange)
{
	TSparseRangeMap map;
	map.Insert(45, 59);

	file_size_t fsStart = 0;
	file_size_t fsEnd = 0;

	EXPECT_EQ(1, map.GetRangeCount());
	map.GetRangeAt(0, fsStart, fsEnd);
	EXPECT_EQ(45, fsStart);
	EXPECT_EQ(59, fsEnd);

	EXPECT_EQ(false, map.OverlapsRange(44, 44));
	EXPECT_EQ(false, map.OverlapsRange(60, 60));
	EXPECT_EQ(true, map.OverlapsRange(45, 45));
	EXPECT_EQ(true, map.OverlapsRange(59, 59));
}

TEST(TestsTSparseRangeMap, MultipleUnrelatedRanges)
{
	TSparseRangeMap map;
	map.Insert(45, 59);
	map.Insert(63, 85);
	map.Insert(92, 87);

	file_size_t fsStart = 0;
	file_size_t fsEnd = 0;

	EXPECT_EQ(3, map.GetRangeCount());
	map.GetRangeAt(0, fsStart, fsEnd);
	EXPECT_EQ(45, fsStart);
	EXPECT_EQ(59, fsEnd);
	map.GetRangeAt(1, fsStart, fsEnd);
	EXPECT_EQ(63, fsStart);
	EXPECT_EQ(85, fsEnd);
	map.GetRangeAt(2, fsStart, fsEnd);
	EXPECT_EQ(87, fsStart);
	EXPECT_EQ(92, fsEnd);

	EXPECT_EQ(false, map.OverlapsRange(44, 44));
	EXPECT_EQ(false, map.OverlapsRange(60, 60));
	EXPECT_EQ(true, map.OverlapsRange(45, 45));
	EXPECT_EQ(true, map.OverlapsRange(59, 59));

	EXPECT_EQ(false, map.OverlapsRange(62, 62));
	EXPECT_EQ(false, map.OverlapsRange(86, 86));
	EXPECT_EQ(true, map.OverlapsRange(63, 63));
	EXPECT_EQ(true, map.OverlapsRange(85, 85));

	EXPECT_EQ(false, map.OverlapsRange(86, 86));
	EXPECT_EQ(false, map.OverlapsRange(93, 93));
	EXPECT_EQ(true, map.OverlapsRange(87, 87));
	EXPECT_EQ(true, map.OverlapsRange(92, 92));
}

TEST(TestsTSparseRangeMap, MultipleRelatedRanges)
{
	TSparseRangeMap map;
	map.Insert(45, 59);
	map.Insert(60, 85);
	map.Insert(36, 50);

	file_size_t fsStart = 0;
	file_size_t fsEnd = 0;

	EXPECT_EQ(1, map.GetRangeCount());
	map.GetRangeAt(0, fsStart, fsEnd);
	EXPECT_EQ(36, fsStart);
	EXPECT_EQ(85, fsEnd);

	EXPECT_EQ(false, map.OverlapsRange(35, 35));
	EXPECT_EQ(false, map.OverlapsRange(86, 86));
	EXPECT_EQ(true, map.OverlapsRange(36, 36));
	EXPECT_EQ(true, map.OverlapsRange(85, 85));
}
