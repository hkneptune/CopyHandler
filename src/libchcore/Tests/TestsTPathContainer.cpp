#include "stdafx.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../TPathContainer.h"

using namespace chcore;
using namespace string;

TEST(TestsTPathContainer, DefaultConstruction)
{
	TPathContainer arrStrings;

	EXPECT_EQ(0, arrStrings.GetCount());
}

TEST(TestsTPathContainer, AddElements)
{
	TPathContainer arrStrings;

	arrStrings.Add(PathFromString(L"SomeString1"));
	arrStrings.Add(PathFromString(L"SomeString2"));

	EXPECT_EQ(2, arrStrings.GetCount());
	EXPECT_STREQ(L"SomeString1", arrStrings.GetAt(0).ToWString().c_str());
	EXPECT_STREQ(L"SomeString2", arrStrings.GetAt(1).ToWString().c_str());
}

TEST(TestsTPathContainer, SetAt)
{
	TPathContainer arrStrings;

	arrStrings.Add(PathFromString(L"SomeString1"));
	arrStrings.Add(PathFromString(L"SomeString2"));

	arrStrings.SetAt(0, PathFromString(L"SomeString3"));

	EXPECT_EQ(2, arrStrings.GetCount());
	EXPECT_STREQ(L"SomeString3", arrStrings.GetAt(0).ToWString().c_str());
	EXPECT_STREQ(L"SomeString2", arrStrings.GetAt(1).ToWString().c_str());
}

TEST(TestsTPathContainer, SetAt_OutOfRange)
{
	TPathContainer arrStrings;

	EXPECT_THROW(arrStrings.SetAt(0, PathFromString(L"SomeString3")), std::out_of_range);
}

TEST(TestsTPathContainer, RemoveAt)
{
	TPathContainer arrStrings;

	arrStrings.Add(PathFromString(L"SomeString1"));
	arrStrings.Add(PathFromString(L"SomeString2"));

	arrStrings.RemoveAt(0);

	EXPECT_EQ(1, arrStrings.GetCount());
	EXPECT_STREQ(L"SomeString2", arrStrings.GetAt(0).ToWString().c_str());
}

TEST(TestsTPathContainer, RemoveAt_OutOfRange)
{
	TPathContainer arrStrings;

	EXPECT_THROW(arrStrings.RemoveAt(0), std::out_of_range);
}

TEST(TestsTPathContainer, Clear)
{
	TPathContainer arrStrings;

	arrStrings.Add(PathFromString(L"SomeString1"));
	arrStrings.Add(PathFromString(L"SomeString2"));

	arrStrings.Clear();

	EXPECT_EQ(0, arrStrings.GetCount());
}

TEST(TestsTPathContainer, Compare_Empty)
{
	TPathContainer arrStrings1, arrStrings2;

	EXPECT_TRUE(arrStrings1 == arrStrings2);
}

TEST(TestsTPathContainer, Compare_Full_Same)
{
	TPathContainer arrStrings1;

	arrStrings1.Add(PathFromString(L"SomeString1"));
	arrStrings1.Add(PathFromString(L"SomeString2"));

	TPathContainer arrStrings2;

	arrStrings2.Add(PathFromString(L"SomeString1"));
	arrStrings2.Add(PathFromString(L"SomeString2"));

	EXPECT_TRUE(arrStrings1 == arrStrings2);
}

TEST(TestsTPathContainer, Compare_Full_NotSame)
{
	TPathContainer arrStrings1;

	arrStrings1.Add(PathFromString(L"SomeString1"));
	arrStrings1.Add(PathFromString(L"SomeString2"));

	TPathContainer arrStrings2;

	arrStrings2.Add(PathFromString(L"SomeString1"));
	arrStrings2.Add(PathFromString(L"SomeString3"));

	EXPECT_FALSE(arrStrings1 == arrStrings2);
}

TEST(TestsTPathContainer, NegativeCompare_Empty)
{
	TPathContainer arrStrings1, arrStrings2;

	EXPECT_FALSE(arrStrings1 != arrStrings2);
}

TEST(TestsTPathContainer, NegativeCompare_Full_Same)
{
	TPathContainer arrStrings1;

	arrStrings1.Add(PathFromString(L"SomeString1"));
	arrStrings1.Add(PathFromString(L"SomeString2"));

	TPathContainer arrStrings2;

	arrStrings2.Add(PathFromString(L"SomeString1"));
	arrStrings2.Add(PathFromString(L"SomeString2"));

	EXPECT_FALSE(arrStrings1 != arrStrings2);
}

TEST(TestsTPathContainer, NegativeCompare_Full_Different)
{
	TPathContainer arrStrings1;

	arrStrings1.Add(PathFromString(L"SomeString1"));
	arrStrings1.Add(PathFromString(L"SomeString2"));

	TPathContainer arrStrings2;

	arrStrings2.Add(PathFromString(L"SomeString1"));
	arrStrings2.Add(PathFromString(L"SomeString3"));

	EXPECT_TRUE(arrStrings1 != arrStrings2);
}

// iterators
TEST(TestsTPathContainer, Iterate_Empty)
{
	TPathContainer arrStrings;
	const TPathContainer arrStrings2;

	EXPECT_EQ(arrStrings.begin(), arrStrings.end());
	EXPECT_EQ(arrStrings.cbegin(), arrStrings.cend());
	EXPECT_EQ(arrStrings2.begin(), arrStrings2.end());
	EXPECT_EQ(arrStrings2.cbegin(), arrStrings2.cend());
}

TEST(TestsTPathContainer, Iterate_BeginEnd)
{
	TPathContainer arrStrings;
	arrStrings.Add(PathFromString(L"SomeString1"));
	arrStrings.Add(PathFromString(L"SomeString2"));

	TPathContainer::iterator iter = arrStrings.begin();

	EXPECT_EQ(PathFromString(L"SomeString1"), *iter);
	++iter;
	EXPECT_EQ(PathFromString(L"SomeString2"), *iter);
	++iter;
	EXPECT_EQ(iter, arrStrings.end());
}

TEST(TestsTPathContainer, Iterate_CBeginCEnd)
{
	TPathContainer arrStrings;
	arrStrings.Add(PathFromString(L"SomeString1"));
	arrStrings.Add(PathFromString(L"SomeString2"));

	TPathContainer::const_iterator iter = arrStrings.cbegin();

	EXPECT_EQ(PathFromString(L"SomeString1"), *iter);
	++iter;
	EXPECT_EQ(PathFromString(L"SomeString2"), *iter);
	++iter;
	EXPECT_EQ(iter, arrStrings.cend());
}

TEST(TestsTPathContainer, Iterate_ConstBeginEnd)
{
	TPathContainer arrStrings;
	arrStrings.Add(PathFromString(L"SomeString1"));
	arrStrings.Add(PathFromString(L"SomeString2"));

	const TPathContainer& rArray = arrStrings;

	TPathContainer::const_iterator iter = rArray.begin();

	EXPECT_EQ(PathFromString(L"SomeString1"), *iter);
	++iter;
	EXPECT_EQ(PathFromString(L"SomeString2"), *iter);
	++iter;
	EXPECT_EQ(iter, rArray.end());
}

TEST(TestsTPathContainer, NewFor_Empty)
{
	TPathContainer arrStrings;

	int iCount = 0;
	for (const TSmartPath& strData : arrStrings)
	{
		strData;
		++iCount;
	}

	EXPECT_EQ(0, iCount);
}

TEST(TestsTPathContainer, NewFor_Full)
{
	TPathContainer arrStrings;
	arrStrings.Add(PathFromString(L"SomeString1"));
	arrStrings.Add(PathFromString(L"SomeString2"));

	int iCount = 0;
	for (const TSmartPath& strData : arrStrings)
	{
		if (iCount == 0)
			EXPECT_EQ(PathFromString(L"SomeString1"), strData);
		else if (iCount == 0)
			EXPECT_EQ(PathFromString(L"SomeString2"), strData);
		++iCount;
	}

	EXPECT_EQ(2, iCount);
}
