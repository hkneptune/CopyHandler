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
	TOrderedBufferQueuePtr spQueue(std::make_shared<TOrderedBufferQueue>(0));
	TOverlappedWriter tWriter(spLogData, spQueue, 0, spEmptyBuffers);

	EXPECT_EQ(nullptr, tWriter.GetWriteBuffer());
	EXPECT_EQ(nullptr, tWriter.GetFailedWriteBuffer());
	EXPECT_EQ(nullptr, tWriter.GetFinishedBuffer());

	EXPECT_NE(nullptr, tWriter.GetEventWritePossibleHandle());
	EXPECT_NE(nullptr, tWriter.GetEventWriteFailedHandle());
	EXPECT_NE(nullptr, tWriter.GetEventWriteFinishedHandle());

	EXPECT_TIMEOUT(tWriter.GetEventWritePossibleHandle());
	EXPECT_TIMEOUT(tWriter.GetEventWriteFailedHandle());
	EXPECT_TIMEOUT(tWriter.GetEventWriteFinishedHandle());
}

TEST(TOverlappedWriterTests, AllocatingConstructor_SanityTest)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TBufferListPtr spEmptyBuffers(std::make_shared<TBufferList>());
	TOrderedBufferQueuePtr spQueue(std::make_shared<TOrderedBufferQueue>(0));
	TOverlappedWriter tWriter(spLogData, spQueue, 0, spEmptyBuffers);

	EXPECT_TIMEOUT(tWriter.GetEventWritePossibleHandle());
	EXPECT_TIMEOUT(tWriter.GetEventWriteFailedHandle());
	EXPECT_TIMEOUT(tWriter.GetEventWriteFinishedHandle());
}
