#include "stdafx.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../TBufferList.h"
#include "../TOverlappedDataBuffer.h"

using namespace chengine;
using namespace chcore;

TEST(TBufferListTests, DefaultConstructionSanityTest)
{
	TBufferList bufferList;

	EXPECT_EQ(0, bufferList.GetCount());
	EXPECT_EQ(true, bufferList.IsEmpty());
	EXPECT_EQ(nullptr, bufferList.Pop());
}

TEST(TBufferListTests, PushNull)
{
	TBufferList bufferList;

	EXPECT_THROW(bufferList.Push(nullptr), TCoreException);
}

TEST(TBufferListTests, PushBuffer)
{
	TBufferList bufferList;

	TOverlappedDataBuffer rBuffer(4096, nullptr);

	bufferList.Push(&rBuffer);

	EXPECT_EQ(1, bufferList.GetCount());
	EXPECT_EQ(false, bufferList.IsEmpty());
	EXPECT_EQ(&rBuffer, bufferList.Pop());
}

TEST(TBufferListTests, Clear)
{
	TBufferList bufferList;

	TOverlappedDataBuffer rBuffer(4096, nullptr);

	bufferList.Push(&rBuffer);
	bufferList.Clear();

	EXPECT_EQ(0, bufferList.GetCount());
	EXPECT_EQ(true, bufferList.IsEmpty());
	EXPECT_EQ(nullptr, bufferList.Pop());
}
