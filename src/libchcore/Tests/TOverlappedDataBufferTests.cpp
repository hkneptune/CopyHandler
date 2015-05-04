#include "stdafx.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../TOverlappedDataBuffer.h"
#include "../TCoreException.h"

using namespace chcore;

TEST(TOverlappedDataBufferTests, DefaultConstructor_InvalidInput)
{
	EXPECT_THROW(TOverlappedDataBuffer(0, nullptr), TCoreException);
}
