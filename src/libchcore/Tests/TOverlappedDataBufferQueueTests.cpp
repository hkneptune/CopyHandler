#include "stdafx.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../TOverlappedDataBufferQueue.h"
#include "../TOverlappedDataBuffer.h"

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


TEST(TOverlappedDataBufferQueueTests, DefaultConstructor_SanityTest)
{
	TOverlappedDataBufferQueue queue;

	EXPECT_EQ(nullptr, queue.GetEmptyBuffer());
	EXPECT_EQ(nullptr, queue.GetFullBuffer());
	EXPECT_EQ(nullptr, queue.GetFinishedBuffer());

	EXPECT_NE(nullptr, queue.GetEventReadPossibleHandle());
	EXPECT_NE(nullptr, queue.GetEventWritePossibleHandle());
	EXPECT_NE(nullptr, queue.GetEventWriteFinishedHandle());

	DWORD dwResult = WaitForSingleObject(queue.GetEventReadPossibleHandle(), 0);
	EXPECT_EQ(WAIT_TIMEOUT, dwResult);

	dwResult = WaitForSingleObject(queue.GetEventWritePossibleHandle(), 0);
	EXPECT_EQ(WAIT_TIMEOUT, dwResult);

	dwResult = WaitForSingleObject(queue.GetEventWriteFinishedHandle(), 0);
	EXPECT_EQ(WAIT_TIMEOUT, dwResult);
}

TEST(TOverlappedDataBufferQueueTests, AllocatingConstructor_SanityTest)
{
	TOverlappedDataBufferQueue queue(3, 32768);

	EXPECT_NE(nullptr, queue.GetEmptyBuffer());
	EXPECT_EQ(nullptr, queue.GetFullBuffer());
	EXPECT_EQ(nullptr, queue.GetFinishedBuffer());

	EXPECT_NE(nullptr, queue.GetEventReadPossibleHandle());
	EXPECT_NE(nullptr, queue.GetEventWritePossibleHandle());
	EXPECT_NE(nullptr, queue.GetEventWriteFinishedHandle());

	DWORD dwResult = WaitForSingleObject(queue.GetEventReadPossibleHandle(), 0);
	EXPECT_EQ(WAIT_OBJECT_0 + 0, dwResult);

	dwResult = WaitForSingleObject(queue.GetEventWritePossibleHandle(), 0);
	EXPECT_EQ(WAIT_TIMEOUT, dwResult);

	dwResult = WaitForSingleObject(queue.GetEventWriteFinishedHandle(), 0);
	EXPECT_EQ(WAIT_TIMEOUT, dwResult);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
TEST(TOverlappedDataBufferQueueTests, GetEmptyBuffer)
{
	TOverlappedDataBufferQueue queue(3, 32768);

	DWORD dwResult = WaitForSingleObject(queue.GetEventReadPossibleHandle(), 0);
	EXPECT_EQ(WAIT_OBJECT_0 + 0, dwResult);

	EXPECT_NE(nullptr, queue.GetEmptyBuffer());
	dwResult = WaitForSingleObject(queue.GetEventReadPossibleHandle(), 0);
	EXPECT_EQ(WAIT_OBJECT_0 + 0, dwResult);

	EXPECT_NE(nullptr, queue.GetEmptyBuffer());
	dwResult = WaitForSingleObject(queue.GetEventReadPossibleHandle(), 0);
	EXPECT_EQ(WAIT_OBJECT_0 + 0, dwResult);

	EXPECT_NE(nullptr, queue.GetEmptyBuffer());
	dwResult = WaitForSingleObject(queue.GetEventReadPossibleHandle(), 0);
	EXPECT_EQ(WAIT_TIMEOUT, dwResult);

	EXPECT_EQ(nullptr, queue.GetEmptyBuffer());
}

TEST(TOverlappedDataBufferQueueTests, AddEmptyBuffer)
{
	TOverlappedDataBufferQueue queue(3, 32768);

	TOverlappedDataBuffer* pBuffers[3] = { queue.GetEmptyBuffer(), queue.GetEmptyBuffer(), queue.GetEmptyBuffer() };

	DWORD dwResult = WaitForSingleObject(queue.GetEventReadPossibleHandle(), 0);
	EXPECT_EQ(WAIT_TIMEOUT, dwResult);

	queue.AddEmptyBuffer(pBuffers[0]);
	dwResult = WaitForSingleObject(queue.GetEventReadPossibleHandle(), 0);
	EXPECT_EQ(WAIT_OBJECT_0 + 0, dwResult);

	queue.AddEmptyBuffer(pBuffers[1]);
	dwResult = WaitForSingleObject(queue.GetEventReadPossibleHandle(), 0);
	EXPECT_EQ(WAIT_OBJECT_0 + 0, dwResult);

	queue.AddEmptyBuffer(pBuffers[2]);
	dwResult = WaitForSingleObject(queue.GetEventReadPossibleHandle(), 0);
	EXPECT_EQ(WAIT_OBJECT_0 + 0, dwResult);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
TEST(TOverlappedDataBufferQueueTests, GetFullBuffer_AddFullBuffer)
{
	TOverlappedDataBufferQueue queue(3, 32768);
	TOverlappedDataBuffer* pBuffer = queue.GetEmptyBuffer();

	pBuffer->InitForRead(0, 1280);
	pBuffer->SetBytesTransferred(1230);
	pBuffer->SetStatusCode(0);

	queue.AddFullBuffer(pBuffer);
	DWORD dwResult = WaitForSingleObject(queue.GetEventWritePossibleHandle(), 0);
	EXPECT_EQ(WAIT_OBJECT_0 + 0, dwResult);

	pBuffer = queue.GetFullBuffer();

	dwResult = WaitForSingleObject(queue.GetEventWritePossibleHandle(), 0);
	EXPECT_EQ(WAIT_TIMEOUT, dwResult);
}

TEST(TOverlappedDataBufferQueueTests, GetFullBuffer_AddFullBuffer_OutOfOrder)
{
	TOverlappedDataBufferQueue queue(3, 32768);
	TOverlappedDataBuffer* pBuffers[3] = { queue.GetEmptyBuffer(), queue.GetEmptyBuffer(), queue.GetEmptyBuffer() };

	pBuffers[0]->InitForRead(0, 1000);
	pBuffers[0]->SetBytesTransferred(1000);
	pBuffers[0]->SetStatusCode(0);

	pBuffers[1]->InitForRead(0, 1200);
	pBuffers[1]->SetBytesTransferred(1200);
	pBuffers[1]->SetStatusCode(0);

	pBuffers[2]->InitForRead(0, 1400);
	pBuffers[2]->SetBytesTransferred(800);
	pBuffers[2]->SetStatusCode(0);
	pBuffers[2]->SetLastPart(true);

	EXPECT_TIMEOUT(queue.GetEventWritePossibleHandle());

	queue.AddFullBuffer(pBuffers[1]);
	EXPECT_TIMEOUT(queue.GetEventWritePossibleHandle());

	queue.AddFullBuffer(pBuffers[2]);
	EXPECT_TIMEOUT(queue.GetEventWritePossibleHandle());

	queue.AddFullBuffer(pBuffers[0]);
	EXPECT_SIGNALED(queue.GetEventWritePossibleHandle());
}

///////////////////////////////////////////////////////////////////////////////////////////////////
TEST(TOverlappedDataBufferQueueTests, GetFinishedBuffer_AddFinishedBuffer_OutOfOrder)
{
	TOverlappedDataBufferQueue queue(3, 32768);
	TOverlappedDataBuffer* pBuffers[3] = { queue.GetEmptyBuffer(), queue.GetEmptyBuffer(), queue.GetEmptyBuffer() };

	pBuffers[0]->InitForRead(0, 1000);
	pBuffers[0]->SetBytesTransferred(1000);
	pBuffers[0]->SetStatusCode(0);

	pBuffers[1]->InitForRead(0, 1200);
	pBuffers[1]->SetBytesTransferred(1200);
	pBuffers[1]->SetStatusCode(0);

	pBuffers[2]->InitForRead(0, 1400);
	pBuffers[2]->SetBytesTransferred(800);
	pBuffers[2]->SetStatusCode(0);
	pBuffers[2]->SetLastPart(true);

	queue.AddFinishedBuffer(pBuffers[1]);
	EXPECT_TIMEOUT(queue.GetEventWriteFinishedHandle());
	queue.AddFinishedBuffer(pBuffers[2]);
	EXPECT_TIMEOUT(queue.GetEventWriteFinishedHandle());
	queue.AddFinishedBuffer(pBuffers[0]);
	EXPECT_SIGNALED(queue.GetEventWriteFinishedHandle());
}
