#include "stdafx.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../TFileFilter.h"
#include "../ECompareType.h"

using namespace chengine;

TEST(TestsTFileFilter, DefaultConstruction)
{
	TFileFilter filter;

	// other
	EXPECT_EQ(0UL, filter.GetObjectID());
	
	EXPECT_EQ(false, filter.GetUseMask());
	EXPECT_STREQ(L"", filter.GetCombinedMask().c_str());

	EXPECT_EQ(false, filter.GetUseExcludeMask());
	EXPECT_STREQ(L"", filter.GetCombinedExcludeMask().c_str());

	EXPECT_EQ(false, filter.GetUseSize1());
	EXPECT_EQ(eCmp_Greater, filter.GetSizeType1());
	EXPECT_EQ(0, filter.GetSize1());

	EXPECT_EQ(false, filter.GetUseSize2());
	EXPECT_EQ(eCmp_Less, filter.GetSizeType2());
	EXPECT_EQ(0, filter.GetSize2());

	// dates
	EXPECT_EQ(eDateType_Created, filter.GetDateType());

	// date 1
	EXPECT_EQ(false, filter.GetUseDateTime1());
	EXPECT_EQ(eCmp_Greater, filter.GetDateCmpType1());
	EXPECT_EQ(false, filter.GetUseDate1());
	EXPECT_EQ(false, filter.GetUseTime1());
//	EXPECT_EQ(TDateTime(), filter.GetDateTime1());

	// date 2
	EXPECT_EQ(false, filter.GetUseDateTime2());
	EXPECT_EQ(eCmp_Less, filter.GetDateCmpType2());
	EXPECT_EQ(false, filter.GetUseDate2());
	EXPECT_EQ(false, filter.GetUseTime2());
//	EXPECT_EQ(TDateTime(), filter.GetDateTime2());

	// attributes
	EXPECT_EQ(false, filter.GetUseAttributes());
	EXPECT_EQ(2, filter.GetArchive());
	EXPECT_EQ(2, filter.GetReadOnly());
	EXPECT_EQ(2, filter.GetHidden());
	EXPECT_EQ(2, filter.GetSystem());
	EXPECT_EQ(2, filter.GetDirectory());

}
