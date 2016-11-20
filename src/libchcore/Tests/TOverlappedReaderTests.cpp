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
	TOverlappedReader tReader(spLogData, spBuffers->GetBufferList(), spRange, 4096);

	EXPECT_EQ(nullptr, tReader.GetEmptyBuffer());
	EXPECT_EQ(nullptr, tReader.GetFailedReadBuffer());

	EXPECT_NE(nullptr, tReader.GetEventReadPossibleHandle());
	EXPECT_NE(nullptr, tReader.GetEventReadFinishedHandle());
	EXPECT_NE(nullptr, tReader.GetEventReadFailedHandle());

	EXPECT_TIMEOUT(tReader.GetEventReadPossibleHandle());
	EXPECT_TIMEOUT(tReader.GetEventReadFinishedHandle());
	EXPECT_TIMEOUT(tReader.GetEventReadFailedHandle());

	EXPECT_FALSE(tReader.IsDataSourceFinished());
}

TEST(TOverlappedReaderTests, AllocatingConstructor_SanityTest)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>(3, 32768));
	TOverlappedProcessorRangePtr spRange(std::make_shared<TOverlappedProcessorRange>(0));
	TOverlappedReader tReader(spLogData, spBuffers->GetBufferList(), spRange, 4096);

	EXPECT_SIGNALED(tReader.GetEventReadPossibleHandle());
	EXPECT_TIMEOUT(tReader.GetEventReadFailedHandle());
	EXPECT_TIMEOUT(tReader.GetEventReadFinishedHandle());

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
	TOverlappedReader tReader(spLogData, spBuffers->GetBufferList(), spRange, 4096);

	EXPECT_SIGNALED(tReader.GetEventReadPossibleHandle());

	EXPECT_NE(nullptr, tReader.GetEmptyBuffer());
	EXPECT_SIGNALED(tReader.GetEventReadPossibleHandle());

	EXPECT_NE(nullptr, tReader.GetEmptyBuffer());
	EXPECT_SIGNALED(tReader.GetEventReadPossibleHandle());

	EXPECT_NE(nullptr, tReader.GetEmptyBuffer());
	EXPECT_TIMEOUT(tReader.GetEventReadPossibleHandle());

	EXPECT_EQ(nullptr, tReader.GetEmptyBuffer());
}

TEST(TOverlappedReaderTests, AddEmptyBuffer)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>(3, 32768));
	TOverlappedProcessorRangePtr spRange(std::make_shared<TOverlappedProcessorRange>(0));
	TOverlappedReader tReader(spLogData, spBuffers->GetBufferList(), spRange, 4096);

	TOverlappedDataBuffer* pBuffers[ 3 ] = { tReader.GetEmptyBuffer(), tReader.GetEmptyBuffer(), tReader.GetEmptyBuffer() };

	EXPECT_TIMEOUT(tReader.GetEventReadPossibleHandle());

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
	TOverlappedReader tReader(spLogData, spBuffers->GetBufferList(), spRange, 4096);

	EXPECT_THROW(tReader.AddEmptyBuffer(nullptr), TCoreException);
	EXPECT_THROW(tReader.AddRetryBuffer(nullptr), TCoreException);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
TEST(TOverlappedReaderTests, AddFullBuffer_GetFullBuffer)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>(3, 32768));
	TOverlappedProcessorRangePtr spRange(std::make_shared<TOverlappedProcessorRange>(0));
	TOverlappedReader tReader(spLogData, spBuffers->GetBufferList(), spRange, 4096);
	TOverlappedDataBuffer* pBuffer = tReader.GetEmptyBuffer();

	tReader.AddFinishedReadBuffer(pBuffer);
	EXPECT_SIGNALED(tReader.GetEventReadFinishedHandle());

	EXPECT_NE(nullptr, tReader.GetFinishedReadBuffer());
	EXPECT_TIMEOUT(tReader.GetEventReadFinishedHandle());
}

TEST(TOverlappedReaderTests, GetFullBuffer_WrongOrder)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>(3, 32768));
	TOverlappedProcessorRangePtr spRange(std::make_shared<TOverlappedProcessorRange>(0));
	TOverlappedReader tReader(spLogData, spBuffers->GetBufferList(), spRange, 4096);
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
	TOverlappedReader tReader(spLogData, spBuffers->GetBufferList(), spRange, 4096);
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
	TOverlappedReader tReader(spLogData, spBuffers->GetBufferList(), spRange, 4096);

	EXPECT_THROW(tReader.AddFinishedReadBuffer(nullptr), TCoreException);
}

TEST(TOverlappedReaderTests, AddFullBuffer_SameBufferTwice)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>(3, 32768));
	TOverlappedProcessorRangePtr spRange(std::make_shared<TOverlappedProcessorRange>(0));
	TOverlappedReader tReader(spLogData, spBuffers->GetBufferList(), spRange, 4096);
	TOverlappedDataBuffer* pBuffer = tReader.GetEmptyBuffer();

	pBuffer->InitForRead(0, 1280);
	pBuffer->SetBytesTransferred(1230);
	pBuffer->SetStatusCode(0);

	tReader.AddFinishedReadBuffer(pBuffer);
	EXPECT_THROW(tReader.AddFinishedReadBuffer(pBuffer), TCoreException);
}

TEST(TOverlappedReaderTests, GetFullBuffer_AddFullBuffer_OutOfOrder)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>(3, 32768));
	TOverlappedProcessorRangePtr spRange(std::make_shared<TOverlappedProcessorRange>(0));
	TOverlappedReader tReader(spLogData, spBuffers->GetBufferList(), spRange, 4096);
	TOverlappedDataBuffer* pBuffers[ 3 ] = { tReader.GetEmptyBuffer(), tReader.GetEmptyBuffer(), tReader.GetEmptyBuffer() };

	pBuffers[ 0 ]->InitForRead(0, 1000);
	pBuffers[ 0 ]->SetBytesTransferred(1000);
	pBuffers[ 0 ]->SetStatusCode(0);

	pBuffers[ 1 ]->InitForRead(1000, 1200);
	pBuffers[ 1 ]->SetBytesTransferred(1200);
	pBuffers[ 1 ]->SetStatusCode(0);

	pBuffers[ 2 ]->InitForRead(2200, 1400);
	pBuffers[ 2 ]->SetBytesTransferred(800);
	pBuffers[ 2 ]->SetStatusCode(0);
	pBuffers[ 2 ]->SetLastPart(true);

	EXPECT_TIMEOUT(tReader.GetEventReadFinishedHandle());

	tReader.AddFinishedReadBuffer(pBuffers[ 1 ]);
	EXPECT_TIMEOUT(tReader.GetEventReadFinishedHandle());

	tReader.AddFinishedReadBuffer(pBuffers[ 2 ]);
	EXPECT_TIMEOUT(tReader.GetEventReadFinishedHandle());

	tReader.AddFinishedReadBuffer(pBuffers[ 0 ]);
	EXPECT_SIGNALED(tReader.GetEventReadFinishedHandle());
}
