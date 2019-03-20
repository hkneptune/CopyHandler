#include "stdafx.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../../libstring/TString.h"
#include "../TSharedMemory.h"
#include "../TCoreException.h"

using namespace chcore;
using namespace string;

TEST(TestsTSharedMemory, DefaultConstruction)
{
	TSharedMemory memory;
	EXPECT_EQ(nullptr, memory.GetData());
	EXPECT_EQ(nullptr, memory.GetFullData());
	EXPECT_EQ(0UL, memory.GetDataSize());
	EXPECT_EQ(0UL, memory.GetSharedMemorySize());
}

TEST(TestsTSharedMemory, Create_ZeroSize)
{
	TSharedMemory memory;
	EXPECT_THROW(memory.Create(L"UnitTestsSharedMemoryName", 0), TCoreException);
}

TEST(TestsTSharedMemory, Create_NonZeroSize)
{
	TSharedMemory memory;
	memory.Create(L"UnitTestsSharedMemoryName", 320);

	EXPECT_TRUE(memory.GetData() != nullptr);
	EXPECT_EQ(0UL, *(unsigned int*)memory.GetData());
	EXPECT_TRUE(memory.GetFullData() != nullptr);
	EXPECT_EQ(4UL, *(unsigned int*)memory.GetFullData());
	EXPECT_EQ(4UL, memory.GetDataSize());
	EXPECT_EQ(324UL, memory.GetSharedMemorySize());
}

TEST(TestsTSharedMemory, Create_FromEmptyString)
{
	TString strData;
	TSharedMemory memory;
	memory.Create(L"UnitTestsSharedMemoryName", strData);
	EXPECT_TRUE(memory.GetData() != nullptr);
	EXPECT_EQ(0UL, *(unsigned int*)memory.GetData());
	EXPECT_TRUE(memory.GetFullData() != nullptr);
	EXPECT_EQ(2UL, *(unsigned int*)memory.GetFullData());
	EXPECT_EQ(2UL, memory.GetDataSize());
	EXPECT_EQ(6UL, memory.GetSharedMemorySize());
}

TEST(TestsTSharedMemory, Create_FromFullString)
{
	TString strData(L"SomeString");
	TSharedMemory memory;
	memory.Create(L"UnitTestsSharedMemoryName", strData);
	EXPECT_TRUE(memory.GetData() != nullptr);
	EXPECT_EQ(L'S', *memory.GetData());
	EXPECT_TRUE(memory.GetFullData() != nullptr);
	EXPECT_EQ(22, *memory.GetFullData());
	EXPECT_EQ(22UL, memory.GetDataSize());
	EXPECT_EQ(26UL, memory.GetSharedMemorySize());
}

TEST(TestsTSharedMemory, Create_FromBufferZeroSize)
{
	char* pszData = "SomeString";
	TSharedMemory memory;
	EXPECT_THROW(memory.Create(L"UnitTestsSharedMemoryName", (BYTE*)pszData, 0), TCoreException);
}

TEST(TestsTSharedMemory, Create_FromNullBufferWithZeroSize)
{
	TSharedMemory memory;
	EXPECT_THROW(memory.Create(L"UnitTestsSharedMemoryName", nullptr, 0), TCoreException);
}

TEST(TestsTSharedMemory, Create_FromNullBufferWithNonZeroSize)
{
	TSharedMemory memory;
	EXPECT_THROW(memory.Create(L"UnitTestsSharedMemoryName", nullptr, 5), TCoreException);
}

TEST(TestsTSharedMemory, Create_FromEmptyBuffer)
{
	char* pszData = "";
	TSharedMemory memory;
	memory.Create(L"UnitTestsSharedMemoryName", (BYTE*)pszData, 1);
	EXPECT_TRUE(memory.GetData() != nullptr);
	EXPECT_EQ(0UL, *(unsigned int*)memory.GetData());
	EXPECT_TRUE(memory.GetFullData() != nullptr);
	EXPECT_EQ(1UL, *(unsigned int*)memory.GetFullData());
	EXPECT_EQ(1UL, memory.GetDataSize());
	EXPECT_EQ(5UL, memory.GetSharedMemorySize());
}

TEST(TestsTSharedMemory, Create_FromFullBuffer)
{
	char* pszData = "SomeString";
	TSharedMemory memory;
	memory.Create(L"UnitTestsSharedMemoryName", (BYTE*)pszData, 4);
	EXPECT_TRUE(memory.GetData() != nullptr);
	EXPECT_EQ(L'S', *memory.GetData());
	EXPECT_TRUE(memory.GetFullData() != nullptr);
	EXPECT_EQ(4, *memory.GetFullData());
	EXPECT_EQ(4UL, memory.GetDataSize());
	EXPECT_EQ(8UL, memory.GetSharedMemorySize());
}

////////////////////////////////////////////////////////////////////////////////
// Receiving side

TEST(TestsTSharedMemory, Open)
{
	char* pszData = "SomeString";
	TSharedMemory srcMemory;
	srcMemory.Create(L"UnitTestsSharedMemoryName", (BYTE*)pszData, 4);

	TSharedMemory dstMemory;
	dstMemory.Open(L"UnitTestsSharedMemoryName");

	EXPECT_TRUE(dstMemory.GetData() != nullptr);
	EXPECT_EQ(L'S', *dstMemory.GetData());
	EXPECT_TRUE(dstMemory.GetFullData() != nullptr);
	EXPECT_EQ(4, *dstMemory.GetFullData());
	EXPECT_EQ(4UL, dstMemory.GetDataSize());
	EXPECT_EQ(8UL, dstMemory.GetSharedMemorySize());
}

TEST(TestsTSharedMemory, OpenNonExistentMemory)
{
	TSharedMemory dstMemory;
	EXPECT_THROW(dstMemory.Open(L"UnitTestsSharedMemoryName"), TCoreException);
}

//////////////////////////////////////////////////////////////////////////////////
// Read/write

TEST(TestsTSharedMemory, WriteStringReadString)
{
	TString strData(L"SomeString");
	TSharedMemory memory;
	memory.Create(L"UnitTestsSharedMemoryName", 256);
	memory.Write(strData);

	EXPECT_TRUE(memory.GetData() != nullptr);
	EXPECT_EQ(L'S', *memory.GetData());
	EXPECT_TRUE(memory.GetFullData() != nullptr);
	EXPECT_EQ(22, *memory.GetFullData());
	EXPECT_EQ(22UL, memory.GetDataSize());
	EXPECT_EQ(260UL, memory.GetSharedMemorySize());

	TString strReadData;
	memory.Read(strReadData);
	EXPECT_EQ(strData, strReadData);
}

TEST(TestsTSharedMemory, WriteBufferReadString)
{
	wchar_t* pszData = L"SomeString";
	TSharedMemory memory;
	memory.Create(L"UnitTestsSharedMemoryName", 256);
	memory.Write((BYTE*)pszData, (unsigned int)((wcslen(pszData) + 1) * sizeof(wchar_t)));

	EXPECT_TRUE(memory.GetData() != nullptr);
	EXPECT_EQ(L'S', *memory.GetData());
	EXPECT_TRUE(memory.GetFullData() != nullptr);
	EXPECT_EQ(22, *memory.GetFullData());
	EXPECT_EQ(22UL, memory.GetDataSize());
	EXPECT_EQ(260UL, memory.GetSharedMemorySize());

	TString strReadData;
	memory.Read(strReadData);
	EXPECT_STREQ(L"SomeString", strReadData.c_str());
}

TEST(TestsTSharedMemory, WriteStringReadString_ReceiverSide)
{
	TString strData(L"SomeString");
	TSharedMemory srcMemory;
	srcMemory.Create(L"UnitTestsSharedMemoryName", 256);
	srcMemory.Write(strData);

	TSharedMemory memory;
	memory.Open(L"UnitTestsSharedMemoryName");

	EXPECT_TRUE(memory.GetData() != nullptr);
	EXPECT_EQ(L'S', *memory.GetData());
	EXPECT_TRUE(memory.GetFullData() != nullptr);
	EXPECT_EQ(22, *memory.GetFullData());
	EXPECT_EQ(22UL, memory.GetDataSize());
	EXPECT_EQ(26UL, memory.GetSharedMemorySize());

	TString strReadData;
	memory.Read(strReadData);
	EXPECT_EQ(strData, strReadData);
}

TEST(TestsTSharedMemory, WriteBufferReadString_ReceiverSide)
{
	wchar_t* pszData = L"SomeString";
	TSharedMemory srcMemory;
	srcMemory.Create(L"UnitTestsSharedMemoryName", 256);
	srcMemory.Write((BYTE*)pszData, (unsigned int)((wcslen(pszData) + 1) * sizeof(wchar_t)));

	TSharedMemory memory;
	memory.Open(L"UnitTestsSharedMemoryName");

	EXPECT_TRUE(memory.GetData() != nullptr);
	EXPECT_EQ(L'S', *memory.GetData());
	EXPECT_TRUE(memory.GetFullData() != nullptr);
	EXPECT_EQ(22, *memory.GetFullData());
	EXPECT_EQ(22UL, memory.GetDataSize());
	EXPECT_EQ(26UL, memory.GetSharedMemorySize());

	TString strReadData;
	memory.Read(strReadData);
	EXPECT_STREQ(L"SomeString", strReadData.c_str());
}
