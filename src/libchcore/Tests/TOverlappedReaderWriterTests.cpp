#include "stdafx.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../TOverlappedReaderWriter.h"
#include "../TOverlappedDataBuffer.h"
#include "../TCoreException.h"
#include "../../liblogger/TLogFileData.h"

using namespace chcore;

#define EXPECT_TIMEOUT(handle)\
	{\
		DWORD dwResult = WaitForSingleObject(handle, 0); \
		EXPECT_EQ(WAIT_TIMEOUT, dwResult); \
	}

#define EXPECT_SIGNALED(handle)\
	{\
		DWORD dwResult = WaitForSingleObject(handle, 0); \
		EXPECT_EQ(WAIT_OBJECT_0 + 0, dwResult); \
	}


TEST(TOverlappedReaderWriterTests, DefaultConstructor_SanityTest)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>());
	TOverlappedReaderWriter tReaderWriter(spLogData, spBuffers, 0, 4096);

	EXPECT_EQ(nullptr, tReaderWriter.GetEmptyBuffer());
	EXPECT_EQ(nullptr, tReaderWriter.GetFullBuffer());
	EXPECT_EQ(nullptr, tReaderWriter.GetFinishedBuffer());

	EXPECT_NE(nullptr, tReaderWriter.GetEventReadPossibleHandle());
	EXPECT_NE(nullptr, tReaderWriter.GetEventWritePossibleHandle());
	EXPECT_NE(nullptr, tReaderWriter.GetEventWriteFinishedHandle());

	EXPECT_TIMEOUT(tReaderWriter.GetEventReadPossibleHandle());
	EXPECT_TIMEOUT(tReaderWriter.GetEventWritePossibleHandle());
	EXPECT_TIMEOUT(tReaderWriter.GetEventWriteFinishedHandle());

	EXPECT_FALSE(tReaderWriter.IsDataSourceFinished());
	EXPECT_FALSE(tReaderWriter.IsDataWritingFinished());

	EXPECT_EQ(0, spBuffers->GetTotalBufferCount());
	EXPECT_EQ(0, spBuffers->GetSingleBufferSize());
}

TEST(TOverlappedReaderWriterTests, AllocatingConstructor_SanityTest)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>(3, 32768));
	TOverlappedReaderWriter tReaderWriter(spLogData, spBuffers, 0, 4096);

	EXPECT_NE(nullptr, tReaderWriter.GetEmptyBuffer());
	EXPECT_EQ(nullptr, tReaderWriter.GetFullBuffer());
	EXPECT_EQ(nullptr, tReaderWriter.GetFinishedBuffer());

	EXPECT_NE(nullptr, tReaderWriter.GetEventReadPossibleHandle());
	EXPECT_NE(nullptr, tReaderWriter.GetEventWritePossibleHandle());
	EXPECT_NE(nullptr, tReaderWriter.GetEventWriteFinishedHandle());

	EXPECT_SIGNALED(tReaderWriter.GetEventReadPossibleHandle());
	EXPECT_TIMEOUT(tReaderWriter.GetEventWritePossibleHandle());
	EXPECT_TIMEOUT(tReaderWriter.GetEventWriteFinishedHandle());

	EXPECT_FALSE(tReaderWriter.IsDataSourceFinished());
	EXPECT_FALSE(tReaderWriter.IsDataWritingFinished());
}

TEST(TOverlappedReaderWriterTests, AllocatingConstructor_CheckBufferSizes)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>(3, 32768));
	TOverlappedReaderWriter tReaderWriter(spLogData, spBuffers, 0, 4096);
	TOverlappedDataBuffer* pBuffers[3] = { tReaderWriter.GetEmptyBuffer(), tReaderWriter.GetEmptyBuffer(), tReaderWriter.GetEmptyBuffer() };

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

	tReaderWriter.GetEmptyBuffer();

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

	TOverlappedDataBuffer* pBuffers[3] = { tReaderWriter.GetEmptyBuffer(), tReaderWriter.GetEmptyBuffer(), tReaderWriter.GetEmptyBuffer() };

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

	TOverlappedDataBuffer* pBuffers[3] = { tReaderWriter.GetEmptyBuffer(), tReaderWriter.GetEmptyBuffer(), tReaderWriter.GetEmptyBuffer() };

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

	TOverlappedDataBuffer* pBuffers[3] = { tReaderWriter.GetEmptyBuffer(), tReaderWriter.GetEmptyBuffer(), tReaderWriter.GetEmptyBuffer() };

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

	TOverlappedDataBuffer* pBuffers[5] = { tReaderWriter.GetEmptyBuffer(), tReaderWriter.GetEmptyBuffer(), tReaderWriter.GetEmptyBuffer(), tReaderWriter.GetEmptyBuffer(), tReaderWriter.GetEmptyBuffer() };

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

	TOverlappedDataBuffer* pBuffers[3] = { tReaderWriter.GetEmptyBuffer(), tReaderWriter.GetEmptyBuffer(), tReaderWriter.GetEmptyBuffer() };

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

	EXPECT_SIGNALED(tReaderWriter.GetEventReadPossibleHandle());

	EXPECT_NE(nullptr, tReaderWriter.GetEmptyBuffer());
	EXPECT_SIGNALED(tReaderWriter.GetEventReadPossibleHandle());

	EXPECT_NE(nullptr, tReaderWriter.GetEmptyBuffer());
	EXPECT_SIGNALED(tReaderWriter.GetEventReadPossibleHandle());

	EXPECT_NE(nullptr, tReaderWriter.GetEmptyBuffer());
	EXPECT_TIMEOUT(tReaderWriter.GetEventReadPossibleHandle());

	EXPECT_EQ(nullptr, tReaderWriter.GetEmptyBuffer());
}

TEST(TOverlappedReaderWriterTests, AddEmptyBuffer)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>(3, 32768));
	TOverlappedReaderWriter tReaderWriter(spLogData, spBuffers, 0, 4096);

	TOverlappedDataBuffer* pBuffers[3] = { tReaderWriter.GetEmptyBuffer(), tReaderWriter.GetEmptyBuffer(), tReaderWriter.GetEmptyBuffer() };

	EXPECT_TIMEOUT(tReaderWriter.GetEventReadPossibleHandle());

	tReaderWriter.AddEmptyBuffer(pBuffers[0]);
	EXPECT_SIGNALED(tReaderWriter.GetEventReadPossibleHandle());

	tReaderWriter.AddEmptyBuffer(pBuffers[1]);
	EXPECT_SIGNALED(tReaderWriter.GetEventReadPossibleHandle());

	tReaderWriter.AddEmptyBuffer(pBuffers[2]);
	EXPECT_SIGNALED(tReaderWriter.GetEventReadPossibleHandle());
}

TEST(TOverlappedReaderWriterTests, AddEmptyBuffer_Null)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>(3, 32768));
	TOverlappedReaderWriter tReaderWriter(spLogData, spBuffers, 0, 4096);

	EXPECT_THROW(tReaderWriter.AddEmptyBuffer(nullptr), TCoreException);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
TEST(TOverlappedReaderWriterTests, AddFullBuffer_GetFullBuffer)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>(3, 32768));
	TOverlappedReaderWriter tReaderWriter(spLogData, spBuffers, 0, 4096);
	TOverlappedDataBuffer* pBuffer = tReaderWriter.GetEmptyBuffer();

	tReaderWriter.AddFullBuffer(pBuffer);
	EXPECT_SIGNALED(tReaderWriter.GetEventWritePossibleHandle());

	tReaderWriter.GetFullBuffer();
	EXPECT_TIMEOUT(tReaderWriter.GetEventWritePossibleHandle());
}

TEST(TOverlappedReaderWriterTests, GetFullBuffer_WrongOrder)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>(3, 32768));
	TOverlappedReaderWriter tReaderWriter(spLogData, spBuffers, 0, 4096);
	TOverlappedDataBuffer* pBuffers[3] = { tReaderWriter.GetEmptyBuffer(), tReaderWriter.GetEmptyBuffer(), tReaderWriter.GetEmptyBuffer() };

	tReaderWriter.AddFullBuffer(pBuffers[1]);
	EXPECT_EQ(nullptr, tReaderWriter.GetFullBuffer());

	tReaderWriter.AddFullBuffer(pBuffers[2]);
	EXPECT_EQ(nullptr, tReaderWriter.GetFullBuffer());

	tReaderWriter.AddFullBuffer(pBuffers[0]);
	EXPECT_NE(nullptr, tReaderWriter.GetFullBuffer());
}

TEST(TOverlappedReaderWriterTests, AddFullBuffer_HandlingSrcEof)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>(3, 32768));
	TOverlappedReaderWriter tReaderWriter(spLogData, spBuffers, 0, 4096);
	TOverlappedDataBuffer* pBuffers[3] = { tReaderWriter.GetEmptyBuffer(), tReaderWriter.GetEmptyBuffer(), tReaderWriter.GetEmptyBuffer() };

	pBuffers[1]->SetLastPart(true);

	tReaderWriter.AddFullBuffer(pBuffers[0]);
	EXPECT_FALSE(tReaderWriter.IsDataSourceFinished());

	tReaderWriter.AddFullBuffer(pBuffers[1]);
	EXPECT_TRUE(tReaderWriter.IsDataSourceFinished());
}

TEST(TOverlappedReaderWriterTests, AddFullBuffer_HandlingDstEof)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>(3, 32768));
	TOverlappedReaderWriter tReaderWriter(spLogData, spBuffers, 0, 4096);
	TOverlappedDataBuffer* pBuffers[3] = { tReaderWriter.GetEmptyBuffer(), tReaderWriter.GetEmptyBuffer(), tReaderWriter.GetEmptyBuffer() };

	pBuffers[2]->SetLastPart(true);

	tReaderWriter.AddFullBuffer(pBuffers[0]);
	tReaderWriter.AddFullBuffer(pBuffers[1]);
	tReaderWriter.AddFullBuffer(pBuffers[2]);

	tReaderWriter.GetFullBuffer();
	EXPECT_FALSE(tReaderWriter.IsDataWritingFinished());

	tReaderWriter.GetFullBuffer();
	EXPECT_FALSE(tReaderWriter.IsDataWritingFinished());

	// getting the last buffer (marked as eof) causes setting the data-writing-finished flag
	tReaderWriter.GetFullBuffer();
	EXPECT_TRUE(tReaderWriter.IsDataWritingFinished());
}

TEST(TOverlappedReaderWriterTests, AddFullBuffer_Null)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>(3, 32768));
	TOverlappedReaderWriter tReaderWriter(spLogData, spBuffers, 0, 4096);

	EXPECT_THROW(tReaderWriter.AddFullBuffer(nullptr), TCoreException);
}

TEST(TOverlappedReaderWriterTests, AddFullBuffer_SameBufferTwice)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>(3, 32768));
	TOverlappedReaderWriter tReaderWriter(spLogData, spBuffers, 0, 4096);
	TOverlappedDataBuffer* pBuffer = tReaderWriter.GetEmptyBuffer();

	pBuffer->InitForRead(0, 1280);
	pBuffer->SetBytesTransferred(1230);
	pBuffer->SetStatusCode(0);

	tReaderWriter.AddFullBuffer(pBuffer);
	EXPECT_THROW(tReaderWriter.AddFullBuffer(pBuffer), TCoreException);
}

TEST(TOverlappedReaderWriterTests, GetFullBuffer_AddFullBuffer_OutOfOrder)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>(3, 32768));
	TOverlappedReaderWriter tReaderWriter(spLogData, spBuffers, 0, 4096);
	TOverlappedDataBuffer* pBuffers[3] = { tReaderWriter.GetEmptyBuffer(), tReaderWriter.GetEmptyBuffer(), tReaderWriter.GetEmptyBuffer() };

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

	EXPECT_TIMEOUT(tReaderWriter.GetEventWritePossibleHandle());

	tReaderWriter.AddFullBuffer(pBuffers[1]);
	EXPECT_TIMEOUT(tReaderWriter.GetEventWritePossibleHandle());

	tReaderWriter.AddFullBuffer(pBuffers[2]);
	EXPECT_TIMEOUT(tReaderWriter.GetEventWritePossibleHandle());

	tReaderWriter.AddFullBuffer(pBuffers[0]);
	EXPECT_SIGNALED(tReaderWriter.GetEventWritePossibleHandle());
}

///////////////////////////////////////////////////////////////////////////////////////////////////
TEST(TOverlappedReaderWriterTests, AddFinishedBuffer_OutOfOrder_Signals)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>(3, 32768));
	TOverlappedReaderWriter tReaderWriter(spLogData, spBuffers, 0, 4096);
	TOverlappedDataBuffer* pBuffers[3] = { tReaderWriter.GetEmptyBuffer(), tReaderWriter.GetEmptyBuffer(), tReaderWriter.GetEmptyBuffer() };

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

	tReaderWriter.AddFinishedBuffer(pBuffers[1]);
	EXPECT_TIMEOUT(tReaderWriter.GetEventWriteFinishedHandle());
	tReaderWriter.AddFinishedBuffer(pBuffers[2]);
	EXPECT_TIMEOUT(tReaderWriter.GetEventWriteFinishedHandle());
	tReaderWriter.AddFinishedBuffer(pBuffers[0]);
	EXPECT_SIGNALED(tReaderWriter.GetEventWriteFinishedHandle());
}

TEST(TOverlappedReaderWriterTests, GetFinishedBuffer_Signals)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>(3, 32768));
	TOverlappedReaderWriter tReaderWriter(spLogData, spBuffers, 0, 4096);
	TOverlappedDataBuffer* pBuffers[3] = { tReaderWriter.GetEmptyBuffer(), tReaderWriter.GetEmptyBuffer(), tReaderWriter.GetEmptyBuffer() };

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

	tReaderWriter.AddFinishedBuffer(pBuffers[1]);
	tReaderWriter.AddFinishedBuffer(pBuffers[2]);
	tReaderWriter.AddFinishedBuffer(pBuffers[0]);

	TOverlappedDataBuffer* pBuffer = tReaderWriter.GetFinishedBuffer();
	EXPECT_TIMEOUT(tReaderWriter.GetEventWriteFinishedHandle());
	tReaderWriter.MarkFinishedBufferAsComplete(pBuffer);
	EXPECT_SIGNALED(tReaderWriter.GetEventWriteFinishedHandle());

	pBuffer = tReaderWriter.GetFinishedBuffer();
	EXPECT_TIMEOUT(tReaderWriter.GetEventWriteFinishedHandle());
	tReaderWriter.MarkFinishedBufferAsComplete(pBuffer);
	EXPECT_SIGNALED(tReaderWriter.GetEventWriteFinishedHandle());

	pBuffer = tReaderWriter.GetFinishedBuffer();
	EXPECT_TIMEOUT(tReaderWriter.GetEventWriteFinishedHandle());
	tReaderWriter.MarkFinishedBufferAsComplete(pBuffer);
	EXPECT_TIMEOUT(tReaderWriter.GetEventWriteFinishedHandle());
}

TEST(TOverlappedReaderWriterTests, GetFinishedBuffer_WrongOrder)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>(3, 32768));
	TOverlappedReaderWriter tReaderWriter(spLogData, spBuffers, 0, 4096);
	TOverlappedDataBuffer* pBuffers[3] = { tReaderWriter.GetEmptyBuffer(), tReaderWriter.GetEmptyBuffer(), tReaderWriter.GetEmptyBuffer() };

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

	tReaderWriter.AddFinishedBuffer(pBuffers[1]);
	EXPECT_EQ(nullptr, tReaderWriter.GetFinishedBuffer());

	tReaderWriter.AddFinishedBuffer(pBuffers[2]);
	EXPECT_EQ(nullptr, tReaderWriter.GetFinishedBuffer());

	tReaderWriter.AddFinishedBuffer(pBuffers[0]);
	EXPECT_NE(nullptr, tReaderWriter.GetFinishedBuffer());
}

TEST(TOverlappedReaderWriterTests, AddFinishedBuffer_Null)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>(3, 32768));
	TOverlappedReaderWriter tReaderWriter(spLogData, spBuffers, 0, 4096);

	EXPECT_THROW(tReaderWriter.AddFinishedBuffer(nullptr), TCoreException);
}

TEST(TOverlappedReaderWriterTests, AddFinishedBuffer_SameBufferTwice)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>(3, 32768));
	TOverlappedReaderWriter tReaderWriter(spLogData, spBuffers, 0, 4096);
	TOverlappedDataBuffer* pBuffer = tReaderWriter.GetEmptyBuffer();
	tReaderWriter.AddFinishedBuffer(pBuffer);
	EXPECT_THROW(tReaderWriter.AddFinishedBuffer(pBuffer), TCoreException);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
TEST(TOverlappedReaderWriterTests, DataSourceChanged_CleanupBuffers)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>(3, 32768));
	TOverlappedReaderWriter tReaderWriter(spLogData, spBuffers, 0, 4096);
	TOverlappedDataBuffer* pBuffers[3] = { tReaderWriter.GetEmptyBuffer(), tReaderWriter.GetEmptyBuffer(), tReaderWriter.GetEmptyBuffer() };

	pBuffers[0]->SetLastPart(true);
	pBuffers[1]->SetLastPart(true);
	pBuffers[2]->SetLastPart(true);

	tReaderWriter.AddFullBuffer(pBuffers[1]);
	tReaderWriter.AddFullBuffer(pBuffers[2]);
	tReaderWriter.AddFullBuffer(pBuffers[0]);

	// this tests if the buffers are properly cleaned up - if they're not, DataSourceChanged() throws an exception
	EXPECT_NO_THROW(tReaderWriter.DataSourceChanged());
}

TEST(TOverlappedReaderWriterTests, DataSourceChanged_InvalidBufferCount)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>(3, 32768));
	TOverlappedReaderWriter tReaderWriter(spLogData, spBuffers, 0, 4096);

	tReaderWriter.GetEmptyBuffer();

	// this tests if the buffers are properly cleaned up - if they're not, DataSourceChanged() throws an exception
	EXPECT_THROW(tReaderWriter.DataSourceChanged(), TCoreException);
}
