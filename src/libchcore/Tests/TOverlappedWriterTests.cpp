#include "stdafx.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../TOverlappedWriter.h"

using namespace chcore;

TEST(TOverlappedWriterTests, DefaultTest)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());
	TOrderedBufferQueuePtr spQueue(std::make_shared<TOrderedBufferQueue>());

	TOverlappedWriter writer(spLogData, spQueue, 0);
}
