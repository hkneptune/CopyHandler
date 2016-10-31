#include "stdafx.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../OverlappedCallbacks.h"
#include "../TOverlappedMemoryPool.h"
#include "../../liblogger/TLogFileData.h"
#include "../TOverlappedReader.h"
#include "../TOverlappedWriter.h"

using namespace chcore;

TEST(OverlappedCallbackTests, OverlappedReadCompleted_Success)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>());
	TOverlappedReader queue(spLogData, spBuffers->GetBufferList(), 0, 4096);
	TOverlappedDataBuffer buffer(16384, &queue);

	buffer.InitForRead(0, 1024);
	buffer.SetStatusCode(0);
	buffer.SetBytesTransferred(234);
	buffer.SetParam(&queue);

	OverlappedReadCompleted(ERROR_SUCCESS, 234, &buffer);

	EXPECT_TRUE(buffer.IsLastPart());
	EXPECT_EQ(ERROR_SUCCESS, buffer.GetErrorCode());
	EXPECT_EQ(234, buffer.GetRealDataSize());
}

TEST(OverlappedCallbackTests, OverlappedReadCompleted_Failure)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>());
	TOverlappedReader queue(spLogData, spBuffers->GetBufferList(), 0, 4096);
	TOverlappedDataBuffer buffer(16384, &queue);

	buffer.InitForRead(0, 1024);
	buffer.SetStatusCode(0);
	buffer.SetBytesTransferred(0);

	OverlappedReadCompleted(ERROR_ACCESS_DENIED, 0, &buffer);

	EXPECT_FALSE(buffer.IsLastPart());
	EXPECT_EQ(ERROR_ACCESS_DENIED, buffer.GetErrorCode());
	EXPECT_EQ(0, buffer.GetRealDataSize());

	EXPECT_EQ(queue.GetFailedReadBuffer(), &buffer);
}

TEST(OverlappedCallbackTests, OverlappedWriteCompleted_Success)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>());
	TOrderedBufferQueuePtr spBuffersToWrite(std::make_shared<TOrderedBufferQueue>(0));
	TOverlappedWriter queue(spLogData, spBuffersToWrite, 0, spBuffers->GetBufferList());
	TOverlappedDataBuffer buffer(16384, &queue);

	buffer.InitForRead(0, 1024);
	buffer.SetStatusCode(0);
	buffer.SetBytesTransferred(234);
	buffer.SetLastPart(true);
	buffer.SetRealDataSize(234);

	OverlappedWriteCompleted(ERROR_SUCCESS, 234, &buffer);

	EXPECT_EQ(ERROR_SUCCESS, buffer.GetErrorCode());
	EXPECT_EQ(queue.GetFinishedBuffer(), &buffer);
}
