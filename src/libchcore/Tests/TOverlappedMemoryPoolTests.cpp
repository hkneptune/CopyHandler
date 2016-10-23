#include "stdafx.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../TOverlappedMemoryPool.h"

using namespace chcore;

TEST(TOverlappedMemoryPoolTests, DefaultTest)
{
	TOverlappedMemoryPool memoryPool;
}
