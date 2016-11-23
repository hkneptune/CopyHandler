#include "stdafx.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../TSimpleOrderedBufferQueue.h"
#include "../TBufferList.h"

using namespace chcore;

// no expected position mode
TEST(TSimpleOrderedBufferQueueTests, ConstructionSanityTest)
{
	TSimpleOrderedBufferQueue queue;

	EXPECT_EQ(0, queue.size());
	EXPECT_EQ(true, queue.empty());
	EXPECT_EQ(nullptr, queue.Peek());
}

TEST(TSimpleOrderedBufferQueueTests, Push)
{
	TSimpleOrderedBufferQueue queue;
	TOverlappedDataBuffer buffer(1024, nullptr);

	queue.Push(&buffer);

	EXPECT_EQ(1, queue.size());
	EXPECT_EQ(false, queue.empty());
	EXPECT_EQ(&buffer, queue.Peek());
}

TEST(TSimpleOrderedBufferQueueTests, PushDuplicate)
{
	TSimpleOrderedBufferQueue queue;
	TOverlappedDataBuffer buffer(1024, nullptr);

	queue.Push(&buffer);
	EXPECT_THROW(queue.Push(&buffer), TCoreException);
}

TEST(TSimpleOrderedBufferQueueTests, Pop)
{
	TSimpleOrderedBufferQueue queue;
	TOverlappedDataBuffer buffer(1024, nullptr);

	queue.Push(&buffer);
	EXPECT_EQ(&buffer, queue.Pop());

	EXPECT_EQ(0, queue.size());
	EXPECT_EQ(true, queue.empty());
	EXPECT_EQ(nullptr, queue.Peek());
}

TEST(TSimpleOrderedBufferQueueTests, Clear)
{
	TSimpleOrderedBufferQueue queue;
	TOverlappedDataBuffer buffer(1024, nullptr);

	queue.Push(&buffer);
	queue.clear();

	EXPECT_EQ(0, queue.size());
	EXPECT_EQ(true, queue.empty());
	EXPECT_EQ(nullptr, queue.Peek());
}

TEST(TSimpleOrderedBufferQueueTests, ReleaseBuffers)
{
	TSimpleOrderedBufferQueue queue;
	TBufferListPtr spReleaseList(std::make_shared<TBufferList>());
	TOverlappedDataBuffer buffer(1024, nullptr);

	queue.Push(&buffer);
	queue.ClearBuffers(spReleaseList);

	EXPECT_EQ(1, spReleaseList->GetCount());
}
