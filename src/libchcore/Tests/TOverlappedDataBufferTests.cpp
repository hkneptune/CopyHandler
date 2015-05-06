#include "stdafx.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../TOverlappedDataBuffer.h"
#include "../TCoreException.h"
#include "../TOverlappedDataBufferQueue.h"

using namespace chcore;

TEST(TOverlappedDataBufferTests, Constructor_InvalidInput)
{
	EXPECT_THROW(TOverlappedDataBuffer(0, nullptr), TCoreException);
}

TEST(TOverlappedDataBufferTests, Constructor_SanityTest)
{
	TOverlappedDataBufferQueue queue;
	TOverlappedDataBuffer buffer(32768, &queue);

	EXPECT_EQ(0, buffer.GetBufferOrder());
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

TEST(TOverlappedDataBufferTests, ReinitializeBuffer_ReduceSize)
{
	TOverlappedDataBufferQueue queue;
	TOverlappedDataBuffer buffer(32768, &queue);

	buffer.ReinitializeBuffer(16384);

	EXPECT_NE(nullptr, buffer.GetBufferPtr());
	EXPECT_EQ(16384, buffer.GetBufferSize());
}

TEST(TOverlappedDataBufferTests, ReinitializeBuffer_IncreaseSize)
{
	TOverlappedDataBufferQueue queue;
	TOverlappedDataBuffer buffer(16384, &queue);

	buffer.ReinitializeBuffer(32768);

	EXPECT_NE(nullptr, buffer.GetBufferPtr());
	EXPECT_EQ(32768, buffer.GetBufferSize());
}

TEST(TOverlappedDataBufferTests, SetRequestedDataSize_GetRequestedDataSize)
{
	TOverlappedDataBufferQueue queue;
	TOverlappedDataBuffer buffer(16384, &queue);

	buffer.SetRequestedDataSize(123);

	EXPECT_EQ(123, buffer.GetRequestedDataSize());
}

TEST(TOverlappedDataBufferTests, SetRealDataSize_GetRealDataSize)
{
	TOverlappedDataBufferQueue queue;
	TOverlappedDataBuffer buffer(16384, &queue);

	buffer.SetRealDataSize(123);

	EXPECT_EQ(123, buffer.GetRealDataSize());
}

TEST(TOverlappedDataBufferTests, SetLastPart_IsLastPart)
{
	TOverlappedDataBufferQueue queue;
	TOverlappedDataBuffer buffer(16384, &queue);

	buffer.SetLastPart(true);

	EXPECT_TRUE(buffer.IsLastPart());
}

TEST(TOverlappedDataBufferTests, SetBufferOrder_GetBufferOrder)
{
	TOverlappedDataBufferQueue queue;
	TOverlappedDataBuffer buffer(16384, &queue);

	buffer.SetBufferOrder(123);

	EXPECT_EQ(123, buffer.GetBufferOrder());
}

TEST(TOverlappedDataBufferTests, SetErrorCode_GetErrorCode)
{
	TOverlappedDataBufferQueue queue;
	TOverlappedDataBuffer buffer(16384, &queue);

	buffer.SetErrorCode(123);

	EXPECT_EQ(123, buffer.GetErrorCode());
}

TEST(TOverlappedDataBufferTests, SetStatusCode_GetStatusCode)
{
	TOverlappedDataBufferQueue queue;
	TOverlappedDataBuffer buffer(16384, &queue);

	buffer.SetStatusCode(123);

	EXPECT_EQ(123, buffer.GetStatusCode());
}

TEST(TOverlappedDataBufferTests, SetBytesTransferred_GetBytesTransferred)
{
	TOverlappedDataBufferQueue queue;
	TOverlappedDataBuffer buffer(16384, &queue);

	buffer.SetBytesTransferred(123);

	EXPECT_EQ(123, buffer.GetBytesTransferred());
}

TEST(TOverlappedDataBufferTests, GetFilePosition_SetFilePosition)
{
	TOverlappedDataBufferQueue queue;
	TOverlappedDataBuffer buffer(16384, &queue);

	buffer.SetFilePosition(123);

	EXPECT_EQ(123, buffer.GetFilePosition());
}
