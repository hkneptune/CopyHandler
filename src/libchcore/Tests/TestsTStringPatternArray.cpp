#include "stdafx.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../TStringPatternArray.h"
#include "../../libstring/TStringArray.h"

using namespace string;
using namespace chcore;

TEST(TestsTStringPatternArray, DefaultConstruction)
{
	TStringPatternArray arrPatterns;

	EXPECT_EQ(0UL, arrPatterns.GetCount());
}

TEST(TestsTStringPatternArray, AddElements)
{
	TStringPatternArray arrPatterns;

	arrPatterns.Add(TStringPattern(L"*.bat", TStringPattern::EPatternType::eType_FilenameWildcard));
	arrPatterns.Add(TStringPattern(L"*.exe", TStringPattern::EPatternType::eType_FilenameWildcard));

	EXPECT_EQ(2UL, arrPatterns.GetCount());
	EXPECT_STREQ(L"*.bat", arrPatterns.GetAt(0).ToString().c_str());
	EXPECT_STREQ(L"*.exe", arrPatterns.GetAt(1).ToString().c_str());
}

TEST(TestsTStringPatternArray, InsertAt)
{
	TStringPatternArray arrPatterns;

	arrPatterns.InsertAt(0, TStringPattern(L"*.bat", TStringPattern::EPatternType::eType_FilenameWildcard));
	arrPatterns.InsertAt(0, TStringPattern(L"*.exe", TStringPattern::EPatternType::eType_FilenameWildcard));

	EXPECT_EQ(2UL, arrPatterns.GetCount());
	EXPECT_STREQ(L"*.bat", arrPatterns.GetAt(1).ToString().c_str());
	EXPECT_STREQ(L"*.exe", arrPatterns.GetAt(0).ToString().c_str());
}

TEST(TestsTStringPatternArray, SetAt)
{
	TStringPatternArray arrPatterns;

	arrPatterns.Add(TStringPattern(L"*.bat", TStringPattern::EPatternType::eType_FilenameWildcard));
	arrPatterns.Add(TStringPattern(L"*.exe", TStringPattern::EPatternType::eType_FilenameWildcard));

	arrPatterns.SetAt(0, TStringPattern(L"*.com", TStringPattern::EPatternType::eType_FilenameWildcard));

	EXPECT_EQ(2UL, arrPatterns.GetCount());
	EXPECT_STREQ(L"*.com", arrPatterns.GetAt(0).ToString().c_str());
	EXPECT_STREQ(L"*.exe", arrPatterns.GetAt(1).ToString().c_str());
}

TEST(TestsTStringPatternArray, RemoveAt)
{
	TStringPatternArray arrPatterns;

	arrPatterns.Add(TStringPattern(L"*.bat", TStringPattern::EPatternType::eType_FilenameWildcard));
	arrPatterns.Add(TStringPattern(L"*.exe", TStringPattern::EPatternType::eType_FilenameWildcard));

	arrPatterns.RemoveAt(0);

	EXPECT_EQ(1UL, arrPatterns.GetCount());
	EXPECT_STREQ(L"*.exe", arrPatterns.GetAt(0).ToString().c_str());
}

TEST(TestsTStringPatternArray, Clear)
{
	TStringPatternArray arrPatterns;

	arrPatterns.Add(TStringPattern(L"*.bat", TStringPattern::EPatternType::eType_FilenameWildcard));
	arrPatterns.Add(TStringPattern(L"*.exe", TStringPattern::EPatternType::eType_FilenameWildcard));

	arrPatterns.Clear();

	EXPECT_EQ(0UL, arrPatterns.GetCount());
}

/////////////////////////////////////////////////
// matches any
TEST(TestsTStringPatternArray, MatchesAny_Positive)
{
	TStringPatternArray arrPatterns;

	arrPatterns.Add(TStringPattern(L"*.bat", TStringPattern::EPatternType::eType_FilenameWildcard));
	arrPatterns.Add(TStringPattern(L"*.exe", TStringPattern::EPatternType::eType_FilenameWildcard));

	EXPECT_TRUE(arrPatterns.MatchesAny(PathFromString(L"autostart.bat")));
}

TEST(TestsTStringPatternArray, MatchesAny_Negative)
{
	TStringPatternArray arrPatterns;

	arrPatterns.Add(TStringPattern(L"*.bat", TStringPattern::EPatternType::eType_FilenameWildcard));
	arrPatterns.Add(TStringPattern(L"*.exe", TStringPattern::EPatternType::eType_FilenameWildcard));

	EXPECT_FALSE(arrPatterns.MatchesAny(PathFromString(L"autostart.com")));
}

/////////////////////////////////////////////////
// matches all
TEST(TestsTStringPatternArray, MatchesAll_Positive)
{
	TStringPatternArray arrPatterns;

	arrPatterns.Add(TStringPattern(L"*.bat", TStringPattern::EPatternType::eType_FilenameWildcard));
	arrPatterns.Add(TStringPattern(L"autostart.*", TStringPattern::EPatternType::eType_FilenameWildcard));

	EXPECT_TRUE(arrPatterns.MatchesAll(PathFromString(L"autostart.bat")));
}

TEST(TestsTStringPatternArray, MatchesAll_Negative)
{
	TStringPatternArray arrPatterns;

	arrPatterns.Add(TStringPattern(L"*.bat", TStringPattern::EPatternType::eType_FilenameWildcard));
	arrPatterns.Add(TStringPattern(L"autostart.*", TStringPattern::EPatternType::eType_FilenameWildcard));

	EXPECT_FALSE(arrPatterns.MatchesAll(PathFromString(L"autostart.exe")));
}

/////////////////////////////////////////////////
// serialization
TEST(TestsTStringPatternArray, ToStringArray)
{
	TStringPatternArray arrPatterns;

	arrPatterns.Add(TStringPattern(L"*.bat", TStringPattern::EPatternType::eType_FilenameWildcard));
	arrPatterns.Add(TStringPattern(L"autostart.*", TStringPattern::EPatternType::eType_FilenameWildcard));

	TStringArray arrElements = arrPatterns.ToSerializedStringArray();
	EXPECT_EQ(2UL, arrElements.GetCount());
	EXPECT_STREQ(L"*.bat", arrElements.GetAt(0).c_str());
	EXPECT_STREQ(L"autostart.*", arrElements.GetAt(1).c_str());
}

TEST(TestsTStringPatternArray, FromStringArray)
{
	TStringArray arrElements;
	arrElements.Add(L"*.bat");
	arrElements.Add(L"autostart.*");

	TStringPatternArray arrPatterns;
	arrPatterns.FromStringArray(arrElements);

	EXPECT_STREQ(L"*.bat", arrPatterns.GetAt(0).ToString().c_str());
	EXPECT_STREQ(L"autostart.*", arrPatterns.GetAt(1).ToString().c_str());
}
