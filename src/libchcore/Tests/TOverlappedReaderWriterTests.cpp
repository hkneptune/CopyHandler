#include "stdafx.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../TOverlappedReaderWriter.h"
#include "../TOverlappedDataBuffer.h"
#include "../TCoreException.h"
#include "../../liblogger/TLogFileData.h"
#include "../GTestMacros.h"

using namespace chcore;

TEST(TOverlappedReaderWriterTests, DefaultConstructor_SanityTest)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>());
	TOverlappedReaderWriter tReaderWriter(spLogData, spBuffers, 0, 4096);

	EXPECT_EQ(nullptr, tReaderWriter.GetReader()->GetEmptyBuffer());
	EXPECT_EQ(nullptr, tReaderWriter.GetWriter()->GetWriteBuffer());
	EXPECT_EQ(nullptr, tReaderWriter.GetWriter()->GetFinishedBuffer());

	EXPECT_NE(nullptr, tReaderWriter.GetReader()->GetEventReadPossibleHandle());
	EXPECT_NE(nullptr, tReaderWriter.GetWriter()->GetEventWritePossibleHandle());
	EXPECT_NE(nullptr, tReaderWriter.GetWriter()->GetEventWriteFinishedHandle());

	EXPECT_TIMEOUT(tReaderWriter.GetReader()->GetEventReadPossibleHandle());
	EXPECT_TIMEOUT(tReaderWriter.GetWriter()->GetEventWritePossibleHandle());
	EXPECT_TIMEOUT(tReaderWriter.GetWriter()->GetEventWriteFinishedHandle());

	EXPECT_FALSE(tReaderWriter.GetReader()->IsDataSourceFinished());
//	EXPECT_FALSE(tReaderWriter.IsDataWritingFinished());

	EXPECT_EQ(0, spBuffers->GetTotalBufferCount());
	EXPECT_EQ(0, spBuffers->GetSingleBufferSize());
}

TEST(TOverlappedReaderWriterTests, AllocatingConstructor_SanityTest)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>(3, 32768));
	TOverlappedReaderWriter tReaderWriter(spLogData, spBuffers, 0, 4096);

	EXPECT_NE(nullptr, tReaderWriter.GetReader()->GetEmptyBuffer());
	EXPECT_EQ(nullptr, tReaderWriter.GetWriter()->GetWriteBuffer());
	EXPECT_EQ(nullptr, tReaderWriter.GetWriter()->GetFinishedBuffer());

	EXPECT_NE(nullptr, tReaderWriter.GetReader()->GetEventReadPossibleHandle());
	EXPECT_NE(nullptr, tReaderWriter.GetWriter()->GetEventWritePossibleHandle());
	EXPECT_NE(nullptr, tReaderWriter.GetWriter()->GetEventWriteFinishedHandle());

	EXPECT_SIGNALED(tReaderWriter.GetReader()->GetEventReadPossibleHandle());
	EXPECT_TIMEOUT(tReaderWriter.GetWriter()->GetEventWritePossibleHandle());
	EXPECT_TIMEOUT(tReaderWriter.GetWriter()->GetEventWriteFinishedHandle());

	EXPECT_FALSE(tReaderWriter.GetReader()->IsDataSourceFinished());
//	EXPECT_FALSE(tReaderWriter.IsDataWritingFinished());
}

TEST(TOverlappedReaderWriterTests, AllocatingConstructor_CheckBufferSizes)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>(3, 32768));
	TOverlappedReaderWriter tReaderWriter(spLogData, spBuffers, 0, 4096);
	TOverlappedDataBuffer* pBuffers[3] = { tReaderWriter.GetReader()->GetEmptyBuffer(), tReaderWriter.GetReader()->GetEmptyBuffer(), tReaderWriter.GetReader()->GetEmptyBuffer() };

	EXPECT_EQ(3, spBuffers->GetTotalBufferCount());
	EXPECT_EQ(32768, spBuffers->GetSingleBufferSize());

	EXPECT_EQ(32768, pBuffers[0]->GetBufferSize());
	EXPECT_EQ(32768, pBuffers[1]->GetBufferSize());
	EXPECT_EQ(32768, pBuffers[2]->GetBufferSize());
}

///////////////////////////////////////////////////////////////////////////////////////////////////
TEST(TOverlappedReaderWriterTests, ReinitializeBuffer_FailsWithBuffersInUse)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>(3, 32768));
	TOverlappedReaderWriter tReaderWriter(spLogData, spBuffers, 0, 4096);

	tReaderWriter.GetReader()->GetEmptyBuffer();

	EXPECT_THROW(spBuffers->ReinitializeBuffers(3, 65536), TCoreException);
}

TEST(TOverlappedReaderWriterTests, ReinitializeBuffer_ZeroLengthBuffers)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>(3, 32768));
	TOverlappedReaderWriter tReaderWriter(spLogData, spBuffers, 0, 4096);

	EXPECT_THROW(spBuffers->ReinitializeBuffers(3, 0), TCoreException);
}

TEST(TOverlappedReaderWriterTests, ReinitializeBuffer_SameSizeSameCount)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>(3, 32768));
	TOverlappedReaderWriter tReaderWriter(spLogData, spBuffers, 0, 4096);
	spBuffers->ReinitializeBuffers(3, 32768);

	TOverlappedDataBuffer* pBuffers[3] = { tReaderWriter.GetReader()->GetEmptyBuffer(), tReaderWriter.GetReader()->GetEmptyBuffer(), tReaderWriter.GetReader()->GetEmptyBuffer() };

	EXPECT_EQ(3, spBuffers->GetTotalBufferCount());
	EXPECT_EQ(32768, spBuffers->GetSingleBufferSize());

	EXPECT_EQ(32768, pBuffers[0]->GetBufferSize());
	EXPECT_EQ(32768, pBuffers[1]->GetBufferSize());
	EXPECT_EQ(32768, pBuffers[2]->GetBufferSize());
}

TEST(TOverlappedReaderWriterTests, ReinitializeBuffer_IncreaseSize)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>(3, 32768));
	TOverlappedReaderWriter tReaderWriter(spLogData, spBuffers, 0, 4096);
	spBuffers->ReinitializeBuffers(3, 65536);

	TOverlappedDataBuffer* pBuffers[3] = { tReaderWriter.GetReader()->GetEmptyBuffer(), tReaderWriter.GetReader()->GetEmptyBuffer(), tReaderWriter.GetReader()->GetEmptyBuffer() };

	EXPECT_EQ(3, spBuffers->GetTotalBufferCount());
	EXPECT_EQ(65536, spBuffers->GetSingleBufferSize());

	EXPECT_EQ(65536, pBuffers[0]->GetBufferSize());
	EXPECT_EQ(65536, pBuffers[1]->GetBufferSize());
	EXPECT_EQ(65536, pBuffers[2]->GetBufferSize());
}

TEST(TOverlappedReaderWriterTests, ReinitializeBuffer_DecreaseSize)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>(3, 65536));
	TOverlappedReaderWriter tReaderWriter(spLogData, spBuffers, 0, 4096);
	spBuffers->ReinitializeBuffers(3, 32768);

	TOverlappedDataBuffer* pBuffers[3] = { tReaderWriter.GetReader()->GetEmptyBuffer(), tReaderWriter.GetReader()->GetEmptyBuffer(), tReaderWriter.GetReader()->GetEmptyBuffer() };

	EXPECT_EQ(3, spBuffers->GetTotalBufferCount());
	EXPECT_EQ(32768, spBuffers->GetSingleBufferSize());

	EXPECT_EQ(32768, pBuffers[0]->GetBufferSize());
	EXPECT_EQ(32768, pBuffers[1]->GetBufferSize());
	EXPECT_EQ(32768, pBuffers[2]->GetBufferSize());
}

TEST(TOverlappedReaderWriterTests, ReinitializeBuffer_IncreaseCount)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>(3, 32768));
	TOverlappedReaderWriter tReaderWriter(spLogData, spBuffers, 0, 4096);
	spBuffers->ReinitializeBuffers(5, 32768);

	EXPECT_EQ(5, spBuffers->GetTotalBufferCount());
	EXPECT_EQ(32768, spBuffers->GetSingleBufferSize());

	TOverlappedDataBuffer* pBuffers[5] = { tReaderWriter.GetReader()->GetEmptyBuffer(), tReaderWriter.GetReader()->GetEmptyBuffer(), tReaderWriter.GetReader()->GetEmptyBuffer(), tReaderWriter.GetReader()->GetEmptyBuffer(), tReaderWriter.GetReader()->GetEmptyBuffer() };

	EXPECT_EQ(32768, pBuffers[0]->GetBufferSize());
	EXPECT_EQ(32768, pBuffers[1]->GetBufferSize());
	EXPECT_EQ(32768, pBuffers[2]->GetBufferSize());
	EXPECT_EQ(32768, pBuffers[3]->GetBufferSize());
	EXPECT_EQ(32768, pBuffers[4]->GetBufferSize());
}

TEST(TOverlappedReaderWriterTests, ReinitializeBuffer_DecreaseCount)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>(5, 32768));
	TOverlappedReaderWriter tReaderWriter(spLogData, spBuffers, 0, 4096);
	spBuffers->ReinitializeBuffers(3, 32768);

	EXPECT_EQ(3, spBuffers->GetTotalBufferCount());
	EXPECT_EQ(32768, spBuffers->GetSingleBufferSize());

	TOverlappedDataBuffer* pBuffers[3] = { tReaderWriter.GetReader()->GetEmptyBuffer(), tReaderWriter.GetReader()->GetEmptyBuffer(), tReaderWriter.GetReader()->GetEmptyBuffer() };

	EXPECT_EQ(32768, pBuffers[0]->GetBufferSize());
	EXPECT_EQ(32768, pBuffers[1]->GetBufferSize());
	EXPECT_EQ(32768, pBuffers[2]->GetBufferSize());
}

///////////////////////////////////////////////////////////////////////////////////////////////////
TEST(TOverlappedReaderWriterTests, GetEmptyBuffer)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>(3, 32768));
	TOverlappedReaderWriter tReaderWriter(spLogData, spBuffers, 0, 4096);

	EXPECT_SIGNALED(tReaderWriter.GetReader()->GetEventReadPossibleHandle());

	EXPECT_NE(nullptr, tReaderWriter.GetReader()->GetEmptyBuffer());
	EXPECT_SIGNALED(tReaderWriter.GetReader()->GetEventReadPossibleHandle());

	EXPECT_NE(nullptr, tReaderWriter.GetReader()->GetEmptyBuffer());
	EXPECT_SIGNALED(tReaderWriter.GetReader()->GetEventReadPossibleHandle());

	EXPECT_NE(nullptr, tReaderWriter.GetReader()->GetEmptyBuffer());
	EXPECT_TIMEOUT(tReaderWriter.GetReader()->GetEventReadPossibleHandle());

	EXPECT_EQ(nullptr, tReaderWriter.GetReader()->GetEmptyBuffer());
}

TEST(TOverlappedReaderWriterTests, AddEmptyBuffer)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>(3, 32768));
	TOverlappedReaderWriter tReaderWriter(spLogData, spBuffers, 0, 4096);

	TOverlappedDataBuffer* pBuffers[3] = { tReaderWriter.GetReader()->GetEmptyBuffer(), tReaderWriter.GetReader()->GetEmptyBuffer(), tReaderWriter.GetReader()->GetEmptyBuffer() };

	EXPECT_TIMEOUT(tReaderWriter.GetReader()->GetEventReadPossibleHandle());

	tReaderWriter.GetReader()->AddEmptyBuffer(pBuffers[0], false);
	EXPECT_SIGNALED(tReaderWriter.GetReader()->GetEventReadPossibleHandle());

	tReaderWriter.GetReader()->AddEmptyBuffer(pBuffers[1], false);
	EXPECT_SIGNALED(tReaderWriter.GetReader()->GetEventReadPossibleHandle());

	tReaderWriter.GetReader()->AddEmptyBuffer(pBuffers[2], false);
	EXPECT_SIGNALED(tReaderWriter.GetReader()->GetEventReadPossibleHandle());
}

TEST(TOverlappedReaderWriterTests, AddEmptyBuffer_Null)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>(3, 32768));
	TOverlappedReaderWriter tReaderWriter(spLogData, spBuffers, 0, 4096);

	EXPECT_THROW(tReaderWriter.GetReader()->AddEmptyBuffer(nullptr, false), TCoreException);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
TEST(TOverlappedReaderWriterTests, AddFullBuffer_GetFullBuffer)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>(3, 32768));
	TOverlappedReaderWriter tReaderWriter(spLogData, spBuffers, 0, 4096);
	TOverlappedDataBuffer* pBuffer = tReaderWriter.GetReader()->GetEmptyBuffer();

	tReaderWriter.GetReader()->AddFullBuffer(pBuffer);
	EXPECT_SIGNALED(tReaderWriter.GetWriter()->GetEventWritePossibleHandle());

	tReaderWriter.GetWriter()->GetWriteBuffer();
	EXPECT_TIMEOUT(tReaderWriter.GetWriter()->GetEventWritePossibleHandle());
}

TEST(TOverlappedReaderWriterTests, GetFullBuffer_WrongOrder)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>(3, 32768));
	TOverlappedReaderWriter tReaderWriter(spLogData, spBuffers, 0, 4096);
	TOverlappedDataBuffer* pBuffers[3] = { tReaderWriter.GetReader()->GetEmptyBuffer(), tReaderWriter.GetReader()->GetEmptyBuffer(), tReaderWriter.GetReader()->GetEmptyBuffer() };

	tReaderWriter.GetReader()->AddFullBuffer(pBuffers[1]);
	EXPECT_EQ(nullptr, tReaderWriter.GetWriter()->GetWriteBuffer());

	tReaderWriter.GetReader()->AddFullBuffer(pBuffers[2]);
	EXPECT_EQ(nullptr, tReaderWriter.GetWriter()->GetWriteBuffer());

	tReaderWriter.GetReader()->AddFullBuffer(pBuffers[0]);
	EXPECT_NE(nullptr, tReaderWriter.GetWriter()->GetWriteBuffer());
}

TEST(TOverlappedReaderWriterTests, AddFullBuffer_HandlingSrcEof)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>(3, 32768));
	TOverlappedReaderWriter tReaderWriter(spLogData, spBuffers, 0, 4096);
	TOverlappedDataBuffer* pBuffers[3] = { tReaderWriter.GetReader()->GetEmptyBuffer(), tReaderWriter.GetReader()->GetEmptyBuffer(), tReaderWriter.GetReader()->GetEmptyBuffer() };

	pBuffers[1]->SetLastPart(true);

	tReaderWriter.GetReader()->AddFullBuffer(pBuffers[0]);
	EXPECT_FALSE(tReaderWriter.GetReader()->IsDataSourceFinished());

	tReaderWriter.GetReader()->AddFullBuffer(pBuffers[1]);
	EXPECT_TRUE(tReaderWriter.GetReader()->IsDataSourceFinished());
}

TEST(TOverlappedReaderWriterTests, AddFullBuffer_HandlingDstEof)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>(3, 32768));
	TOverlappedReaderWriter tReaderWriter(spLogData, spBuffers, 0, 4096);
	TOverlappedDataBuffer* pBuffers[3] = { tReaderWriter.GetReader()->GetEmptyBuffer(), tReaderWriter.GetReader()->GetEmptyBuffer(), tReaderWriter.GetReader()->GetEmptyBuffer() };

	pBuffers[2]->SetLastPart(true);

	tReaderWriter.GetReader()->AddFullBuffer(pBuffers[0]);
	tReaderWriter.GetReader()->AddFullBuffer(pBuffers[1]);
	tReaderWriter.GetReader()->AddFullBuffer(pBuffers[2]);

	tReaderWriter.GetWriter()->GetWriteBuffer();
//	EXPECT_FALSE(tReaderWriter.IsDataWritingFinished());

	tReaderWriter.GetWriter()->GetWriteBuffer();
//	EXPECT_FALSE(tReaderWriter.IsDataWritingFinished());

	// getting the last buffer (marked as eof) causes setting the data-writing-finished flag
	tReaderWriter.GetWriter()->GetWriteBuffer();
//	EXPECT_TRUE(tReaderWriter.IsDataWritingFinished());
}

TEST(TOverlappedReaderWriterTests, AddFullBuffer_Null)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>(3, 32768));
	TOverlappedReaderWriter tReaderWriter(spLogData, spBuffers, 0, 4096);

	EXPECT_THROW(tReaderWriter.GetReader()->AddFullBuffer(nullptr), TCoreException);
}

TEST(TOverlappedReaderWriterTests, AddFullBuffer_SameBufferTwice)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>(3, 32768));
	TOverlappedReaderWriter tReaderWriter(spLogData, spBuffers, 0, 4096);
	TOverlappedDataBuffer* pBuffer = tReaderWriter.GetReader()->GetEmptyBuffer();

	pBuffer->InitForRead(0, 1280);
	pBuffer->SetBytesTransferred(1230);
	pBuffer->SetStatusCode(0);

	tReaderWriter.GetReader()->AddFullBuffer(pBuffer);
	EXPECT_THROW(tReaderWriter.GetReader()->AddFullBuffer(pBuffer), TCoreException);
}

TEST(TOverlappedReaderWriterTests, GetFullBuffer_AddFullBuffer_OutOfOrder)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>(3, 32768));
	TOverlappedReaderWriter tReaderWriter(spLogData, spBuffers, 0, 4096);
	TOverlappedDataBuffer* pBuffers[3] = { tReaderWriter.GetReader()->GetEmptyBuffer(), tReaderWriter.GetReader()->GetEmptyBuffer(), tReaderWriter.GetReader()->GetEmptyBuffer() };

	pBuffers[0]->InitForRead(0, 1000);
	pBuffers[0]->SetBytesTransferred(1000);
	pBuffers[0]->SetStatusCode(0);

	pBuffers[1]->InitForRead(1000, 1200);
	pBuffers[1]->SetBytesTransferred(1200);
	pBuffers[1]->SetStatusCode(0);

	pBuffers[2]->InitForRead(2200, 1400);
	pBuffers[2]->SetBytesTransferred(800);
	pBuffers[2]->SetStatusCode(0);
	pBuffers[2]->SetLastPart(true);

	EXPECT_TIMEOUT(tReaderWriter.GetWriter()->GetEventWritePossibleHandle());

	tReaderWriter.GetReader()->AddFullBuffer(pBuffers[1]);
	EXPECT_TIMEOUT(tReaderWriter.GetWriter()->GetEventWritePossibleHandle());

	tReaderWriter.GetReader()->AddFullBuffer(pBuffers[2]);
	EXPECT_TIMEOUT(tReaderWriter.GetWriter()->GetEventWritePossibleHandle());

	tReaderWriter.GetReader()->AddFullBuffer(pBuffers[0]);
	EXPECT_SIGNALED(tReaderWriter.GetWriter()->GetEventWritePossibleHandle());
}

///////////////////////////////////////////////////////////////////////////////////////////////////
TEST(TOverlappedReaderWriterTests, AddFinishedBuffer_OutOfOrder_Signals)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>(3, 32768));
	TOverlappedReaderWriter tReaderWriter(spLogData, spBuffers, 0, 4096);
	TOverlappedDataBuffer* pBuffers[3] = { tReaderWriter.GetReader()->GetEmptyBuffer(), tReaderWriter.GetReader()->GetEmptyBuffer(), tReaderWriter.GetReader()->GetEmptyBuffer() };

	pBuffers[0]->InitForRead(0, 1000);
	pBuffers[0]->SetBytesTransferred(1000);
	pBuffers[0]->SetStatusCode(0);

	pBuffers[1]->InitForRead(1000, 1200);
	pBuffers[1]->SetBytesTransferred(1200);
	pBuffers[1]->SetStatusCode(0);

	pBuffers[2]->InitForRead(2200, 1400);
	pBuffers[2]->SetBytesTransferred(800);
	pBuffers[2]->SetStatusCode(0);
	pBuffers[2]->SetLastPart(true);

	tReaderWriter.GetWriter()->AddFinishedBuffer(pBuffers[1]);
	EXPECT_TIMEOUT(tReaderWriter.GetWriter()->GetEventWriteFinishedHandle());
	tReaderWriter.GetWriter()->AddFinishedBuffer(pBuffers[2]);
	EXPECT_TIMEOUT(tReaderWriter.GetWriter()->GetEventWriteFinishedHandle());
	tReaderWriter.GetWriter()->AddFinishedBuffer(pBuffers[0]);
	EXPECT_SIGNALED(tReaderWriter.GetWriter()->GetEventWriteFinishedHandle());
}

TEST(TOverlappedReaderWriterTests, GetFinishedBuffer_Signals)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>(3, 32768));
	TOverlappedReaderWriter tReaderWriter(spLogData, spBuffers, 0, 4096);
	TOverlappedDataBuffer* pBuffers[3] = { tReaderWriter.GetReader()->GetEmptyBuffer(), tReaderWriter.GetReader()->GetEmptyBuffer(), tReaderWriter.GetReader()->GetEmptyBuffer() };

	pBuffers[0]->InitForRead(0, 4096);
	pBuffers[0]->SetBytesTransferred(4096);
	pBuffers[0]->SetStatusCode(0);

	pBuffers[1]->InitForRead(4096, 4096);
	pBuffers[1]->SetBytesTransferred(4096);
	pBuffers[1]->SetStatusCode(0);

	pBuffers[2]->InitForRead(8192, 4096);
	pBuffers[2]->SetBytesTransferred(800);
	pBuffers[2]->SetStatusCode(0);
	pBuffers[2]->SetLastPart(true);

	tReaderWriter.GetWriter()->AddFinishedBuffer(pBuffers[1]);
	tReaderWriter.GetWriter()->AddFinishedBuffer(pBuffers[2]);
	tReaderWriter.GetWriter()->AddFinishedBuffer(pBuffers[0]);

	TOverlappedDataBuffer* pBuffer = tReaderWriter.GetWriter()->GetFinishedBuffer();
	EXPECT_SIGNALED(tReaderWriter.GetWriter()->GetEventWriteFinishedHandle());

	pBuffer = tReaderWriter.GetWriter()->GetFinishedBuffer();
	EXPECT_SIGNALED(tReaderWriter.GetWriter()->GetEventWriteFinishedHandle());

	pBuffer = tReaderWriter.GetWriter()->GetFinishedBuffer();
	EXPECT_TIMEOUT(tReaderWriter.GetWriter()->GetEventWriteFinishedHandle());
}

TEST(TOverlappedReaderWriterTests, GetFinishedBuffer_WrongOrder)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>(3, 32768));
	TOverlappedReaderWriter tReaderWriter(spLogData, spBuffers, 0, 4096);
	TOverlappedDataBuffer* pBuffers[3] = { tReaderWriter.GetReader()->GetEmptyBuffer(), tReaderWriter.GetReader()->GetEmptyBuffer(), tReaderWriter.GetReader()->GetEmptyBuffer() };

	pBuffers[0]->InitForRead(0, 1000);
	pBuffers[0]->SetBytesTransferred(1000);
	pBuffers[0]->SetStatusCode(0);

	pBuffers[1]->InitForRead(1000, 1200);
	pBuffers[1]->SetBytesTransferred(1200);
	pBuffers[1]->SetStatusCode(0);

	pBuffers[2]->InitForRead(2200, 1400);
	pBuffers[2]->SetBytesTransferred(800);
	pBuffers[2]->SetStatusCode(0);
	pBuffers[2]->SetLastPart(true);

	tReaderWriter.GetWriter()->AddFinishedBuffer(pBuffers[1]);
	EXPECT_EQ(nullptr, tReaderWriter.GetWriter()->GetFinishedBuffer());

	tReaderWriter.GetWriter()->AddFinishedBuffer(pBuffers[2]);
	EXPECT_EQ(nullptr, tReaderWriter.GetWriter()->GetFinishedBuffer());

	tReaderWriter.GetWriter()->AddFinishedBuffer(pBuffers[0]);
	EXPECT_NE(nullptr, tReaderWriter.GetWriter()->GetFinishedBuffer());
}

TEST(TOverlappedReaderWriterTests, AddFinishedBuffer_Null)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>(3, 32768));
	TOverlappedReaderWriter tReaderWriter(spLogData, spBuffers, 0, 4096);

	EXPECT_THROW(tReaderWriter.GetWriter()->AddFinishedBuffer(nullptr), TCoreException);
}

TEST(TOverlappedReaderWriterTests, AddFinishedBuffer_SameBufferTwice)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>(3, 32768));
	TOverlappedReaderWriter tReaderWriter(spLogData, spBuffers, 0, 4096);
	TOverlappedDataBuffer* pBuffer = tReaderWriter.GetReader()->GetEmptyBuffer();
	tReaderWriter.GetWriter()->AddFinishedBuffer(pBuffer);
	EXPECT_THROW(tReaderWriter.GetWriter()->AddFinishedBuffer(pBuffer), TCoreException);
}
