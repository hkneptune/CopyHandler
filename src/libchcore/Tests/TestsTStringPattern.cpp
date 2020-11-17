#include "stdafx.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../TStringPattern.h"

using namespace chcore;

TEST(TestsTStringPattern, DefaultConstruction)
{
	TStringPattern patternEmpty;
	EXPECT_STREQ(L"", patternEmpty.ToString().c_str());
	EXPECT_STREQ(L"", patternEmpty.GetPattern().c_str());
	EXPECT_EQ(TStringPattern::EPatternType::eType_Wildcard, patternEmpty.GetPatternType());
}

TEST(TestsTStringPattern, PatternConstruction)
{
	TStringPattern patternEmpty(L"*.*", TStringPattern::EPatternType::eType_Wildcard);
	EXPECT_STREQ(L"*.*", patternEmpty.ToString().c_str());
	EXPECT_STREQ(L"*.*", patternEmpty.GetPattern().c_str());
	EXPECT_EQ(TStringPattern::EPatternType::eType_Wildcard, patternEmpty.GetPatternType());
}

TEST(TestsTStringPattern, SetPattern)
{
	TStringPattern patternEmpty(L"*.*", TStringPattern::EPatternType::eType_Wildcard);

	patternEmpty.SetPattern(L"*.bat", TStringPattern::EPatternType::eType_Wildcard);

	EXPECT_STREQ(L"*.bat", patternEmpty.ToString().c_str());
	EXPECT_STREQ(L"*.bat", patternEmpty.GetPattern().c_str());
	EXPECT_EQ(TStringPattern::EPatternType::eType_Wildcard, patternEmpty.GetPatternType());
}

TEST(TestsTStringPattern, CreateFromSerializedString)
{
	TStringPattern patternEmpty = TStringPattern::CreateFromString(L"WC;*.*");

	EXPECT_STREQ(L"*.*", patternEmpty.ToString().c_str());
	EXPECT_STREQ(L"*.*", patternEmpty.GetPattern().c_str());
	EXPECT_EQ(TStringPattern::EPatternType::eType_Wildcard, patternEmpty.GetPatternType());
}

TEST(TestsTStringPattern, FromStringString)
{
	TStringPattern patternEmpty;
	patternEmpty.FromString(L"WC;*.*");

	EXPECT_STREQ(L"*.*", patternEmpty.ToString().c_str());
	EXPECT_STREQ(L"*.*", patternEmpty.GetPattern().c_str());
	EXPECT_EQ(TStringPattern::EPatternType::eType_Wildcard, patternEmpty.GetPatternType());
}

TEST(TestsTStringPattern, Matches_Positive_StarDotBat)
{
	TStringPattern patternEmpty(L"*.bat", TStringPattern::EPatternType::eType_Wildcard);

	EXPECT_TRUE(patternEmpty.Matches(L"autorun.bat"));
}

TEST(TestsTStringPattern, Matches_Negative_StarDotBat)
{
	TStringPattern patternEmpty(L"*.bat", TStringPattern::EPatternType::eType_Wildcard);

	EXPECT_FALSE(patternEmpty.Matches(L"autorun.batx"));
}

TEST(TestsTStringPattern, Matches_Positive_StarDotStar)
{
	TStringPattern patternEmpty(L"*.*", TStringPattern::EPatternType::eType_Wildcard);

	EXPECT_TRUE(patternEmpty.Matches(L"autorun.bat"));
}

TEST(TestsTStringPattern, Matches_Negative_StarDotStar)
{
	TStringPattern patternEmpty(L"*.*", TStringPattern::EPatternType::eType_Wildcard);

	EXPECT_FALSE(patternEmpty.Matches(L"autorun"));
}

///////////////////////////////////////////////////////////
// Multiple asterisks

TEST(TestsTStringPattern, Matches_Positive_MultiStar)
{
	TStringPattern patternEmpty(L"ad*bo*", TStringPattern::EPatternType::eType_Wildcard);

	EXPECT_TRUE(patternEmpty.Matches(L"addon-boo.bat"));
}

TEST(TestsTStringPattern, Matches_Negative_MultiStar)
{
	TStringPattern patternEmpty(L"ad*bo*", TStringPattern::EPatternType::eType_Wildcard);

	EXPECT_FALSE(patternEmpty.Matches(L"addon-doo.bat"));
}

////////////////////////////////////////////////////////////
// asterisks

TEST(TestsTStringPattern, Matches_Positive_QuestionMultiPos)
{
	TStringPattern patternEmpty(L"a??b?r", TStringPattern::EPatternType::eType_Wildcard);

	EXPECT_TRUE(patternEmpty.Matches(L"arbbar"));
}

TEST(TestsTStringPattern, Matches_Negative_QuestionMultiPos)
{
	TStringPattern patternEmpty(L"a??b?r", TStringPattern::EPatternType::eType_Wildcard);

	EXPECT_FALSE(patternEmpty.Matches(L"arbxar"));
}
