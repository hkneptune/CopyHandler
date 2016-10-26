#include "stdafx.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../TOrderedBufferQueue.h"
#include "../GTestMacros.h"
#include "../TCoreException.h"

using namespace chcore;

// no expected position mode
TEST(TOrderedBufferQueueTests, NoExpectedPos_ConstructionSanityTest)
{
	TOrderedBufferQueue queue;

	EXPECT_EQ(0, queue.GetCount());
	EXPECT_EQ(true, queue.IsEmpty());
	EXPECT_TIMEOUT(queue.GetHasBuffersEvent());
	EXPECT_EQ(nullptr, queue.Peek());
}

TEST(TOrderedBufferQueueTests, NoExpectedPos_Push)
{
	TOrderedBufferQueue queue;
	TOverlappedDataBuffer buffer(1024, nullptr);

	queue.Push(&buffer);

	EXPECT_EQ(1, queue.GetCount());
	EXPECT_EQ(false, queue.IsEmpty());
	EXPECT_SIGNALED(queue.GetHasBuffersEvent());
	EXPECT_EQ(&buffer, queue.Peek());
}

TEST(TOrderedBufferQueueTests, NoExpectedPos_Pop)
{
	TOrderedBufferQueue queue;
	TOverlappedDataBuffer buffer(1024, nullptr);

	queue.Push(&buffer);
	EXPECT_EQ(&buffer, queue.Pop());

	EXPECT_EQ(0, queue.GetCount());
	EXPECT_EQ(true, queue.IsEmpty());
	EXPECT_TIMEOUT(queue.GetHasBuffersEvent());
	EXPECT_EQ(nullptr, queue.Peek());
}

TEST(TOrderedBufferQueueTests, NoExpectedPos_Clear)
{
	TOrderedBufferQueue queue;
	TOverlappedDataBuffer buffer(1024, nullptr);

	queue.Push(&buffer);
	queue.Clear();

	EXPECT_EQ(0, queue.GetCount());
	EXPECT_EQ(true, queue.IsEmpty());
	EXPECT_TIMEOUT(queue.GetHasBuffersEvent());
	EXPECT_EQ(nullptr, queue.Peek());
}

TEST(TOrderedBufferQueueTests, NoExpectedPos_ReleaseBuffers)
{
	TOrderedBufferQueue queue;
	TBufferListPtr spReleaseList(std::make_shared<TBufferList>());
	TOverlappedDataBuffer buffer(1024, nullptr);

	queue.Push(&buffer);
	queue.ReleaseBuffers(spReleaseList);

	EXPECT_EQ(1, spReleaseList->GetCount());
}

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

TEST(TOrderedBufferQueueTests, ExpectedPos_Clear)
{
	TOrderedBufferQueue queue(0);
	TOverlappedDataBuffer buffer(1024, nullptr);

	queue.Push(&buffer);
	queue.Clear();

	EXPECT_EQ(0, queue.GetCount());
	EXPECT_EQ(true, queue.IsEmpty());
	EXPECT_TIMEOUT(queue.GetHasBuffersEvent());
	EXPECT_EQ(nullptr, queue.Peek());
}

TEST(TOrderedBufferQueueTests, ExpectedPos_ClearUnordered)
{
	TOrderedBufferQueue queue(0);
	TOverlappedDataBuffer buffer1(1024, nullptr);
	buffer1.SetFilePosition(1000);
	buffer1.SetRequestedDataSize(1000);

	queue.Push(&buffer1);

	queue.Clear();

	EXPECT_EQ(0, queue.GetCount());
	EXPECT_EQ(true, queue.IsEmpty());
	EXPECT_TIMEOUT(queue.GetHasBuffersEvent());
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

/////////////////////////////////////////////////////////////////////////
// custom, specialized functionality

TEST(TOrderedBufferQueueTests, GetUnneededLastParts_NoLastParts)
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

	auto vParts = queue.GetUnneededLastParts();

	EXPECT_EQ(0, vParts.size());
}

TEST(TOrderedBufferQueueTests, GetUnneededLastParts_SingleLastPart)
{
	TOrderedBufferQueue queue(0);
	TOverlappedDataBuffer buffer1(1024, nullptr);
	buffer1.SetFilePosition(1000);
	buffer1.SetRequestedDataSize(1000);
	buffer1.SetLastPart(true);
	TOverlappedDataBuffer buffer2(1024, nullptr);
	buffer2.SetFilePosition(0);
	buffer2.SetRequestedDataSize(1000);

	queue.Push(&buffer1);
	queue.Push(&buffer2);

	auto vParts = queue.GetUnneededLastParts();

	EXPECT_EQ(0, vParts.size());
}

TEST(TOrderedBufferQueueTests, GetUnneededLastParts_UnfinishedBufferAfterFinished)
{
	TOrderedBufferQueue queue(0);
	TOverlappedDataBuffer buffer1(1024, nullptr);
	buffer1.SetFilePosition(1000);
	buffer1.SetRequestedDataSize(1000);
	TOverlappedDataBuffer buffer2(1024, nullptr);
	buffer2.SetFilePosition(0);
	buffer2.SetRequestedDataSize(1000);
	buffer2.SetLastPart(true);

	queue.Push(&buffer1);
	queue.Push(&buffer2);

	EXPECT_THROW(queue.GetUnneededLastParts(), TCoreException);
}

TEST(TOrderedBufferQueueTests, GetUnneededLastParts_TwoLastParts)
{
	TOrderedBufferQueue queue(0);
	TOverlappedDataBuffer buffer1(1024, nullptr);
	buffer1.SetFilePosition(1000);
	buffer1.SetRequestedDataSize(1000);
	buffer1.SetLastPart(true);
	TOverlappedDataBuffer buffer2(1024, nullptr);
	buffer2.SetFilePosition(0);
	buffer2.SetRequestedDataSize(1000);
	buffer2.SetLastPart(true);

	queue.Push(&buffer1);
	queue.Push(&buffer2);

	auto vParts = queue.GetUnneededLastParts();

	EXPECT_EQ(1, vParts.size());
	EXPECT_EQ(&buffer1, vParts[0]);
}
