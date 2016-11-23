#include "stdafx.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../TOrderedBufferQueue.h"
#include "../GTestMacros.h"
#include "../TCoreException.h"

class FallbackCollection : public std::vector<chcore::TOverlappedDataBuffer*>
{
public:
	void Push(chcore::TOverlappedDataBuffer* pBuffer)
	{
		push_back(pBuffer);
	}
};

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
	queue.ClearBuffers(spReleaseList);

	EXPECT_EQ(1, spReleaseList->GetCount());
}

TEST(TOrderedBufferQueueTests, ExpectedPos_ReleaseBuffersUnordered)
{
	TOrderedBufferQueue queue(0);
	TBufferListPtr spReleaseList(std::make_shared<TBufferList>());
	TOverlappedDataBuffer buffer(1024, nullptr);
	buffer.SetFilePosition(1000);

	queue.Push(&buffer);
	queue.ClearBuffers(spReleaseList);

	EXPECT_EQ(1, spReleaseList->GetCount());
}

///////////////////////////////////////////////////////////////////////////////////////////
// failed buffers

///////////////////////////////////////////////////////////////////////////////
// Construction tests

TEST(TOrderedBufferQueueTests, ConstructionSanityTest)
{
	TOrderedBufferQueue queue(0);

	EXPECT_EQ(0, queue.GetCount());
	EXPECT_TIMEOUT(queue.GetHasErrorEvent());
	EXPECT_EQ(true, queue.IsEmpty());
}

///////////////////////////////////////////////////////////////////////////////
// PushBuffer tests

TEST(TOrderedBufferQueueTests, PushBuffer_FirstFailure)
{
	TOrderedBufferQueue queue(0);
	TOverlappedDataBuffer buffer(4096, nullptr);
	buffer.SetErrorCode(123);

	FallbackCollection collection;

	queue.PushError(&buffer, collection);
	EXPECT_EQ(1, queue.GetCount());
	EXPECT_SIGNALED(queue.GetHasErrorEvent());
	EXPECT_EQ(true, collection.empty());
}

TEST(TOrderedBufferQueueTests, PushBuffer_TwoSubsequentFailures)
{
	TOrderedBufferQueue queue(0);
	TOverlappedDataBuffer buffer1(4096, nullptr);
	TOverlappedDataBuffer buffer2(4096, nullptr);
	buffer1.SetFilePosition(0);
	buffer1.SetErrorCode(123);
	buffer2.SetFilePosition(1000);
	buffer2.SetErrorCode(234);

	FallbackCollection collection;

	queue.PushError(&buffer1, collection);
	queue.PushError(&buffer2, collection);

	EXPECT_EQ(1, queue.GetCount());
	EXPECT_SIGNALED(queue.GetHasErrorEvent());
	EXPECT_EQ(1, collection.size());
	EXPECT_EQ(&buffer2, collection.front());
	EXPECT_EQ(1000, collection.front()->GetFilePosition());
	EXPECT_EQ(0, collection.front()->GetErrorCode());
}

TEST(TOrderedBufferQueueTests, PushBuffer_TwoFailuresOutOfOrder)
{
	TOrderedBufferQueue queue(0);
	TOverlappedDataBuffer buffer1(4096, nullptr);
	TOverlappedDataBuffer buffer2(4096, nullptr);
	buffer1.SetFilePosition(0);
	buffer1.SetErrorCode(123);
	buffer2.SetFilePosition(1000);
	buffer2.SetErrorCode(234);

	FallbackCollection collection;

	queue.PushError(&buffer2, collection);
	queue.PushError(&buffer1, collection);

	EXPECT_EQ(1, queue.GetCount());
	EXPECT_SIGNALED(queue.GetHasErrorEvent());
	EXPECT_EQ(1, collection.size());
	EXPECT_EQ(&buffer2, collection.front());
	EXPECT_EQ(1000, collection.front()->GetFilePosition());
	EXPECT_EQ(0, collection.front()->GetErrorCode());
}

TEST(TOrderedBufferQueueTests, PushBuffer_ThrowOnNonErrorBuffer)
{
	TOrderedBufferQueue queue(0);
	TOverlappedDataBuffer buffer1(4096, nullptr);
	buffer1.SetFilePosition(0);
	buffer1.SetErrorCode(ERROR_SUCCESS);

	FallbackCollection collection;

	EXPECT_THROW(queue.PushError(&buffer1, collection), TCoreException);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Pop tests

TEST(TOrderedBufferQueueTests, PopBuffer_EmptyContainer)
{
	TOrderedBufferQueue queue(0);
	EXPECT_EQ(nullptr, queue.Pop());
}

TEST(TOrderedBufferQueueTests, PopBuffer_WithSamePosition)
{
	TOrderedBufferQueue queue(0);
	TOverlappedDataBuffer buffer1(4096, nullptr);
	TOverlappedDataBuffer buffer2(4096, nullptr);
	TOverlappedDataBuffer buffer3(4096, nullptr);
	buffer1.SetFilePosition(0);
	buffer1.SetErrorCode(123);
	buffer2.SetFilePosition(1000);
	buffer2.SetErrorCode(234);

	FallbackCollection collection;

	queue.PushError(&buffer1, collection);
	queue.PopError();

	EXPECT_EQ(0, collection.size());
	EXPECT_TIMEOUT(queue.GetHasErrorEvent());

	queue.PushError(&buffer2, collection);
	EXPECT_EQ(0, queue.GetCount());
	EXPECT_EQ(1, collection.size());
}
