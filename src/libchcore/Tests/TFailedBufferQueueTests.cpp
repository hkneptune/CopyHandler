#include "stdafx.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../TFailedBufferQueue.h"
#include "../GTestMacros.h"
#include "../TOverlappedDataBuffer.h"
#include <memory>

using namespace chcore;

class FallbackCollection : public std::vector<TOverlappedDataBuffer*>
{
public:
	void Push(TOverlappedDataBuffer* pBuffer, bool /*bKeepPos*/)
	{
		push_back(pBuffer);
	}
};

///////////////////////////////////////////////////////////////////////////////
// Construction tests

TEST(TFailedBufferQueueTests, ConstructionSanityTest)
{
	TFailedBufferQueue queue;

	EXPECT_EQ(0, queue.GetCount());
	EXPECT_TIMEOUT(queue.GetHasBuffersEvent());
	EXPECT_EQ(true, queue.IsEmpty());
}

///////////////////////////////////////////////////////////////////////////////
// PushBuffer tests

TEST(TFailedBufferQueueTests, PushBuffer_FirstFailure)
{
	TFailedBufferQueue queue;
	TOverlappedDataBuffer buffer(4096, nullptr);
	buffer.SetErrorCode(123);

	FallbackCollection collection;

	queue.PushWithFallback(&buffer, collection);
	EXPECT_EQ(false, queue.IsEmpty());
	EXPECT_EQ(1, queue.GetCount());
	EXPECT_SIGNALED(queue.GetHasBuffersEvent());
	EXPECT_EQ(true, collection.empty());
}

TEST(TFailedBufferQueueTests, PushBuffer_TwoSubsequentFailures)
{
	TFailedBufferQueue queue;
	TOverlappedDataBuffer buffer1(4096, nullptr);
	TOverlappedDataBuffer buffer2(4096, nullptr);
	buffer1.SetFilePosition(0);
	buffer1.SetErrorCode(123);
	buffer2.SetFilePosition(1000);
	buffer2.SetErrorCode(234);

	FallbackCollection collection;

	queue.PushWithFallback(&buffer1, collection);
	queue.PushWithFallback(&buffer2, collection);

	EXPECT_EQ(false, queue.IsEmpty());
	EXPECT_EQ(1, queue.GetCount());
	EXPECT_SIGNALED(queue.GetHasBuffersEvent());
	EXPECT_EQ(1, collection.size());
	EXPECT_EQ(&buffer2, collection.front());
	EXPECT_EQ(1000, collection.front()->GetFilePosition());
	EXPECT_EQ(234, collection.front()->GetErrorCode());
}

TEST(TFailedBufferQueueTests, PushBuffer_TwoFailuresOutOfOrder)
{
	TFailedBufferQueue queue;
	TOverlappedDataBuffer buffer1(4096, nullptr);
	TOverlappedDataBuffer buffer2(4096, nullptr);
	buffer1.SetFilePosition(0);
	buffer1.SetErrorCode(123);
	buffer2.SetFilePosition(1000);
	buffer2.SetErrorCode(234);

	FallbackCollection collection;

	queue.PushWithFallback(&buffer2, collection);
	queue.PushWithFallback(&buffer1, collection);

	EXPECT_EQ(false, queue.IsEmpty());
	EXPECT_EQ(1, queue.GetCount());
	EXPECT_SIGNALED(queue.GetHasBuffersEvent());
	EXPECT_EQ(1, collection.size());
	EXPECT_EQ(&buffer2, collection.front());
	EXPECT_EQ(1000, collection.front()->GetFilePosition());
	EXPECT_EQ(234, collection.front()->GetErrorCode());
}

TEST(TFailedBufferQueueTests, PushBuffer_ThrowOnNonErrorBuffer)
{
	TFailedBufferQueue queue;
	TOverlappedDataBuffer buffer1(4096, nullptr);
	buffer1.SetFilePosition(0);
	buffer1.SetErrorCode(ERROR_SUCCESS);

	FallbackCollection collection;

	EXPECT_THROW(queue.PushWithFallback(&buffer1, collection), TCoreException);
}

TEST(TFailedBufferQueueTests, PushBuffer_WithSamePosition)
{
	TFailedBufferQueue queue;
	TOverlappedDataBuffer buffer1(4096, nullptr);
	TOverlappedDataBuffer buffer2(4096, nullptr);
	buffer1.SetFilePosition(1000);
	buffer1.SetErrorCode(123);
	buffer2.SetFilePosition(1000);
	buffer2.SetErrorCode(234);

	FallbackCollection collection;

	queue.PushWithFallback(&buffer1, collection);
	EXPECT_THROW(queue.PushWithFallback(&buffer2, collection), TCoreException);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Pop tests

TEST(TFailedBufferQueueTests, PopBuffer_EmptyContainer)
{
	TFailedBufferQueue queue;
	EXPECT_EQ(nullptr, queue.Pop());
}

TEST(TFailedBufferQueueTests, PopBuffer_WithSamePosition)
{
	TFailedBufferQueue queue;
	TOverlappedDataBuffer buffer1(4096, nullptr);
	TOverlappedDataBuffer buffer2(4096, nullptr);
	TOverlappedDataBuffer buffer3(4096, nullptr);
	buffer1.SetFilePosition(0);
	buffer1.SetErrorCode(123);
	buffer2.SetFilePosition(1000);
	buffer2.SetErrorCode(234);

	FallbackCollection collection;

	queue.PushWithFallback(&buffer1, collection);
	queue.Pop();

	EXPECT_EQ(0, collection.size());
	EXPECT_TIMEOUT(queue.GetHasBuffersEvent());

	queue.PushWithFallback(&buffer2, collection);
	EXPECT_EQ(1, queue.GetCount());
}

///////////////////////////////////////////////////////////////////////////////////////////
// Clear tests

TEST(TFailedBufferQueueTests, Clear)
{
	TFailedBufferQueue queue;
	TOverlappedDataBuffer buffer1(4096, nullptr);
	buffer1.SetFilePosition(0);
	buffer1.SetErrorCode(123);

	FallbackCollection collection;

	queue.PushWithFallback(&buffer1, collection);
	queue.Clear();

	EXPECT_EQ(0, queue.GetCount());
}

//////////////////////////////////////////////////////////////////////////////////////////
// ReleaseBuffers

TEST(TFailedBufferQueueTests, ReleaseBuffers)
{
	TFailedBufferQueue queue;
	TOverlappedDataBuffer buffer1(4096, nullptr);
	buffer1.SetFilePosition(0);
	buffer1.SetErrorCode(123);

	FallbackCollection collection;

	queue.PushWithFallback(&buffer1, collection);

	TBufferListPtr spReturnedBuffers(std::make_shared<TBufferList>());
	queue.ReleaseBuffers(spReturnedBuffers);

	EXPECT_EQ(1, spReturnedBuffers->GetCount());
}
