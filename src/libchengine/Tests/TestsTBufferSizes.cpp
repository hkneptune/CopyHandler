#include "stdafx.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../TBufferSizes.h"
#include "../../libchcore/TCoreException.h"

using namespace chengine;

TEST(TestsTBufferSizes, DefaultConstructor)
{
	TBufferSizes tSizes;

	EXPECT_EQ(TBufferSizes::MinBufferCount, tSizes.GetBufferCount());
	EXPECT_EQ(TBufferSizes::BufferGranularity, tSizes.GetDefaultSize());
	EXPECT_EQ(TBufferSizes::BufferGranularity, tSizes.GetOneDiskSize());
	EXPECT_EQ(TBufferSizes::BufferGranularity, tSizes.GetTwoDisksSize());
	EXPECT_EQ(TBufferSizes::BufferGranularity, tSizes.GetCDSize());
	EXPECT_EQ(TBufferSizes::BufferGranularity, tSizes.GetLANSize());
	EXPECT_EQ(false, tSizes.IsOnlyDefault());
}

TEST(TestsTBufferSizes, ParametrizedConstructor_RoundedValues)
{
	TBufferSizes tSizes(true, 2, 4096, 8192, 16384, 32768, 65536, 10, 2, 1);

	EXPECT_EQ(2, tSizes.GetBufferCount());
	EXPECT_EQ(4096, tSizes.GetDefaultSize());
	EXPECT_EQ(8192, tSizes.GetOneDiskSize());
	EXPECT_EQ(16384, tSizes.GetTwoDisksSize());
	EXPECT_EQ(32768, tSizes.GetCDSize());
	EXPECT_EQ(65536, tSizes.GetLANSize());
	EXPECT_EQ(true, tSizes.IsOnlyDefault());
	EXPECT_EQ(10, tSizes.GetMaxReadAheadBuffers());
	EXPECT_EQ(2, tSizes.GetMaxConcurrentReads());
	EXPECT_EQ(1, tSizes.GetMaxConcurrentWrites());
}

TEST(TestsTBufferSizes, ParametrizedConstructor_MinimumCheck)
{
	TBufferSizes tSizes(true, 0, 0, 0, 0, 0, 0, 0, 0, 0);

	EXPECT_EQ(TBufferSizes::MinBufferCount, tSizes.GetBufferCount());
	EXPECT_EQ(TBufferSizes::BufferGranularity, tSizes.GetDefaultSize());
	EXPECT_EQ(TBufferSizes::BufferGranularity, tSizes.GetOneDiskSize());
	EXPECT_EQ(TBufferSizes::BufferGranularity, tSizes.GetTwoDisksSize());
	EXPECT_EQ(TBufferSizes::BufferGranularity, tSizes.GetCDSize());
	EXPECT_EQ(TBufferSizes::BufferGranularity, tSizes.GetLANSize());
	EXPECT_EQ(true, tSizes.IsOnlyDefault());
	EXPECT_EQ(1, tSizes.GetMaxReadAheadBuffers());
	EXPECT_EQ(1, tSizes.GetMaxConcurrentReads());
	EXPECT_EQ(1, tSizes.GetMaxConcurrentWrites());
}

TEST(TestsTBufferSizes, ParametrizedConstructor_RoundingCheck)
{
	TBufferSizes tSizes(true, 2, 6543, 9891, 17123, 37012, 72089, 10, 2, 1);

	EXPECT_EQ(2, tSizes.GetBufferCount());
	EXPECT_EQ(8192, tSizes.GetDefaultSize());
	EXPECT_EQ(12288, tSizes.GetOneDiskSize());
	EXPECT_EQ(20480, tSizes.GetTwoDisksSize());
	EXPECT_EQ(40960, tSizes.GetCDSize());
	EXPECT_EQ(73728, tSizes.GetLANSize());
	EXPECT_EQ(true, tSizes.IsOnlyDefault());
	EXPECT_EQ(10, tSizes.GetMaxReadAheadBuffers());
	EXPECT_EQ(2, tSizes.GetMaxConcurrentReads());
	EXPECT_EQ(1, tSizes.GetMaxConcurrentWrites());
}

TEST(TestsTBufferSizes, Clear)
{
	TBufferSizes tSizes(true, 2, 6543, 9891, 17123, 37012, 72089, 10, 2, 1);

	tSizes.Clear();

	EXPECT_EQ(TBufferSizes::MinBufferCount, tSizes.GetBufferCount());
	EXPECT_EQ(TBufferSizes::BufferGranularity, tSizes.GetDefaultSize());
	EXPECT_EQ(TBufferSizes::BufferGranularity, tSizes.GetOneDiskSize());
	EXPECT_EQ(TBufferSizes::BufferGranularity, tSizes.GetTwoDisksSize());
	EXPECT_EQ(TBufferSizes::BufferGranularity, tSizes.GetCDSize());
	EXPECT_EQ(TBufferSizes::BufferGranularity, tSizes.GetLANSize());
	EXPECT_EQ(false, tSizes.IsOnlyDefault());
	EXPECT_EQ(1, tSizes.GetMaxReadAheadBuffers());
	EXPECT_EQ(1, tSizes.GetMaxConcurrentReads());
	EXPECT_EQ(1, tSizes.GetMaxConcurrentWrites());
}

TEST(TestsTBufferSizes, SetOnlyDefault_IsOnlyDefault)
{
	TBufferSizes tSizes;

	tSizes.SetOnlyDefault(true);

	EXPECT_EQ(true, tSizes.IsOnlyDefault());
}

////////////////////////////////////////////////////////////////////////
TEST(TestsTBufferSizes, SetDefaultSize_GetDefaultSize_Rounded)
{
	TBufferSizes tSizes;

	tSizes.SetDefaultSize(8192);

	EXPECT_EQ(8192, tSizes.GetDefaultSize());
}

TEST(TestsTBufferSizes, SetDefaultSize_GetDefaultSize_MinCheck)
{
	TBufferSizes tSizes;

	tSizes.SetDefaultSize(1);

	EXPECT_EQ(4096, tSizes.GetDefaultSize());
}

TEST(TestsTBufferSizes, SetDefaultSize_GetDefaultSize_RoundCheck)
{
	TBufferSizes tSizes;

	tSizes.SetDefaultSize(8193);

	EXPECT_EQ(12288, tSizes.GetDefaultSize());
}

/////////////////////////////////////////////////////////////////////////
TEST(TestsTBufferSizes, SetOneDiskSize_GetOneDiskSize_Rounded)
{
	TBufferSizes tSizes;

	tSizes.SetOneDiskSize(8192);

	EXPECT_EQ(8192, tSizes.GetOneDiskSize());
}

TEST(TestsTBufferSizes, SetOneDiskSize_GetOneDiskSize_MinCheck)
{
	TBufferSizes tSizes;

	tSizes.SetOneDiskSize(1);

	EXPECT_EQ(4096, tSizes.GetOneDiskSize());
}

TEST(TestsTBufferSizes, SetOneDiskSize_GetOneDiskSize_RoundCheck)
{
	TBufferSizes tSizes;

	tSizes.SetOneDiskSize(8193);

	EXPECT_EQ(12288, tSizes.GetOneDiskSize());
}

////////////////////////////////////////////////////////////////////////
TEST(TestsTBufferSizes, SetTwoDisksSize_GetTwoDisksSize_Rounded)
{
	TBufferSizes tSizes;

	tSizes.SetTwoDisksSize(8192);

	EXPECT_EQ(8192, tSizes.GetTwoDisksSize());
}

TEST(TestsTBufferSizes, SetTwoDisksSize_GetTwoDisksSize_MinCheck)
{
	TBufferSizes tSizes;

	tSizes.SetTwoDisksSize(1);

	EXPECT_EQ(4096, tSizes.GetTwoDisksSize());
}

TEST(TestsTBufferSizes, SetTwoDisksSize_GetTwoDisksSize_RoundCheck)
{
	TBufferSizes tSizes;

	tSizes.SetTwoDisksSize(8193);

	EXPECT_EQ(12288, tSizes.GetTwoDisksSize());
}
////////////////////////////////////////////////////////////////////////
TEST(TestsTBufferSizes, SetCDSize_GetCDSize_Rounded)
{
	TBufferSizes tSizes;

	tSizes.SetCDSize(8192);

	EXPECT_EQ(8192, tSizes.GetCDSize());
}

TEST(TestsTBufferSizes, SetCDSize_GetCDSize_MinCheck)
{
	TBufferSizes tSizes;

	tSizes.SetCDSize(1);

	EXPECT_EQ(4096, tSizes.GetCDSize());
}

TEST(TestsTBufferSizes, SetCDSize_GetCDSize_RoundCheck)
{
	TBufferSizes tSizes;

	tSizes.SetCDSize(8193);

	EXPECT_EQ(12288, tSizes.GetCDSize());
}

////////////////////////////////////////////////////////////////////////
TEST(TestsTBufferSizes, SetLANSize_GetLANSize_Rounded)
{
	TBufferSizes tSizes;

	tSizes.SetLANSize(8192);

	EXPECT_EQ(8192, tSizes.GetLANSize());
}

TEST(TestsTBufferSizes, SetLANSize_GetLANSize_MinCheck)
{
	TBufferSizes tSizes;

	tSizes.SetLANSize(1);

	EXPECT_EQ(4096, tSizes.GetLANSize());
}

TEST(TestsTBufferSizes, SetLANSize_GetLANSize_RoundCheck)
{
	TBufferSizes tSizes;

	tSizes.SetLANSize(8193);

	EXPECT_EQ(12288, tSizes.GetLANSize());
}

//////////////////////////////////////////////////////////////////////
TEST(TestsTBufferSizes, SetBufferCount_GetBufferCount)
{
	TBufferSizes tSizes;

	tSizes.SetBufferCount(5);

	EXPECT_EQ(5, tSizes.GetBufferCount());
}

TEST(TestsTBufferSizes, SetBufferCount_GetBufferCount_MinSize)
{
	TBufferSizes tSizes;

	tSizes.SetBufferCount(0);

	EXPECT_EQ(1, tSizes.GetBufferCount());
}

////////////////////////////////////////////////////////////////////
TEST(TestsTBufferSizes, SetSizeByType_GetSizeByType_RoundedSize)
{
	TBufferSizes tSizes;

	tSizes.SetSizeByType(TBufferSizes::eBuffer_Default, 8192);
	tSizes.SetSizeByType(TBufferSizes::eBuffer_OneDisk, 16384);
	tSizes.SetSizeByType(TBufferSizes::eBuffer_TwoDisks, 32768);
	tSizes.SetSizeByType(TBufferSizes::eBuffer_CD, 65536);
	tSizes.SetSizeByType(TBufferSizes::eBuffer_LAN, 131072);

	EXPECT_EQ(8192, tSizes.GetSizeByType(TBufferSizes::eBuffer_Default));
	EXPECT_EQ(16384, tSizes.GetSizeByType(TBufferSizes::eBuffer_OneDisk));
	EXPECT_EQ(32768, tSizes.GetSizeByType(TBufferSizes::eBuffer_TwoDisks));
	EXPECT_EQ(65536, tSizes.GetSizeByType(TBufferSizes::eBuffer_CD));
	EXPECT_EQ(131072, tSizes.GetSizeByType(TBufferSizes::eBuffer_LAN));
}

TEST(TestsTBufferSizes, SetSizeByType_GetSizeByType_RoundCheck)
{
	TBufferSizes tSizes;

	tSizes.SetSizeByType(TBufferSizes::eBuffer_Default, 8190);
	tSizes.SetSizeByType(TBufferSizes::eBuffer_OneDisk, 16380);
	tSizes.SetSizeByType(TBufferSizes::eBuffer_TwoDisks, 32760);
	tSizes.SetSizeByType(TBufferSizes::eBuffer_CD, 65530);
	tSizes.SetSizeByType(TBufferSizes::eBuffer_LAN, 131070);

	EXPECT_EQ(8192, tSizes.GetSizeByType(TBufferSizes::eBuffer_Default));
	EXPECT_EQ(16384, tSizes.GetSizeByType(TBufferSizes::eBuffer_OneDisk));
	EXPECT_EQ(32768, tSizes.GetSizeByType(TBufferSizes::eBuffer_TwoDisks));
	EXPECT_EQ(65536, tSizes.GetSizeByType(TBufferSizes::eBuffer_CD));
	EXPECT_EQ(131072, tSizes.GetSizeByType(TBufferSizes::eBuffer_LAN));
}

TEST(TestsTBufferSizes, SetSizeByType_GetSizeByType_MinSize)
{
	TBufferSizes tSizes;

	tSizes.SetSizeByType(TBufferSizes::eBuffer_Default, 0);
	tSizes.SetSizeByType(TBufferSizes::eBuffer_OneDisk, 0);
	tSizes.SetSizeByType(TBufferSizes::eBuffer_TwoDisks, 0);
	tSizes.SetSizeByType(TBufferSizes::eBuffer_CD, 0);
	tSizes.SetSizeByType(TBufferSizes::eBuffer_LAN, 0);

	EXPECT_EQ(4096, tSizes.GetSizeByType(TBufferSizes::eBuffer_Default));
	EXPECT_EQ(4096, tSizes.GetSizeByType(TBufferSizes::eBuffer_OneDisk));
	EXPECT_EQ(4096, tSizes.GetSizeByType(TBufferSizes::eBuffer_TwoDisks));
	EXPECT_EQ(4096, tSizes.GetSizeByType(TBufferSizes::eBuffer_CD));
	EXPECT_EQ(4096, tSizes.GetSizeByType(TBufferSizes::eBuffer_LAN));
}

TEST(TestsTBufferSizes, SetSizeByType_GetSizeByType_OutOfRange)
{
	TBufferSizes tSizes;

	EXPECT_THROW(tSizes.SetSizeByType(TBufferSizes::eBuffer_Last, 0), chcore::TCoreException);
	EXPECT_THROW(tSizes.GetSizeByType(TBufferSizes::eBuffer_Last), chcore::TCoreException);
}

////////////////////////////////////////////////////////////////////////////////////////////////
TEST(TestsTBufferSizes, GetMaxSize_Default)
{
	TBufferSizes tSizes(false, 1, 16384, 0, 0, 0, 0, 0, 0, 0);

	EXPECT_EQ(16384, tSizes.GetMaxSize());
}

TEST(TestsTBufferSizes, GetMaxSize_OneDisk)
{
	TBufferSizes tSizes(false, 1, 0, 16384, 0, 0, 0, 0, 0, 0);

	EXPECT_EQ(16384, tSizes.GetMaxSize());
}

TEST(TestsTBufferSizes, GetMaxSize_TwoDisks)
{
	TBufferSizes tSizes(false, 1, 0, 0, 16384, 0, 0, 0, 0, 0);

	EXPECT_EQ(16384, tSizes.GetMaxSize());
}

TEST(TestsTBufferSizes, GetMaxSize_CD)
{
	TBufferSizes tSizes(false, 1, 0, 0, 0, 16384, 0, 0, 0, 0);

	EXPECT_EQ(16384, tSizes.GetMaxSize());
}

TEST(TestsTBufferSizes, GetMaxSize_LAN)
{
	TBufferSizes tSizes(false, 1, 0, 0, 0, 0, 16384, 0, 0, 0);

	EXPECT_EQ(16384, tSizes.GetMaxSize());
}

TEST(TestsTBufferSizes, GetMaxSize_OnlyDefault)
{
	TBufferSizes tSizes(true, 1, 16384, 0, 0, 0, 32768, 0, 0, 0);

	EXPECT_EQ(16384, tSizes.GetMaxSize());
}
