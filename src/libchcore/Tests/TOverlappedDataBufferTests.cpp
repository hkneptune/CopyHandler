#include "stdafx.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../TOverlappedDataBuffer.h"
#include "../TOverlappedReaderWriter.h"
#include "../../liblogger/TLogFileData.h"
#include "../OverlappedCallbacks.h"

using namespace chcore;

TEST(TOverlappedDataBufferTests, Constructor_NullParam)
{
	TOverlappedDataBuffer buffer(0, nullptr);

	EXPECT_EQ(nullptr, buffer.GetParam());
}

TEST(TOverlappedDataBufferTests, SetParam_GetParam)
{
	int iTest = 5;

	TOverlappedDataBuffer buffer(0, &iTest);

	EXPECT_EQ(&iTest, buffer.GetParam());
}

TEST(TOverlappedDataBufferTests, Constructor_SanityTest)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>());
	TOverlappedReaderWriter queue(spLogData, spBuffers, 0, 4096);
	TOverlappedDataBuffer buffer(32768, &queue);

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
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>());
	TOverlappedReaderWriter queue(spLogData, spBuffers, 0, 4096);
	TOverlappedDataBuffer buffer(32768, &queue);

	buffer.ReinitializeBuffer(16384);

	EXPECT_NE(nullptr, buffer.GetBufferPtr());
	EXPECT_EQ(16384, buffer.GetBufferSize());
}

TEST(TOverlappedDataBufferTests, ReinitializeBuffer_IncreaseSize)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>());
	TOverlappedReaderWriter queue(spLogData, spBuffers, 0, 4096);
	TOverlappedDataBuffer buffer(16384, &queue);

	buffer.ReinitializeBuffer(32768);

	EXPECT_NE(nullptr, buffer.GetBufferPtr());
	EXPECT_EQ(32768, buffer.GetBufferSize());
}

TEST(TOverlappedDataBufferTests, SetRequestedDataSize_GetRequestedDataSize)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>());
	TOverlappedReaderWriter queue(spLogData, spBuffers, 0, 4096);
	TOverlappedDataBuffer buffer(16384, &queue);

	buffer.SetRequestedDataSize(123);

	EXPECT_EQ(123, buffer.GetRequestedDataSize());
}

TEST(TOverlappedDataBufferTests, SetRealDataSize_GetRealDataSize)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>());
	TOverlappedReaderWriter queue(spLogData, spBuffers, 0, 4096);
	TOverlappedDataBuffer buffer(16384, &queue);

	buffer.SetRealDataSize(123);

	EXPECT_EQ(123, buffer.GetRealDataSize());
}

TEST(TOverlappedDataBufferTests, SetLastPart_IsLastPart)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>());
	TOverlappedReaderWriter queue(spLogData, spBuffers, 0, 4096);
	TOverlappedDataBuffer buffer(16384, &queue);

	buffer.SetLastPart(true);

	EXPECT_TRUE(buffer.IsLastPart());
}

TEST(TOverlappedDataBufferTests, SetErrorCode_GetErrorCode)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>());
	TOverlappedReaderWriter queue(spLogData, spBuffers, 0, 4096);
	TOverlappedDataBuffer buffer(16384, &queue);

	buffer.SetErrorCode(123);

	EXPECT_EQ(123, buffer.GetErrorCode());
}

TEST(TOverlappedDataBufferTests, SetStatusCode_GetStatusCode)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>());
	TOverlappedReaderWriter queue(spLogData, spBuffers, 0, 4096);
	TOverlappedDataBuffer buffer(16384, &queue);

	buffer.SetStatusCode(123);

	EXPECT_EQ(123, buffer.GetStatusCode());
}

TEST(TOverlappedDataBufferTests, SetBytesTransferred_GetBytesTransferred)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>());
	TOverlappedReaderWriter queue(spLogData, spBuffers, 0, 4096);
	TOverlappedDataBuffer buffer(16384, &queue);

	buffer.SetBytesTransferred(123);

	EXPECT_EQ(123, buffer.GetBytesTransferred());
}

TEST(TOverlappedDataBufferTests, GetFilePosition_SetFilePosition)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>());
	TOverlappedReaderWriter queue(spLogData, spBuffers, 0, 4096);
	TOverlappedDataBuffer buffer(16384, &queue);

	buffer.SetFilePosition(123);

	EXPECT_EQ(123, buffer.GetFilePosition());
}

///////////////////////////////////////////////////////////////////////////////////////////////////
TEST(TOverlappedDataBufferTests, InitForRead)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>());
	TOverlappedReaderWriter queue(spLogData, spBuffers, 0, 4096);
	TOverlappedDataBuffer buffer(16384, &queue);

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
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>());
	TOverlappedReaderWriter queue(spLogData, spBuffers, 0, 4096);
	TOverlappedDataBuffer buffer(16384, &queue);

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
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>());
	TOverlappedReaderWriter queue(spLogData, spBuffers, 0, 4096);
	TOverlappedDataBuffer buffer(16384, &queue);

	buffer.SetRequestedDataSize(123);
	buffer.SetFilePosition(1);
	buffer.SetRealDataSize(120);
	buffer.SetLastPart(true);
	buffer.SetErrorCode(54);
	buffer.SetStatusCode(3);
	buffer.SetBytesTransferred(12);

	buffer.Reset();

	EXPECT_EQ(0, buffer.GetRequestedDataSize());
	EXPECT_EQ(0, buffer.GetFilePosition());
	EXPECT_EQ(0, buffer.GetRealDataSize());
	EXPECT_EQ(false, buffer.IsLastPart());
	EXPECT_EQ(0, buffer.GetErrorCode());
	EXPECT_EQ(0, buffer.GetStatusCode());
	EXPECT_EQ(0, buffer.GetBytesTransferred());
}

///////////////////////////////////////////////////////////////////////////////////////////////////
