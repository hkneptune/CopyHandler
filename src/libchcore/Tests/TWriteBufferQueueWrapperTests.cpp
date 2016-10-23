#include "stdafx.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../TWriteBufferQueueWrapper.h"
#include <memory>

using namespace chcore;

TEST(TWriteBufferQueueWrapperTests, DefaultTest)
{
	TOrderedBufferQueuePtr spQueue(std::make_shared<TOrderedBufferQueue>());
	TWriteBufferQueueWrapper queueWrapper(spQueue);
}
