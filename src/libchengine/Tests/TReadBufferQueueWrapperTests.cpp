#include "stdafx.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../TReadBufferQueueWrapper.h"
#include <memory>
#include "../../common/GTestMacros.h"

using namespace chengine;
using namespace chcore;

TEST(TReadBufferQueueWrapperTests, ConstructorWithZeroChunkSize)
{
	TBufferListPtr spList(std::make_shared<TBufferList>());

	EXPECT_THROW(TReadBufferQueueWrapper(spList, 0, 0, 0, 0, nullptr, nullptr), TCoreException);
}

TEST(TReadBufferQueueWrapperTests, Constructor)
{
	TBufferListPtr spList(std::make_shared<TBufferList>());

	TSharedCountPtr<size_t> spOtfBufferCount(std::make_shared<TSharedCount<size_t>>());
	TSharedCountMTPtr<size_t> spReadAheadBufferCount(std::make_shared<TSharedCountMT<size_t>>());

	TReadBufferQueueWrapper queue(spList, 0, 1024, 1, 1, spOtfBufferCount, spReadAheadBufferCount);
	EXPECT_SIGNALED(queue.GetHasBuffersEvent());
	EXPECT_EQ(false, queue.IsDataSourceFinished());
}

TEST(TReadBufferQueueWrapperTests, Pop_EmptyQueue)
{
	TBufferListPtr spList(std::make_shared<TBufferList>());

	TSharedCountPtr<size_t> spOtfBufferCount(std::make_shared<TSharedCount<size_t>>());
	TSharedCountMTPtr<size_t> spReadAheadBufferCount(std::make_shared<TSharedCountMT<size_t>>());

	TReadBufferQueueWrapper queue(spList, 0, 1024, 1, 1, spOtfBufferCount, spReadAheadBufferCount);

	EXPECT_EQ(nullptr, queue.Pop());
}

TEST(TReadBufferQueueWrapperTests, Pop_FromBufferList)
{
	TBufferListPtr spList(std::make_shared<TBufferList>());
	TOverlappedDataBuffer buffer1(1024, nullptr);
	TOverlappedDataBuffer buffer2(1024, nullptr);
	TOverlappedDataBuffer buffer3(1024, nullptr);
	TOverlappedDataBuffer buffer4(1024, nullptr);
	spList->Push(&buffer1);
	spList->Push(&buffer2);
	spList->Push(&buffer3);
	spList->Push(&buffer4);

	TSharedCountPtr<size_t> spOtfBufferCount(std::make_shared<TSharedCount<size_t>>());
	TSharedCountMTPtr<size_t> spReadAheadBufferCount(std::make_shared<TSharedCountMT<size_t>>());

	TReadBufferQueueWrapper queue(spList, 0, 1024, 1, 1, spOtfBufferCount, spReadAheadBufferCount);

	EXPECT_EQ(&buffer4, queue.Pop());
	EXPECT_EQ(0, buffer4.GetFilePosition());
	EXPECT_EQ(1024, buffer4.GetRequestedDataSize());

	EXPECT_EQ(&buffer3, queue.Pop());
	EXPECT_EQ(1024, buffer3.GetFilePosition());
	EXPECT_EQ(1024, buffer3.GetRequestedDataSize());

	EXPECT_EQ(&buffer2, queue.Pop());
	EXPECT_EQ(2048, buffer2.GetFilePosition());
	EXPECT_EQ(1024, buffer2.GetRequestedDataSize());

	EXPECT_EQ(&buffer1, queue.Pop());
	EXPECT_EQ(3072, buffer1.GetFilePosition());
	EXPECT_EQ(1024, buffer1.GetRequestedDataSize());

	EXPECT_EQ(nullptr, queue.Pop());
}

TEST(TReadBufferQueueWrapperTests, PushPop_ClaimedBuffers)
{
	TBufferListPtr spList(std::make_shared<TBufferList>());
	TSharedCountPtr<size_t> spOtfBufferCount(std::make_shared<TSharedCount<size_t>>());
	TSharedCountMTPtr<size_t> spReadAheadBufferCount(std::make_shared<TSharedCountMT<size_t>>());

	TReadBufferQueueWrapper queue(spList, 0, 1024, 1, 1, spOtfBufferCount, spReadAheadBufferCount);

	TOverlappedDataBuffer buffer1(1024, nullptr);
	TOverlappedDataBuffer buffer2(1024, nullptr);
	TOverlappedDataBuffer buffer3(1024, nullptr);
	TOverlappedDataBuffer buffer4(1024, nullptr);
	queue.PushEmpty(&buffer1);
	queue.PushEmpty(&buffer2);
	queue.PushEmpty(&buffer3);
	queue.PushEmpty(&buffer4);

	EXPECT_SIGNALED(queue.GetHasBuffersEvent());
	EXPECT_EQ(&buffer4, queue.Pop());
	EXPECT_EQ(0, buffer4.GetFilePosition());
	EXPECT_EQ(1024, buffer4.GetRequestedDataSize());

	EXPECT_SIGNALED(queue.GetHasBuffersEvent());
	EXPECT_EQ(&buffer3, queue.Pop());
	EXPECT_EQ(1024, buffer3.GetFilePosition());
	EXPECT_EQ(1024, buffer3.GetRequestedDataSize());

	EXPECT_SIGNALED(queue.GetHasBuffersEvent());
	EXPECT_EQ(&buffer2, queue.Pop());
	EXPECT_EQ(2048, buffer2.GetFilePosition());
	EXPECT_EQ(1024, buffer2.GetRequestedDataSize());

	EXPECT_SIGNALED(queue.GetHasBuffersEvent());
	EXPECT_EQ(&buffer1, queue.Pop());
	EXPECT_EQ(3072, buffer1.GetFilePosition());
	EXPECT_EQ(1024, buffer1.GetRequestedDataSize());

	EXPECT_SIGNALED(queue.GetHasBuffersEvent());
	EXPECT_EQ(nullptr, queue.Pop());
	EXPECT_TIMEOUT(queue.GetHasBuffersEvent());
}

TEST(TReadBufferQueueWrapperTests, PushPop_MixedBuffers)
{
	TBufferListPtr spList(std::make_shared<TBufferList>());
	TOverlappedDataBuffer buffer1(1024, nullptr);
	TOverlappedDataBuffer buffer2(1024, nullptr);
	spList->Push(&buffer1);
	spList->Push(&buffer2);

	TSharedCountPtr<size_t> spOtfBufferCount(std::make_shared<TSharedCount<size_t>>());
	TSharedCountMTPtr<size_t> spReadAheadBufferCount(std::make_shared<TSharedCountMT<size_t>>());

	TReadBufferQueueWrapper queue(spList, 0, 1024, 1, 1, spOtfBufferCount, spReadAheadBufferCount);

	TOverlappedDataBuffer buffer3(1024, nullptr);
	TOverlappedDataBuffer buffer4(1024, nullptr);
	queue.PushEmpty(&buffer3);
	queue.PushEmpty(&buffer4);

	EXPECT_SIGNALED(queue.GetHasBuffersEvent());
	EXPECT_EQ(&buffer4, queue.Pop());
	EXPECT_EQ(0, buffer4.GetFilePosition());
	EXPECT_EQ(1024, buffer4.GetRequestedDataSize());

	EXPECT_SIGNALED(queue.GetHasBuffersEvent());
	EXPECT_EQ(&buffer3, queue.Pop());
	EXPECT_EQ(1024, buffer3.GetFilePosition());
	EXPECT_EQ(1024, buffer3.GetRequestedDataSize());

	EXPECT_SIGNALED(queue.GetHasBuffersEvent());
	EXPECT_EQ(&buffer2, queue.Pop());
	EXPECT_EQ(2048, buffer2.GetFilePosition());
	EXPECT_EQ(1024, buffer2.GetRequestedDataSize());

	EXPECT_SIGNALED(queue.GetHasBuffersEvent());
	EXPECT_EQ(&buffer1, queue.Pop());
	EXPECT_EQ(3072, buffer1.GetFilePosition());
	EXPECT_EQ(1024, buffer1.GetRequestedDataSize());

	EXPECT_SIGNALED(queue.GetHasBuffersEvent());
	EXPECT_EQ(nullptr, queue.Pop());
	EXPECT_TIMEOUT(queue.GetHasBuffersEvent());
}

/////////////////////////////////////////////////////////////////////////////////
// data source finished mode

TEST(TReadBufferQueueWrapperTests, PushPop_DataSourceFinished)
{
	TBufferListPtr spList(std::make_shared<TBufferList>());

	TOverlappedDataBuffer buffer1(1024, nullptr);
	buffer1.SetLastPart(true);

	spList->Push(&buffer1);

	TSharedCountPtr<size_t> spOtfBufferCount(std::make_shared<TSharedCount<size_t>>());
	TSharedCountMTPtr<size_t> spReadAheadBufferCount(std::make_shared<TSharedCountMT<size_t>>());

	TReadBufferQueueWrapper queue(spList, 0, 1024, 1, 1, spOtfBufferCount, spReadAheadBufferCount);
	queue.SetDataSourceFinished(&buffer1);

	EXPECT_EQ(true, queue.IsDataSourceFinished());
	EXPECT_EQ(nullptr, queue.Pop());
}

TEST(TReadBufferQueueWrapperTests, PushPop_DataSourceFinishedUsingInvalidBuffer)
{
	TBufferListPtr spList(std::make_shared<TBufferList>());

	TOverlappedDataBuffer buffer1(1024, nullptr);
	spList->Push(&buffer1);

	TSharedCountPtr<size_t> spOtfBufferCount(std::make_shared<TSharedCount<size_t>>());
	TSharedCountMTPtr<size_t> spReadAheadBufferCount(std::make_shared<TSharedCountMT<size_t>>());

	TReadBufferQueueWrapper queue(spList, 0, 1024, 1, 1, spOtfBufferCount, spReadAheadBufferCount);

	EXPECT_THROW(queue.SetDataSourceFinished(&buffer1), TCoreException);
}

TEST(TReadBufferQueueWrapperTests, PushPop_DataSourceFinished_CheckBufferMaintenance)
{
	TBufferListPtr spList(std::make_shared<TBufferList>());
	TSharedCountPtr<size_t> spOtfBufferCount(std::make_shared<TSharedCount<size_t>>());
	TSharedCountMTPtr<size_t> spReadAheadBufferCount(std::make_shared<TSharedCountMT<size_t>>());

	TReadBufferQueueWrapper queue(spList, 0, 1024, 1, 1, spOtfBufferCount, spReadAheadBufferCount);

	TOverlappedDataBuffer buffer1(1024, nullptr);
	buffer1.SetFilePosition(0);
	buffer1.SetLastPart(true);
	queue.Push(&buffer1);
	TOverlappedDataBuffer buffer2(1024, nullptr);
	buffer2.SetFilePosition(1024);
	buffer2.SetLastPart(true);
	queue.Push(&buffer2);

	queue.SetDataSourceFinished(&buffer1);

	EXPECT_EQ(&buffer1, queue.Pop());

	EXPECT_EQ(0, spList->GetCount());
	EXPECT_EQ(nullptr, spList->Pop());
}

TEST(TReadBufferQueueWrapperTests, PushPop_DataSourceFinished_ValidPushAfterFinished)
{
	TBufferListPtr spList(std::make_shared<TBufferList>());
	TSharedCountPtr<size_t> spOtfBufferCount(std::make_shared<TSharedCount<size_t>>());
	TSharedCountMTPtr<size_t> spReadAheadBufferCount(std::make_shared<TSharedCountMT<size_t>>());

	TReadBufferQueueWrapper queue(spList, 0, 1024, 1, 1, spOtfBufferCount, spReadAheadBufferCount);

	TOverlappedDataBuffer buffer1(1024, nullptr);
	buffer1.SetLastPart(true);
	queue.Push(&buffer1);

	queue.SetDataSourceFinished(&buffer1);

	EXPECT_EQ(0, spList->GetCount());

	TOverlappedDataBuffer buffer2(1024, nullptr);
	buffer2.SetLastPart(true);
	queue.Push(&buffer2);

	EXPECT_EQ(&buffer1, queue.Pop());
	EXPECT_EQ(1, spList->GetCount());
	EXPECT_EQ(&buffer2, spList->Pop());
}

TEST(TReadBufferQueueWrapperTests, PushPop_DataSourceFinished_InvalidPushAfterFinished)
{
	TBufferListPtr spList(std::make_shared<TBufferList>());
	TSharedCountPtr<size_t> spOtfBufferCount(std::make_shared<TSharedCount<size_t>>());
	TSharedCountMTPtr<size_t> spReadAheadBufferCount(std::make_shared<TSharedCountMT<size_t>>());

	TReadBufferQueueWrapper queue(spList, 0, 1024, 1, 1, spOtfBufferCount, spReadAheadBufferCount);

	TOverlappedDataBuffer buffer1(1024, nullptr);
	buffer1.SetLastPart(true);
	buffer1.SetFilePosition(0);
	queue.Push(&buffer1);

	queue.SetDataSourceFinished(&buffer1);

	EXPECT_EQ(&buffer1, queue.Pop());
	EXPECT_EQ(0, spList->GetCount());

	TOverlappedDataBuffer buffer2(1024, nullptr);
	buffer2.SetFilePosition(1000);
	EXPECT_THROW(queue.Push(&buffer2), TCoreException);
}
