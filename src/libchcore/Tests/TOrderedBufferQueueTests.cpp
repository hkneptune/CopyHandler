#include "stdafx.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../TOrderedBufferQueue.h"
#include "../GTestMacros.h"
#include "../TCoreException.h"

using namespace chcore;

///////////////////////////////////////////////////////////////////////////////////////////
// expected position mode
TEST(TOrderedBufferQueueTests, ExpectedPos_ConstructionSanityTest)
{
	TOrderedBufferQueue queue(0);

	EXPECT_EQ(0, queue.GetCount());
	EXPECT_EQ(true, queue.IsEmpty());
	EXPECT_TIMEOUT(queue.GetHasBuffersEvent());
	EXPECT_EQ(nullptr, queue.Peek());
}

TEST(TOrderedBufferQueueTests, ExpectedPos_PushAtExpectedPosition)
{
	TOrderedBufferQueue queue(0);
	TOverlappedDataBuffer buffer(1024, nullptr);
	buffer.SetFilePosition(0);

	queue.Push(&buffer);

	EXPECT_EQ(1, queue.GetCount());
	EXPECT_EQ(false, queue.IsEmpty());
	EXPECT_SIGNALED(queue.GetHasBuffersEvent());
	EXPECT_EQ(&buffer, queue.Peek());
}

TEST(TOrderedBufferQueueTests, ExpectedPos_PushAtOtherPosition)
{
	TOrderedBufferQueue queue(0);
	TOverlappedDataBuffer buffer(1024, nullptr);
	buffer.SetFilePosition(1000);

	queue.Push(&buffer);

	EXPECT_EQ(1, queue.GetCount());
	EXPECT_EQ(false, queue.IsEmpty());
	EXPECT_TIMEOUT(queue.GetHasBuffersEvent());
	EXPECT_EQ(&buffer, queue.Peek());
	EXPECT_EQ(nullptr, queue.Pop());
}

TEST(TOrderedBufferQueueTests, ExpectedPos_PushOutOfOrder)
{
	TOrderedBufferQueue queue(0);
	TOverlappedDataBuffer buffer1(1024, nullptr);
	buffer1.SetFilePosition(1000);
	buffer1.SetRequestedDataSize(1000);
	TOverlappedDataBuffer buffer2(1024, nullptr);
	buffer2.SetFilePosition(0);
	buffer2.SetRequestedDataSize(1000);

	queue.Push(&buffer1);
	queue.Push(&buffer2);

	EXPECT_EQ(2, queue.GetCount());
	EXPECT_EQ(false, queue.IsEmpty());
	EXPECT_SIGNALED(queue.GetHasBuffersEvent());
	EXPECT_EQ(&buffer2, queue.Peek());
	EXPECT_EQ(&buffer2, queue.Pop());
	EXPECT_EQ(&buffer1, queue.Pop());
}

TEST(TOrderedBufferQueueTests, ExpectedPos_Pop)
{
	TOrderedBufferQueue queue(0);
	TOverlappedDataBuffer buffer(1024, nullptr);

	queue.Push(&buffer);
	EXPECT_EQ(&buffer, queue.Pop());

	EXPECT_EQ(0, queue.GetCount());
	EXPECT_EQ(true, queue.IsEmpty());
	EXPECT_TIMEOUT(queue.GetHasBuffersEvent());
	EXPECT_EQ(nullptr, queue.Peek());
}

TEST(TOrderedBufferQueueTests, ExpectedPos_ReleaseBuffers)
{
	TOrderedBufferQueue queue(0);
	TBufferListPtr spReleaseList(std::make_shared<TBufferList>());
	TOverlappedDataBuffer buffer(1024, nullptr);

	queue.Push(&buffer);
	queue.ReleaseBuffers(spReleaseList);

	EXPECT_EQ(1, spReleaseList->GetCount());
}

TEST(TOrderedBufferQueueTests, ExpectedPos_ReleaseBuffersUnordered)
{
	TOrderedBufferQueue queue(0);
	TBufferListPtr spReleaseList(std::make_shared<TBufferList>());
	TOverlappedDataBuffer buffer(1024, nullptr);
	buffer.SetFilePosition(1000);

	queue.Push(&buffer);
	queue.ReleaseBuffers(spReleaseList);

	EXPECT_EQ(1, spReleaseList->GetCount());
}
