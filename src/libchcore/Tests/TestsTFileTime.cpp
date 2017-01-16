#include "stdafx.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../TFileTime.h"

using namespace chcore;

TEST(TestsTFileTime, DefaultConstruction)
{
	TFileTime fTime;
	EXPECT_EQ(0, fTime.ToUInt64());
	EXPECT_EQ(0, fTime.GetAsFiletime().dwLowDateTime);
	EXPECT_EQ(0, fTime.GetAsFiletime().dwHighDateTime);
}

TEST(TestsTFileTime, ConstructionFromFILETIME)
{
	FILETIME filetime = { 0x00000034, 0x00000045 };

	TFileTime fTime(filetime);
	EXPECT_EQ(0x0000004500000034, fTime.ToUInt64());
	EXPECT_EQ(0x00000034, fTime.GetAsFiletime().dwLowDateTime);
	EXPECT_EQ(0x00000045, fTime.GetAsFiletime().dwHighDateTime);
}

TEST(TestsTFileTime, AssignmentOperator)
{
	FILETIME filetime1 = { 0x00000034, 0x00000045 };
	FILETIME filetime2 = { 0x00000074, 0x00000085 };

	TFileTime fTime(filetime1);
	fTime = filetime2;
	EXPECT_EQ(0x0000008500000074, fTime.ToUInt64());
	EXPECT_EQ(0x00000074, fTime.GetAsFiletime().dwLowDateTime);
	EXPECT_EQ(0x00000085, fTime.GetAsFiletime().dwHighDateTime);
}

TEST(TestsTFileTime, CompareOperator_Default)
{
	TFileTime fTime1;
	TFileTime fTime2;
	EXPECT_TRUE(fTime1 == fTime2);
	EXPECT_FALSE(fTime1 != fTime2);
}

TEST(TestsTFileTime, CompareOperator_Same)
{
	FILETIME filetime = { 0x00000074, 0x00000085 };

	TFileTime fTime1(filetime);
	TFileTime fTime2(filetime);
	EXPECT_TRUE(fTime1 == fTime2);
	EXPECT_FALSE(fTime1 != fTime2);
}

TEST(TestsTFileTime, CompareOperator_Different)
{
	FILETIME filetime1 = { 0x00000034, 0x00000045 };
	FILETIME filetime2 = { 0x00000074, 0x00000085 };

	TFileTime fTime1(filetime1);
	TFileTime fTime2(filetime2);
	EXPECT_FALSE(fTime1 == fTime2);
	EXPECT_TRUE(fTime1 != fTime2);
}

TEST(TestsTFileTime, FromToUint64)
{
	FILETIME filetime = { 0x00000034, 0x00000045 };
	unsigned long long ullFiletime = 0x0000004500000034;

	TFileTime fTime;
	fTime.FromUInt64(ullFiletime);
	EXPECT_EQ(fTime.GetAsFiletime().dwLowDateTime, filetime.dwLowDateTime);
	EXPECT_EQ(fTime.GetAsFiletime().dwLowDateTime, filetime.dwLowDateTime);
}
