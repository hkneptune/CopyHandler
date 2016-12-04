#include "stdafx.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../TOverlappedDataBuffer.h"
#include "../TCoreException.h"

using namespace chcore;

///////////////////////////////////////////////////////////////////////////////
// construction

TEST(TOverlappedDataBufferTests, Constructor_ZeroSizedBuffer)
{
	EXPECT_THROW(TOverlappedDataBuffer(0, nullptr), TCoreException);
}

TEST(TOverlappedDataBufferTests, Constructor_NullParam)
{
	TOverlappedDataBuffer buffer(4096, nullptr);

	EXPECT_EQ(nullptr, buffer.GetParam());
}

TEST(TOverlappedDataBufferTests, Constructor_NonNullParam)
{
	TOverlappedDataBuffer buffer(4096, (void*)5);

	EXPECT_EQ((void*)5, buffer.GetParam());
}

TEST(TOverlappedDataBufferTests, Constructor_SanityTest)
{
	int iParam = 5;
	TOverlappedDataBuffer buffer(32768, &iParam);

	EXPECT_NE(nullptr, buffer.GetBufferPtr());
	EXPECT_EQ(32768, buffer.GetBufferSize());
	EXPECT_EQ(0, buffer.GetBytesTransferred());
	EXPECT_EQ(0, buffer.GetErrorCode());
	EXPECT_EQ(0, buffer.GetFilePosition());
	EXPECT_EQ(0, buffer.GetRealDataSize());
	EXPECT_EQ(0, buffer.GetRequestedDataSize());
	EXPECT_EQ(0, buffer.GetStatusCode());
	EXPECT_FALSE(buffer.IsLastPart());
}

///////////////////////////////////////////////////////////////////////////////
// parameter handling

TEST(TOverlappedDataBufferTests, GetParam)
{
	int iTest = 5;

	TOverlappedDataBuffer buffer(4096, &iTest);

	EXPECT_EQ(&iTest, buffer.GetParam());
}

TEST(TOverlappedDataBufferTests, SetParam_GetParam)
{
	int iTest = 5;

	TOverlappedDataBuffer buffer(4096, &iTest);
	buffer.SetParam(nullptr);

	EXPECT_EQ(nullptr, buffer.GetParam());
}

///////////////////////////////////////////////////////////////////////////////
// buffer re-initialization

TEST(TOverlappedDataBufferTests, ReinitializeBuffer_ZeroSize)
{
	TOverlappedDataBuffer buffer(32768, nullptr);

	EXPECT_THROW(buffer.ReinitializeBuffer(0), TCoreException);
}

TEST(TOverlappedDataBufferTests, ReinitializeBuffer_ReduceSize)
{
	TOverlappedDataBuffer buffer(32768, nullptr);

	buffer.ReinitializeBuffer(16384);

	EXPECT_NE(nullptr, buffer.GetBufferPtr());
	EXPECT_EQ(16384, buffer.GetBufferSize());
}

TEST(TOverlappedDataBufferTests, ReinitializeBuffer_IncreaseSize)
{
	TOverlappedDataBuffer buffer(16384, nullptr);

	buffer.ReinitializeBuffer(32768);

	EXPECT_NE(nullptr, buffer.GetBufferPtr());
	EXPECT_EQ(32768, buffer.GetBufferSize());
}

TEST(TOverlappedDataBufferTests, ReinitializeBuffer_SameSize)
{
	TOverlappedDataBuffer buffer(16384, nullptr);

	void* pBuffer = buffer.GetBufferPtr();

	buffer.ReinitializeBuffer(16384);

	EXPECT_EQ(pBuffer, buffer.GetBufferPtr());
}

///////////////////////////////////////////////////////////////////////////////
// other attributes

TEST(TOverlappedDataBufferTests, SetRequestedDataSize_GetRequestedDataSize)
{
	TOverlappedDataBuffer buffer(16384, nullptr);

	buffer.SetRequestedDataSize(123);

	EXPECT_EQ(123, buffer.GetRequestedDataSize());
}

TEST(TOverlappedDataBufferTests, SetRealDataSize_GetRealDataSize)
{
	TOverlappedDataBuffer buffer(16384, nullptr);

	buffer.SetRealDataSize(123);

	EXPECT_EQ(123, buffer.GetRealDataSize());
}

TEST(TOverlappedDataBufferTests, SetLastPart_IsLastPart)
{
	TOverlappedDataBuffer buffer(16384, nullptr);

	buffer.SetLastPart(true);

	EXPECT_TRUE(buffer.IsLastPart());
}

TEST(TOverlappedDataBufferTests, SetErrorCode_GetErrorCode)
{
	TOverlappedDataBuffer buffer(16384, nullptr);

	buffer.SetErrorCode(123);

	EXPECT_EQ(123, buffer.GetErrorCode());
}

TEST(TOverlappedDataBufferTests, SetStatusCode_GetStatusCode)
{
	TOverlappedDataBuffer buffer(16384, nullptr);

	buffer.SetStatusCode(123);

	EXPECT_EQ(123, buffer.GetStatusCode());
}

TEST(TOverlappedDataBufferTests, SetBytesTransferred_GetBytesTransferred)
{
	TOverlappedDataBuffer buffer(16384, nullptr);

	buffer.SetBytesTransferred(123);

	EXPECT_EQ(123, buffer.GetBytesTransferred());
}

TEST(TOverlappedDataBufferTests, GetFilePosition_SetFilePosition)
{
	TOverlappedDataBuffer buffer(16384, nullptr);

	buffer.SetFilePosition(123);

	EXPECT_EQ(123, buffer.GetFilePosition());
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// 
TEST(TOverlappedDataBufferTests, InitForRead)
{
	TOverlappedDataBuffer buffer(16384, nullptr);

	buffer.SetRequestedDataSize(123);
	buffer.SetFilePosition(1);
	buffer.SetRealDataSize(120);
	buffer.SetLastPart(true);
	buffer.SetErrorCode(54);
	buffer.SetStatusCode(3);
	buffer.SetBytesTransferred(12);

	buffer.InitForRead(320, 600);

	EXPECT_EQ(600, buffer.GetRequestedDataSize());
	EXPECT_EQ(320, buffer.GetFilePosition());
	EXPECT_EQ(0, buffer.GetRealDataSize());
	EXPECT_EQ(false, buffer.IsLastPart());
	EXPECT_EQ(0, buffer.GetErrorCode());
	EXPECT_EQ(0, buffer.GetStatusCode());
	EXPECT_EQ(0, buffer.GetBytesTransferred());
}

TEST(TOverlappedDataBufferTests, InitForWrite)
{
	TOverlappedDataBuffer buffer(16384, nullptr);

	buffer.SetRequestedDataSize(123);
	buffer.SetFilePosition(1);
	buffer.SetRealDataSize(120);
	buffer.SetLastPart(true);
	buffer.SetErrorCode(54);
	buffer.SetStatusCode(3);
	buffer.SetBytesTransferred(12);

	buffer.InitForWrite();

	EXPECT_EQ(0, buffer.GetErrorCode());
	EXPECT_EQ(0, buffer.GetStatusCode());
	EXPECT_EQ(0, buffer.GetBytesTransferred());
}

TEST(TOverlappedDataBufferTests, Reset)
{
	TOverlappedDataBuffer buffer(16384, nullptr);

	buffer.SetRequestedDataSize(123);
	buffer.SetFilePosition(1);
	buffer.SetRealDataSize(120);
	buffer.SetLastPart(true);
	buffer.SetErrorCode(54);
	buffer.SetStatusCode(3);
	buffer.SetBytesTransferred(12);
	buffer.SetParam((void*)1);

	buffer.Reset();

	EXPECT_EQ(0, buffer.GetRequestedDataSize());
	EXPECT_EQ(0, buffer.GetFilePosition());
	EXPECT_EQ(0, buffer.GetRealDataSize());
	EXPECT_EQ(false, buffer.IsLastPart());
	EXPECT_EQ(0, buffer.GetErrorCode());
	EXPECT_EQ(0, buffer.GetStatusCode());
	EXPECT_EQ(0, buffer.GetBytesTransferred());
	EXPECT_EQ(nullptr, buffer.GetParam());
}

///////////////////////////////////////////////////////////////////////////////////////////////////
