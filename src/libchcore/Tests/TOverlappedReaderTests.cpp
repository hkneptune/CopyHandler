#include "stdafx.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../TOverlappedReader.h"

using namespace chcore;

TEST(TOverlappedReaderTests, DefaultTest)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());
	TBufferListPtr spQueue(std::make_shared<TBufferList>());
	TOverlappedReader reader(spLogData, spQueue, 0, 4096);
}
