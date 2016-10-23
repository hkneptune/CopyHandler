#include "stdafx.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../TReadBufferQueueWrapper.h"
#include <memory>

using namespace chcore;

TEST(TReadBufferQueueWrapperTests, DefaultTest)
{
	TBufferListPtr spList(std::make_shared<TBufferList>());

	TReadBufferQueueWrapper queueWrapper(spList, 0, 0);
}
