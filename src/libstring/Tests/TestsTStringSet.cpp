#include "stdafx.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../TStringSet.h"

using namespace string;

TEST(TestsTStringSet, DefaultConstruction)
{
	TStringSet setStrings;

	EXPECT_EQ(0UL, setStrings.GetCount());
}

TEST(TestsTStringSet, InsertElements)
{
	TStringSet setStrings;

	setStrings.Insert(TString(L"SomeString1"));
	setStrings.Insert(TString(L"SomeString2"));

	EXPECT_EQ(2UL, setStrings.GetCount());
	EXPECT_TRUE(setStrings.HasValue(L"SomeString1"));
	EXPECT_TRUE(setStrings.HasValue(L"SomeString2"));
	EXPECT_FALSE(setStrings.HasValue(TString()));
}

TEST(TestsTStringSet, InsertCollection)
{
	TStringSet setStrings;

	setStrings.Insert(TString(L"SomeString1"));
	setStrings.Insert(TString(L"SomeString2"));

	TStringSet arrPatterns2;

	arrPatterns2.Insert(TString(L"SomeString3"));

	setStrings.Insert(arrPatterns2);

	EXPECT_EQ(3UL, setStrings.GetCount());
	EXPECT_TRUE(setStrings.HasValue(L"SomeString1"));
	EXPECT_TRUE(setStrings.HasValue(L"SomeString2"));
	EXPECT_TRUE(setStrings.HasValue(L"SomeString3"));
}

TEST(TestsTStringSet, Remove)
{
	TStringSet setStrings;

	setStrings.Insert(TString(L"SomeString1"));
	setStrings.Insert(TString(L"SomeString2"));

	setStrings.Remove(L"SomeString1");

	EXPECT_EQ(1UL, setStrings.GetCount());
	EXPECT_FALSE(setStrings.HasValue(L"SomeString1"));
	EXPECT_TRUE(setStrings.HasValue(L"SomeString2"));
	EXPECT_FALSE(setStrings.HasValue(TString()));
}

TEST(TestsTStringSet, Clear)
{
	TStringSet setStrings;

	setStrings.Insert(TString(L"SomeString1"));
	setStrings.Insert(TString(L"SomeString2"));

	setStrings.Clear();

	EXPECT_EQ(0UL, setStrings.GetCount());
}

TEST(TestsTStringSet, IsEmpty_Empty)
{
	TStringSet setStrings;

	EXPECT_TRUE(setStrings.IsEmpty());
}

TEST(TestsTStringSet, IsEmpty_Full)
{
	TStringSet setStrings;
	setStrings.Insert(TString(L"SomeString1"));

	EXPECT_FALSE(setStrings.IsEmpty());
	setStrings.Clear();
	EXPECT_TRUE(setStrings.IsEmpty());
}

// iterators
TEST(TestsTStringSet, Iterate_Empty)
{
	TStringSet arrStrings;
	const TStringSet arrStrings2;

	EXPECT_EQ(arrStrings.begin(), arrStrings.end());
	EXPECT_EQ(arrStrings.cbegin(), arrStrings.cend());
	EXPECT_EQ(arrStrings2.begin(), arrStrings2.end());
	EXPECT_EQ(arrStrings2.cbegin(), arrStrings2.cend());
}

TEST(TestsTStringSet, Iterate_BeginEnd)
{
	TStringSet arrStrings;
	arrStrings.Insert(TString(L"SomeString1"));
	arrStrings.Insert(TString(L"SomeString2"));

	TStringSet::iterator iter = arrStrings.begin();

	EXPECT_EQ(TString(L"SomeString1"), *iter);
	++iter;
	EXPECT_EQ(TString(L"SomeString2"), *iter);
	++iter;
	EXPECT_EQ(iter, arrStrings.end());
}

TEST(TestsTStringSet, Iterate_CBeginCEnd)
{
	TStringSet arrStrings;
	arrStrings.Insert(TString(L"SomeString1"));
	arrStrings.Insert(TString(L"SomeString2"));

	TStringSet::const_iterator iter = arrStrings.cbegin();

	EXPECT_EQ(TString(L"SomeString1"), *iter);
	++iter;
	EXPECT_EQ(TString(L"SomeString2"), *iter);
	++iter;
	EXPECT_EQ(iter, arrStrings.cend());
}

TEST(TestsTStringSet, Iterate_ConstBeginEnd)
{
	TStringSet arrStrings;
	arrStrings.Insert(TString(L"SomeString1"));
	arrStrings.Insert(TString(L"SomeString2"));

	const TStringSet& rArray = arrStrings;

	TStringSet::const_iterator iter = rArray.begin();

	EXPECT_EQ(TString(L"SomeString1"), *iter);
	++iter;
	EXPECT_EQ(TString(L"SomeString2"), *iter);
	++iter;
	EXPECT_EQ(iter, rArray.end());
}

TEST(TestsTStringSet, NewFor_Empty)
{
	TStringSet arrStrings;

	int iCount = 0;
	for (const TString& strData : arrStrings)
	{
		strData;
		++iCount;
	}

	EXPECT_EQ(0, iCount);
}

TEST(TestsTStringSet, NewFor_Full)
{
	TStringSet arrStrings;
	arrStrings.Insert(TString(L"SomeString1"));
	arrStrings.Insert(TString(L"SomeString2"));

	int iCount = 0;
	for (const TString& strData : arrStrings)
	{
		if (iCount == 0)
			EXPECT_EQ(TString(L"SomeString1"), strData);
		else if (iCount == 1)
			EXPECT_EQ(TString(L"SomeString2"), strData);
		++iCount;
	}

	EXPECT_EQ(2, iCount);
}
