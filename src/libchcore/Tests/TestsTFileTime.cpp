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
