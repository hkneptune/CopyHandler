#include "stdafx.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../WaitableQueue.h"
#include "../../common/GTestMacros.h"

using namespace chengine;

TEST(TestsWaitableQueue, DefaultConstruction)
{
	WaitableQueue<int> queue;
	EXPECT_TRUE(queue.GetWaitHandle() != INVALID_HANDLE_VALUE);
	EXPECT_TIMEOUT(queue.GetWaitHandle());
}

TEST(TestsWaitableQueue, PopFrom_EmptyContainer)
{
	WaitableQueue<int> queue;
	EXPECT_THROW(queue.PopFront(), std::exception);
}

TEST(TestsWaitableQueue, Push)
{
	WaitableQueue<int> queue;
	queue.PushBack(5);
	EXPECT_SIGNALED(queue.GetWaitHandle());
	EXPECT_EQ(5, queue.PopFront());
}
