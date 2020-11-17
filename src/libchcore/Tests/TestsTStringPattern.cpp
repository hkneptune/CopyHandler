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
	EXPECT_EQ(TStringPattern::EPatternType::eType_FilenameWildcard, patternEmpty.GetPatternType());
}

TEST(TestsTStringPattern, PatternConstruction)
{
	TStringPattern patternEmpty(L"*.*", TStringPattern::EPatternType::eType_FilenameWildcard);
	EXPECT_STREQ(L"*.*", patternEmpty.ToString().c_str());
	EXPECT_STREQ(L"*.*", patternEmpty.GetPattern().c_str());
	EXPECT_EQ(TStringPattern::EPatternType::eType_FilenameWildcard, patternEmpty.GetPatternType());
}

TEST(TestsTStringPattern, SetPattern)
{
	TStringPattern patternEmpty(L"*.*", TStringPattern::EPatternType::eType_FilenameWildcard);

	patternEmpty.SetPattern(L"*.bat", TStringPattern::EPatternType::eType_FilenameWildcard);

	EXPECT_STREQ(L"*.bat", patternEmpty.ToString().c_str());
	EXPECT_STREQ(L"*.bat", patternEmpty.GetPattern().c_str());
	EXPECT_EQ(TStringPattern::EPatternType::eType_FilenameWildcard, patternEmpty.GetPatternType());
}

TEST(TestsTStringPattern, CreateFromSerializedString)
{
	TStringPattern patternEmpty = TStringPattern::CreateFromString(L"file:*.*");

	EXPECT_STREQ(L"*.*", patternEmpty.ToString().c_str());
	EXPECT_STREQ(L"*.*", patternEmpty.GetPattern().c_str());
	EXPECT_EQ(TStringPattern::EPatternType::eType_FilenameWildcard, patternEmpty.GetPatternType());
}

TEST(TestsTStringPattern, FromStringString)
{
	TStringPattern patternEmpty;
	patternEmpty.FromString(L"file:*.*");

	EXPECT_EQ(TStringPattern::EPatternType::eType_FilenameWildcard, patternEmpty.GetPatternType());
	EXPECT_STREQ(L"*.*", patternEmpty.GetPattern().c_str());
	EXPECT_STREQ(L"*.*", patternEmpty.ToString().c_str());
}

TEST(TestsTStringPattern, Matches_DoubleWildcard)
{
	TStringPattern patternEmpty(L"*.*", TStringPattern::EPatternType::eType_FilenameWildcard);

	EXPECT_TRUE(patternEmpty.Matches(PathFromString(L"autorun")));
	EXPECT_TRUE(patternEmpty.Matches(PathFromString(L"autorun.txt")));
}

TEST(TestsTStringPattern, Matches_SingleWildcard)
{
	TStringPattern patternEmpty(L"*", TStringPattern::EPatternType::eType_FilenameWildcard);

	EXPECT_TRUE(patternEmpty.Matches(PathFromString(L"autorun")));
	EXPECT_TRUE(patternEmpty.Matches(PathFromString(L"autorun.txt")));
}

TEST(TestsTStringPattern, Matches_Positive_StarDotBat)
{
	TStringPattern patternEmpty(L"*.bat", TStringPattern::EPatternType::eType_FilenameWildcard);

	EXPECT_TRUE(patternEmpty.Matches(PathFromString(L"autorun.bat")));
}

TEST(TestsTStringPattern, Matches_Negative_StarDotBat)
{
	TStringPattern patternEmpty(L"*.bat", TStringPattern::EPatternType::eType_FilenameWildcard);

	EXPECT_FALSE(patternEmpty.Matches(PathFromString(L"autorun.batx")));
}

TEST(TestsTStringPattern, Matches_Positive_StarDotStar)
{
	TStringPattern patternEmpty(L"*.*", TStringPattern::EPatternType::eType_FilenameWildcard);

	EXPECT_TRUE(patternEmpty.Matches(PathFromString(L"autorun.bat")));
}

///////////////////////////////////////////////////////////
// Multiple asterisks

TEST(TestsTStringPattern, Matches_Positive_MultiStar)
{
	TStringPattern patternEmpty(L"ad*bo*", TStringPattern::EPatternType::eType_FilenameWildcard);

	EXPECT_TRUE(patternEmpty.Matches(PathFromString(L"addon-boo.bat")));
}

TEST(TestsTStringPattern, Matches_Negative_MultiStar)
{
	TStringPattern patternEmpty(L"ad*bo*", TStringPattern::EPatternType::eType_FilenameWildcard);

	EXPECT_FALSE(patternEmpty.Matches(PathFromString(L"addon-doo.bat")));
}

////////////////////////////////////////////////////////////
// asterisks
TEST(TestsTStringPattern, Matches_Positive_QuestionMultiPos)
{
	TStringPattern patternEmpty(L"a??b?r", TStringPattern::EPatternType::eType_FilenameWildcard);

	EXPECT_TRUE(patternEmpty.Matches(PathFromString(L"arbbar")));
}

TEST(TestsTStringPattern, Matches_Negative_QuestionMultiPos)
{
	TStringPattern patternEmpty(L"a??b?r", TStringPattern::EPatternType::eType_FilenameWildcard);

	EXPECT_FALSE(patternEmpty.Matches(PathFromString(L"arbxar")));
}

// full path matching
TEST(TestsTStringPattern, FullPath_MatchFilename)
{
	TStringPattern patternEmpty(L"a??b?r", TStringPattern::EPatternType::eType_FilenameWildcard);

	EXPECT_TRUE(patternEmpty.Matches(PathFromString(LR"(c:\windows\arbbar)")));
}

TEST(TestsTStringPattern, FullPath_NotMatchWholePath)
{
	TStringPattern patternEmpty(L"a??b?r", TStringPattern::EPatternType::eType_FullPathWildcard);

	EXPECT_FALSE(patternEmpty.Matches(PathFromString(LR"(c:\windows\arbbar)")));
}

TEST(TestsTStringPattern, FullPath_MatchWholePath)
{
	TStringPattern patternEmpty(L"*a??b?r", TStringPattern::EPatternType::eType_FullPathWildcard);

	EXPECT_TRUE(patternEmpty.Matches(PathFromString(LR"(c:\windows\arbbar)")));
}

TEST(TestsTStringPattern, FullPath_MatchComplexPattern)
{
	TStringPattern patternEmpty(LR"(c:\*\a??b?r)", TStringPattern::EPatternType::eType_FullPathWildcard);

	EXPECT_TRUE(patternEmpty.Matches(PathFromString(LR"(c:\windows\arbbar)")));
}

// auto-detection
TEST(TestsTStringPattern, FullPath_DetectWildcard)
{
	TStringPattern patternEmpty(L"file:a??b?r");

	EXPECT_TRUE(patternEmpty.Matches(PathFromString(LR"(c:\windows\arbbar)")));
}

TEST(TestsTStringPattern, FullPath_DetectFullPathWildcard)
{
	TStringPattern patternEmpty(L"path:a??b?r");

	EXPECT_FALSE(patternEmpty.Matches(PathFromString(LR"(c:\windows\arbbar)")));
}

///////////////////////////////////////////////
// regex

// auto-detection
TEST(TestsTStringPattern, RegexFullPath_DetectWildcard)
{
	TStringPattern patternEmpty(L"rfile:ar[bar]+");

	EXPECT_TRUE(patternEmpty.Matches(PathFromString(LR"(c:\windows\arbbar)")));
}

TEST(TestsTStringPattern, RegexFullPath_DetectFullPathWildcard)
{
	TStringPattern patternEmpty(L"rpath:c:\\\\[a-z]+\\\\a[bar]*");

	EXPECT_TRUE(patternEmpty.Matches(PathFromString(LR"(c:\windows\arbbar)")));
}
