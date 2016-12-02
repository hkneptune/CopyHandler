#include "stdafx.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../TOverlappedDataBuffer.h"
#include "../TCoreException.h"
#include "../../liblogger/TLogFileData.h"
#include "../GTestMacros.h"
#include "../TOverlappedMemoryPool.h"
#include "../TOverlappedReader.h"

using namespace chcore;

TEST(TOverlappedReaderTests, DefaultConstructor_SanityTest)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>());
	TOverlappedProcessorRangePtr spRange(std::make_shared<TOverlappedProcessorRange>(0));
	TSharedCountPtr<size_t> spOtfBufferCount(std::make_shared<TSharedCount<size_t>>());
	TOverlappedReader tReader(spLogData, spBuffers->GetBufferList(), spRange, 4096, 1, 1, spOtfBufferCount);

	EXPECT_EQ(nullptr, tReader.GetEmptyBuffer());
	EXPECT_EQ(nullptr, tReader.GetFailedReadBuffer());

	EXPECT_NE(nullptr, tReader.GetEventReadPossibleHandle());
	EXPECT_NE(nullptr, tReader.GetEventReadFailedHandle());

	EXPECT_SIGNALED(tReader.GetEventReadPossibleHandle());
	EXPECT_TIMEOUT(tReader.GetEventReadFailedHandle());

	EXPECT_FALSE(tReader.IsDataSourceFinished());
}

TEST(TOverlappedReaderTests, AllocatingConstructor_SanityTest)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>(3, 32768));
	TOverlappedProcessorRangePtr spRange(std::make_shared<TOverlappedProcessorRange>(0));
	TSharedCountPtr<size_t> spOtfBufferCount(std::make_shared<TSharedCount<size_t>>());
	TOverlappedReader tReader(spLogData, spBuffers->GetBufferList(), spRange, 4096, 1, 1, spOtfBufferCount);

	EXPECT_SIGNALED(tReader.GetEventReadPossibleHandle());
	EXPECT_TIMEOUT(tReader.GetEventReadFailedHandle());

	EXPECT_NE(nullptr, tReader.GetEmptyBuffer());
	EXPECT_NE(nullptr, tReader.GetEmptyBuffer());
	EXPECT_NE(nullptr, tReader.GetEmptyBuffer());
	EXPECT_EQ(nullptr, tReader.GetEmptyBuffer());

	EXPECT_EQ(nullptr, tReader.GetFailedReadBuffer());
	EXPECT_FALSE(tReader.IsDataSourceFinished());
}

///////////////////////////////////////////////////////////////////////////////////////////////////
TEST(TOverlappedReaderTests, GetEmptyBuffer)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>(3, 32768));
	TOverlappedProcessorRangePtr spRange(std::make_shared<TOverlappedProcessorRange>(0));
	TSharedCountPtr<size_t> spOtfBufferCount(std::make_shared<TSharedCount<size_t>>());
	TOverlappedReader tReader(spLogData, spBuffers->GetBufferList(), spRange, 4096, 1, 1, spOtfBufferCount);

	EXPECT_SIGNALED(tReader.GetEventReadPossibleHandle());

	EXPECT_NE(nullptr, tReader.GetEmptyBuffer());
	EXPECT_SIGNALED(tReader.GetEventReadPossibleHandle());

	EXPECT_NE(nullptr, tReader.GetEmptyBuffer());
	EXPECT_SIGNALED(tReader.GetEventReadPossibleHandle());

	EXPECT_NE(nullptr, tReader.GetEmptyBuffer());

	EXPECT_SIGNALED(tReader.GetEventReadPossibleHandle());
	EXPECT_EQ(nullptr, tReader.GetEmptyBuffer());
	EXPECT_TIMEOUT(tReader.GetEventReadPossibleHandle());
}

TEST(TOverlappedReaderTests, AddEmptyBuffer)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>(3, 32768));
	TOverlappedProcessorRangePtr spRange(std::make_shared<TOverlappedProcessorRange>(0));
	TSharedCountPtr<size_t> spOtfBufferCount(std::make_shared<TSharedCount<size_t>>());
	TOverlappedReader tReader(spLogData, spBuffers->GetBufferList(), spRange, 4096, 1, 1, spOtfBufferCount);

	TOverlappedDataBuffer* pBuffers[ 3 ] = { tReader.GetEmptyBuffer(), tReader.GetEmptyBuffer(), tReader.GetEmptyBuffer() };

	EXPECT_SIGNALED(tReader.GetEventReadPossibleHandle());

	tReader.AddEmptyBuffer(pBuffers[ 0 ]);
	EXPECT_SIGNALED(tReader.GetEventReadPossibleHandle());

	tReader.AddEmptyBuffer(pBuffers[ 1 ]);
	EXPECT_SIGNALED(tReader.GetEventReadPossibleHandle());

	tReader.AddEmptyBuffer(pBuffers[ 2 ]);
	EXPECT_SIGNALED(tReader.GetEventReadPossibleHandle());
}

TEST(TOverlappedReaderTests, AddEmptyBuffer_Null)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>(3, 32768));
	TOverlappedProcessorRangePtr spRange(std::make_shared<TOverlappedProcessorRange>(0));
	TSharedCountPtr<size_t> spOtfBufferCount(std::make_shared<TSharedCount<size_t>>());
	TOverlappedReader tReader(spLogData, spBuffers->GetBufferList(), spRange, 4096, 1, 1, spOtfBufferCount);

	EXPECT_THROW(tReader.AddEmptyBuffer(nullptr), TCoreException);
	EXPECT_THROW(tReader.AddRetryBuffer(nullptr), TCoreException);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
TEST(TOverlappedReaderTests, AddFullBuffer_GetFullBuffer)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>(3, 32768));
	TOverlappedProcessorRangePtr spRange(std::make_shared<TOverlappedProcessorRange>(0));
	TSharedCountPtr<size_t> spOtfBufferCount(std::make_shared<TSharedCount<size_t>>());
	TOverlappedReader tReader(spLogData, spBuffers->GetBufferList(), spRange, 4096, 1, 1, spOtfBufferCount);

	TOverlappedDataBuffer* pBuffer = tReader.GetEmptyBuffer();

	tReader.AddFinishedReadBuffer(pBuffer);
	EXPECT_NE(nullptr, tReader.GetFinishedReadBuffer());
}

TEST(TOverlappedReaderTests, GetFullBuffer_WrongOrder)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>(3, 32768));
	TOverlappedProcessorRangePtr spRange(std::make_shared<TOverlappedProcessorRange>(0));
	TSharedCountPtr<size_t> spOtfBufferCount(std::make_shared<TSharedCount<size_t>>());
	TOverlappedReader tReader(spLogData, spBuffers->GetBufferList(), spRange, 4096, 1, 1, spOtfBufferCount);

	TOverlappedDataBuffer* pBuffers[ 3 ] = { tReader.GetEmptyBuffer(), tReader.GetEmptyBuffer(), tReader.GetEmptyBuffer() };

	tReader.AddFinishedReadBuffer(pBuffers[ 1 ]);
	EXPECT_EQ(nullptr, tReader.GetFinishedReadBuffer());

	tReader.AddFinishedReadBuffer(pBuffers[ 2 ]);
	EXPECT_EQ(nullptr, tReader.GetFinishedReadBuffer());

	tReader.AddFinishedReadBuffer(pBuffers[ 0 ]);
	EXPECT_NE(nullptr, tReader.GetFinishedReadBuffer());
}

TEST(TOverlappedReaderTests, AddFullBuffer_HandlingSrcEof)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>(3, 32768));
	TOverlappedProcessorRangePtr spRange(std::make_shared<TOverlappedProcessorRange>(0));
	TSharedCountPtr<size_t> spOtfBufferCount(std::make_shared<TSharedCount<size_t>>());
	TOverlappedReader tReader(spLogData, spBuffers->GetBufferList(), spRange, 4096, 1, 1, spOtfBufferCount);

	TOverlappedDataBuffer* pBuffers[ 3 ] = { tReader.GetEmptyBuffer(), tReader.GetEmptyBuffer(), tReader.GetEmptyBuffer() };

	pBuffers[ 1 ]->SetLastPart(true);

	tReader.AddFinishedReadBuffer(pBuffers[ 0 ]);
	EXPECT_FALSE(tReader.IsDataSourceFinished());

	tReader.AddFinishedReadBuffer(pBuffers[ 1 ]);
	EXPECT_TRUE(tReader.IsDataSourceFinished());
}

TEST(TOverlappedReaderTests, AddFullBuffer_Null)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>(3, 32768));
	TOverlappedProcessorRangePtr spRange(std::make_shared<TOverlappedProcessorRange>(0));
	TSharedCountPtr<size_t> spOtfBufferCount(std::make_shared<TSharedCount<size_t>>());
	TOverlappedReader tReader(spLogData, spBuffers->GetBufferList(), spRange, 4096, 1, 1, spOtfBufferCount);

	EXPECT_THROW(tReader.AddFinishedReadBuffer(nullptr), TCoreException);
}

TEST(TOverlappedReaderTests, AddFullBuffer_SameBufferTwice)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>(3, 32768));
	TOverlappedProcessorRangePtr spRange(std::make_shared<TOverlappedProcessorRange>(0));
	TSharedCountPtr<size_t> spOtfBufferCount(std::make_shared<TSharedCount<size_t>>());
	TOverlappedReader tReader(spLogData, spBuffers->GetBufferList(), spRange, 4096, 1, 1, spOtfBufferCount);

	TOverlappedDataBuffer* pBuffer = tReader.GetEmptyBuffer();

	pBuffer->InitForRead(0, 1280);
	pBuffer->SetBytesTransferred(1230);
	pBuffer->SetStatusCode(0);

	tReader.AddFinishedReadBuffer(pBuffer);
	EXPECT_THROW(tReader.AddFinishedReadBuffer(pBuffer), TCoreException);
}
