#include "stdafx.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../TIpcMutex.h"
#include "../TCoreException.h"

using namespace chcore;

TEST(TestsTIpcMutex, DefaultConstruction)
{
	TIpcMutex mutex;

	EXPECT_FALSE(mutex.IsLocked());
	EXPECT_THROW(mutex.Lock(), TCoreException);
	EXPECT_FALSE(mutex.IsLocked());
	EXPECT_THROW(mutex.Unlock(), TCoreException);
	EXPECT_FALSE(mutex.IsLocked());
}

TEST(TestsTIpcMutex, NamedConstruction)
{
	TIpcMutex mutex(L"UnitTestsNamedIpcMutex");

	EXPECT_FALSE(mutex.IsLocked());
	mutex.Lock(100);
	EXPECT_TRUE(mutex.IsLocked());
	mutex.Unlock();
	EXPECT_FALSE(mutex.IsLocked());
}

TEST(TestsTIpcMutex, SeparateCreation)
{
	TIpcMutex mutex;
	mutex.CreateMutex(L"UnitTestsNamedIpcMutex");

	EXPECT_FALSE(mutex.IsLocked());
	mutex.Lock();
	EXPECT_TRUE(mutex.IsLocked());
	mutex.Unlock();
	EXPECT_FALSE(mutex.IsLocked());
}

TEST(TestsTIpcMutex, RecreateMutex)
{
	TIpcMutex mutex(L"UnitTestsNamedIpcMutex2");
	mutex.Lock();

	mutex.CreateMutex(L"UnitTestsNamedIpcMutex");

	EXPECT_FALSE(mutex.IsLocked());
	mutex.Lock();
	EXPECT_TRUE(mutex.IsLocked());
	mutex.Unlock();
	EXPECT_FALSE(mutex.IsLocked());
}
