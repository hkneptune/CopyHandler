#include "stdafx.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../TOverlappedWriter.h"
#include "../GTestMacros.h"

using namespace chcore;

TEST(TOverlappedWriterTests, DefaultConstructor_SanityTest)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TBufferListPtr spEmptyBuffers(std::make_shared<TBufferList>());
	TOrderedBufferQueuePtr spQueue(std::make_shared<TOrderedBufferQueue>(spEmptyBuffers, 0));
	TOverlappedProcessorRangePtr spRange(std::make_shared<TOverlappedProcessorRange>(0));
	TSharedCountPtr<size_t> spOtfBufferCount(std::make_shared<TSharedCount<size_t>>());
	TOverlappedWriter tWriter(spLogData, spQueue, spRange, spEmptyBuffers, 1, spOtfBufferCount);

	EXPECT_EQ(nullptr, tWriter.GetWriteBuffer());
	EXPECT_EQ(nullptr, tWriter.GetFailedWriteBuffer());
	EXPECT_EQ(nullptr, tWriter.GetFinishedBuffer());

	EXPECT_NE(nullptr, tWriter.GetEventWritePossibleHandle());
	EXPECT_NE(nullptr, tWriter.GetEventWriteFailedHandle());
	EXPECT_NE(nullptr, tWriter.GetEventWriteFinishedHandle());

	EXPECT_SIGNALED(tWriter.GetEventWritePossibleHandle());
	EXPECT_TIMEOUT(tWriter.GetEventWritePossibleHandle());

	EXPECT_TIMEOUT(tWriter.GetEventWriteFailedHandle());
	EXPECT_TIMEOUT(tWriter.GetEventWriteFinishedHandle());
}

TEST(TOverlappedWriterTests, AllocatingConstructor_SanityTest)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TBufferListPtr spEmptyBuffers(std::make_shared<TBufferList>());
	TOrderedBufferQueuePtr spQueue(std::make_shared<TOrderedBufferQueue>(spEmptyBuffers, 0));
	TOverlappedProcessorRangePtr spRange(std::make_shared<TOverlappedProcessorRange>(0));
	TSharedCountPtr<size_t> spOtfBufferCount(std::make_shared<TSharedCount<size_t>>());
	TOverlappedWriter tWriter(spLogData, spQueue, spRange, spEmptyBuffers, 1, spOtfBufferCount);

	EXPECT_SIGNALED(tWriter.GetEventWritePossibleHandle());
	EXPECT_TIMEOUT(tWriter.GetEventWriteFailedHandle());
	EXPECT_TIMEOUT(tWriter.GetEventWriteFinishedHandle());
}
