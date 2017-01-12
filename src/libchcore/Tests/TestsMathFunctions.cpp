#include "stdafx.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../MathFunctions.h"

using namespace chcore;

TEST(TestsMathFunctions, Div64_64)
{
	EXPECT_EQ(0, Math::Div64(0ULL, 1ULL));
	EXPECT_EQ(1, Math::Div64(1ULL, 1ULL));
	EXPECT_EQ(0, Math::Div64(0ULL, 0xFFFFFFFFFFFFFFFF));
	EXPECT_EQ(1, Math::Div64(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF));
}

TEST(TestsMathFunctions, Div64_64_By0)
{
	EXPECT_EQ(0, Math::Div64(0ULL, 0ULL));
	EXPECT_EQ(0, Math::Div64(1ULL, 0ULL));
	EXPECT_EQ(0, Math::Div64(0ULL, 0ULL));
	EXPECT_EQ(0, Math::Div64(0xFFFFFFFFFFFFFFFF, 0ULL));
}

TEST(TestsMathFunctions, Div64_Double)
{
	EXPECT_EQ(0, Math::Div64(0ULL, 1.0));
	EXPECT_EQ(1, Math::Div64(1ULL, 1.0));
}

TEST(TestsMathFunctions, Div64_Double_By0)
{
	EXPECT_EQ(0, Math::Div64(0ULL, 0.0));
	EXPECT_EQ(0, Math::Div64(1ULL, 0.0));
	EXPECT_EQ(0, Math::Div64(0ULL, 0.0));
	EXPECT_EQ(0, Math::Div64(0xFFFFFFFFFFFFFFFF, 0.0));
}
