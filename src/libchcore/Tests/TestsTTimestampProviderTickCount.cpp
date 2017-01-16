#include "stdafx.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../TTimestampProviderTickCount.h"

using namespace chcore;

TEST(TestsTTimestampProviderTickCount, DefaultConstruction)
{
	TTimestampProviderTickCount provider;
	unsigned long long ullTimestamp1 = provider.GetCurrentTimestamp();
	unsigned long long ullTimestamp2 = provider.GetCurrentTimestamp();

	EXPECT_LE(ullTimestamp1, ullTimestamp2);
}
