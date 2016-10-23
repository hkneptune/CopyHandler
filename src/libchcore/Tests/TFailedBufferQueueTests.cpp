#include "stdafx.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../TFailedBufferQueue.h"

using namespace chcore;

TEST(TFailedBufferQueueTests, DefaultTest)
{
	TFailedBufferQueue queue;
}
