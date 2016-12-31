#include "stdafx.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../TStringArray.h"
#include "../TStringException.h"

using namespace string;

TEST(TestsTStringArray, DefaultConstruction)
{
	TStringArray arrStrings;

	EXPECT_EQ(0, arrStrings.GetCount());
}

TEST(TestsTStringArray, AddElements)
{
	TStringArray arrStrings;

	arrStrings.Add(TString(L"SomeString1"));
	arrStrings.Add(TString(L"SomeString2"));

	EXPECT_EQ(2, arrStrings.GetCount());
	EXPECT_STREQ(L"SomeString1", arrStrings.GetAt(0).c_str());
	EXPECT_STREQ(L"SomeString2", arrStrings.GetAt(1).c_str());
}

TEST(TestsTStringArray, InsertAt)
{
	TStringArray arrStrings;

	arrStrings.InsertAt(0, TString(L"SomeString1"));
	arrStrings.InsertAt(0, TString(L"SomeString2"));

	EXPECT_EQ(2, arrStrings.GetCount());
	EXPECT_STREQ(L"SomeString1", arrStrings.GetAt(1).c_str());
	EXPECT_STREQ(L"SomeString2", arrStrings.GetAt(0).c_str());
}

TEST(TestsTStringArray, InsertAt_OutOfRange)
{
	TStringArray arrStrings;

	EXPECT_THROW(arrStrings.InsertAt(1, TString(L"SomeString1")), std::out_of_range);
}

TEST(TestsTStringArray, SetAt)
{
	TStringArray arrStrings;

	arrStrings.Add(TString(L"SomeString1"));
	arrStrings.Add(TString(L"SomeString2"));

	arrStrings.SetAt(0, TString(L"SomeString3"));

	EXPECT_EQ(2, arrStrings.GetCount());
	EXPECT_STREQ(L"SomeString3", arrStrings.GetAt(0).c_str());
	EXPECT_STREQ(L"SomeString2", arrStrings.GetAt(1).c_str());
}

TEST(TestsTStringArray, SetAt_OutOfRange)
{
	TStringArray arrStrings;

	EXPECT_THROW(arrStrings.SetAt(0, TString(L"SomeString3")), std::out_of_range);
}

TEST(TestsTStringArray, RemoveAt)
{
	TStringArray arrStrings;

	arrStrings.Add(TString(L"SomeString1"));
	arrStrings.Add(TString(L"SomeString2"));

	arrStrings.RemoveAt(0);

	EXPECT_EQ(1, arrStrings.GetCount());
	EXPECT_STREQ(L"SomeString2", arrStrings.GetAt(0).c_str());
}

TEST(TestsTStringArray, RemoveAt_OutOfRange)
{
	TStringArray arrStrings;

	EXPECT_THROW(arrStrings.RemoveAt(0), std::out_of_range);
}

TEST(TestsTStringArray, Clear)
{
	TStringArray arrStrings;

	arrStrings.Add(TString(L"SomeString1"));
	arrStrings.Add(TString(L"SomeString2"));

	arrStrings.Clear();

	EXPECT_EQ(0, arrStrings.GetCount());
}

TEST(TestsTStringArray, Compare_Empty)
{
	TStringArray arrStrings1, arrStrings2;

	EXPECT_TRUE(arrStrings1 == arrStrings2);
}

TEST(TestsTStringArray, Compare_Full_Same)
{
	TStringArray arrStrings1;

	arrStrings1.Add(TString(L"SomeString1"));
	arrStrings1.Add(TString(L"SomeString2"));

	TStringArray arrStrings2;

	arrStrings2.Add(TString(L"SomeString1"));
	arrStrings2.Add(TString(L"SomeString2"));

	EXPECT_TRUE(arrStrings1 == arrStrings2);
}

TEST(TestsTStringArray, Compare_Full_NotSame)
{
	TStringArray arrStrings1;

	arrStrings1.Add(TString(L"SomeString1"));
	arrStrings1.Add(TString(L"SomeString2"));

	TStringArray arrStrings2;

	arrStrings2.Add(TString(L"SomeString1"));
	arrStrings2.Add(TString(L"SomeString3"));

	EXPECT_FALSE(arrStrings1 == arrStrings2);
}

TEST(TestsTStringArray, NegativeCompare_Empty)
{
	TStringArray arrStrings1, arrStrings2;

	EXPECT_FALSE(arrStrings1 != arrStrings2);
}

TEST(TestsTStringArray, NegativeCompare_Full_Same)
{
	TStringArray arrStrings1;

	arrStrings1.Add(TString(L"SomeString1"));
	arrStrings1.Add(TString(L"SomeString2"));

	TStringArray arrStrings2;

	arrStrings2.Add(TString(L"SomeString1"));
	arrStrings2.Add(TString(L"SomeString2"));

	EXPECT_FALSE(arrStrings1 != arrStrings2);
}

TEST(TestsTStringArray, NegativeCompare_Full_Different)
{
	TStringArray arrStrings1;

	arrStrings1.Add(TString(L"SomeString1"));
	arrStrings1.Add(TString(L"SomeString2"));

	TStringArray arrStrings2;

	arrStrings2.Add(TString(L"SomeString1"));
	arrStrings2.Add(TString(L"SomeString3"));

	EXPECT_TRUE(arrStrings1 != arrStrings2);
}

// iterators
TEST(TestsTStringArray, Iterate_Empty)
{
	TStringArray arrStrings;
	const TStringArray arrStrings2;

	EXPECT_EQ(arrStrings.begin(), arrStrings.end());
	EXPECT_EQ(arrStrings.cbegin(), arrStrings.cend());
	EXPECT_EQ(arrStrings2.begin(), arrStrings2.end());
	EXPECT_EQ(arrStrings2.cbegin(), arrStrings2.cend());
}

TEST(TestsTStringArray, Iterate_BeginEnd)
{
	TStringArray arrStrings;
	arrStrings.Add(TString(L"SomeString1"));
	arrStrings.Add(TString(L"SomeString2"));

	TStringArray::iterator iter = arrStrings.begin();

	EXPECT_EQ(TString(L"SomeString1"), *iter);
	++iter;
	EXPECT_EQ(TString(L"SomeString2"), *iter);
	++iter;
	EXPECT_EQ(iter, arrStrings.end());
}

TEST(TestsTStringArray, Iterate_CBeginCEnd)
{
	TStringArray arrStrings;
	arrStrings.Add(TString(L"SomeString1"));
	arrStrings.Add(TString(L"SomeString2"));

	TStringArray::const_iterator iter = arrStrings.cbegin();

	EXPECT_EQ(TString(L"SomeString1"), *iter);
	++iter;
	EXPECT_EQ(TString(L"SomeString2"), *iter);
	++iter;
	EXPECT_EQ(iter, arrStrings.cend());
}

TEST(TestsTStringArray, Iterate_ConstBeginEnd)
{
	TStringArray arrStrings;
	arrStrings.Add(TString(L"SomeString1"));
	arrStrings.Add(TString(L"SomeString2"));

	const TStringArray& rArray = arrStrings;

	TStringArray::const_iterator iter = rArray.begin();

	EXPECT_EQ(TString(L"SomeString1"), *iter);
	++iter;
	EXPECT_EQ(TString(L"SomeString2"), *iter);
	++iter;
	EXPECT_EQ(iter, rArray.end());
}

TEST(TestsTStringArray, NewFor_Empty)
{
	TStringArray arrStrings;

	int iCount = 0;
	for (const TString& strData : arrStrings)
	{
		strData;
		++iCount;
	}

	EXPECT_EQ(0, iCount);
}

TEST(TestsTStringArray, NewFor_Full)
{
	TStringArray arrStrings;
	arrStrings.Add(TString(L"SomeString1"));
	arrStrings.Add(TString(L"SomeString2"));

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
