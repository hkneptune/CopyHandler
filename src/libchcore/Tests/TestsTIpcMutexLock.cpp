#include "stdafx.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../TIpcMutex.h"
#include "../TIpcMutexLock.h"
#include "../TCoreException.h"

using namespace chcore;

TEST(TestsTIpcMutexLock, DefaultConstruction)
{
	TIpcMutex mutex;

	EXPECT_THROW(TIpcMutexLock lock(mutex), TCoreException);
}

TEST(TestsTIpcMutexLock, ScopedLock)
{
	TIpcMutex mutex(L"UnitTestsNamedIpcMutex");

	{
		TIpcMutexLock lock(mutex);
		EXPECT_TRUE(mutex.IsLocked());
	}

	EXPECT_FALSE(mutex.IsLocked());
}

TEST(TestsTIpcMutexLock, ScopedLock_AlreadyLocked)
{
	TIpcMutex mutex(L"UnitTestsNamedIpcMutex");
	mutex.Lock();
	EXPECT_TRUE(mutex.IsLocked());
	EXPECT_THROW(TIpcMutexLock lock(mutex, 100), TCoreException);
}
