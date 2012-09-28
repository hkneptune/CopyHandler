#include "stdafx.h"
#include "../../../src/libicpf/gen_types.h"
#include "../../../src/libchcore/TDataBuffer.h"
#include "../../../src/libchcore/TCoreException.h"

///////////////////////////////////////////////////////////////////////////////
// TSimpleDataBuffer

TEST(TSimpleDataBuffer, GetBufferPtr)
{
	chcore::TSimpleDataBuffer tDataBuffer;
	EXPECT_EQ(tDataBuffer.GetBufferPtr(), (LPVOID)NULL);
}

TEST(TSimpleDataBuffer, ReleaseBuffer)
{
	chcore::TSimpleDataBuffer tDataBuffer;
	EXPECT_NO_FATAL_FAILURE(tDataBuffer.ReleaseBuffer());
}


TEST(TSimpleDataBuffer, GetSetDataSize)
{
	chcore::TSimpleDataBuffer tDataBuffer;
	EXPECT_EQ(tDataBuffer.GetDataSize(), 0);
	EXPECT_THROW(tDataBuffer.SetDataSize(4273), chcore::TCoreException);
	EXPECT_EQ(tDataBuffer.GetDataSize(), 0);
}


TEST(TSimpleDataBuffer, CutDataFromBuffer)
{
	chcore::TSimpleDataBuffer tDataBuffer;

	EXPECT_NO_FATAL_FAILURE(tDataBuffer.CutDataFromBuffer(7344));
	EXPECT_NO_FATAL_FAILURE(tDataBuffer.CutDataFromBuffer(0));
}

///////////////////////////////////////////////////////////////////////////////
// TDataBufferManager

TEST(TDataBufferManager, CheckBufferConfigBase)
{
	// only max mem - default values
	size_t stMaxMem(0);
	EXPECT_EQ(chcore::TDataBufferManager::CheckBufferConfig(stMaxMem), false);
	EXPECT_EQ(stMaxMem, chcore::TDataBufferManager::DefaultMaxMemory);

	const size_t stTestSize = 103145;
	stMaxMem = stTestSize;
	EXPECT_EQ(chcore::TDataBufferManager::CheckBufferConfig(stMaxMem), false);
	EXPECT_LE(stTestSize, stMaxMem);
}

TEST(TDataBufferManager, CheckBufferConfigExt)
{
	// detailed config - default values
	size_t stMaxMem = 0;
	size_t stPageSize(0);
	size_t stChunkSize(0);
	EXPECT_EQ(chcore::TDataBufferManager::CheckBufferConfig(stMaxMem, stPageSize, stChunkSize), false);
	EXPECT_EQ(stMaxMem, chcore::TDataBufferManager::DefaultMaxMemory);
	EXPECT_EQ(stPageSize, chcore::TDataBufferManager::DefaultPageSize);
	EXPECT_EQ(stChunkSize, chcore::TDataBufferManager::DefaultBufferSize);

	const size_t stTestMaxSize = 1237645;
	const size_t stTestPageSize = 34563;
	const size_t stTestBufferSize = 120;
	stMaxMem = stTestMaxSize;
	stPageSize = stTestPageSize;
	stChunkSize = stTestBufferSize;
	EXPECT_EQ(chcore::TDataBufferManager::CheckBufferConfig(stMaxMem, stPageSize, stChunkSize), false);
	EXPECT_LE(stTestMaxSize, stMaxMem);
	EXPECT_LE(stTestPageSize, stPageSize);
	EXPECT_LE(stTestBufferSize, stChunkSize);
}

TEST(TDataBufferManager, FailedInitializations)
{
	chcore::TDataBufferManager tBufferManager;

	// failed initializations
	EXPECT_THROW(tBufferManager.Initialize(chcore::TDataBufferManager::DefaultMaxMemory - 1), chcore::TCoreException);
	EXPECT_EQ(tBufferManager.IsInitialized(), false);

	EXPECT_THROW(tBufferManager.Initialize(chcore::TDataBufferManager::DefaultMaxMemory, chcore::TDataBufferManager::DefaultPageSize - 1, 
		chcore::TDataBufferManager::DefaultBufferSize), chcore::TCoreException);
	EXPECT_EQ(tBufferManager.IsInitialized(), false);

	// succeeded initialization
	EXPECT_NO_FATAL_FAILURE(tBufferManager.Initialize(chcore::TDataBufferManager::DefaultPageSize));
	EXPECT_EQ(tBufferManager.IsInitialized(), true);
	EXPECT_EQ(tBufferManager.GetMaxMemorySize(), chcore::TDataBufferManager::DefaultPageSize);
}

class TInitializedBufferManager : public ::testing::Test
{
protected:
	virtual void SetUp()
	{
		size_t stMaxMemory = 1048034;

		chcore::TDataBufferManager::CheckBufferConfig(stMaxMemory);
		tBufferManager.Initialize(stMaxMemory);
	}

	chcore::TDataBufferManager tBufferManager;
};

TEST_F(TInitializedBufferManager, FailedResize)
{
	EXPECT_TRUE(tBufferManager.IsInitialized());

	size_t stCurrentMaxSize = tBufferManager.GetMaxMemorySize();
	size_t stCurrentPageSize = tBufferManager.GetPageSize();
	size_t stCurrentBufferSize = tBufferManager.GetSimpleBufferSize();

	// try to change to something useless, check if nothing breaks inside
	EXPECT_THROW(tBufferManager.ChangeMaxMemorySize(0), chcore::TCoreException);
	EXPECT_EQ(stCurrentMaxSize ,tBufferManager.GetMaxMemorySize());
	EXPECT_EQ(stCurrentPageSize, tBufferManager.GetPageSize());
	EXPECT_EQ(stCurrentBufferSize, tBufferManager.GetSimpleBufferSize());
}

TEST_F(TInitializedBufferManager, ResizeWithSimpleBufferChecks)
{
	EXPECT_TRUE(tBufferManager.IsInitialized());
	EXPECT_EQ((tBufferManager.GetMaxMemorySize() / tBufferManager.GetSimpleBufferSize()), tBufferManager.GetCountOfFreeBuffers());

	size_t stCurrentMaxSize = tBufferManager.GetMaxMemorySize();

	// try to change to something useless, check if nothing breaks inside
	size_t stNewBufferSize = stCurrentMaxSize / 2;
	tBufferManager.CheckResizeSize(stNewBufferSize);	// can't assume that it will return true here, since we don't really know what's the buffer size now...

	EXPECT_NO_FATAL_FAILURE(tBufferManager.ChangeMaxMemorySize(stNewBufferSize));
	EXPECT_EQ(tBufferManager.GetMaxMemorySize(), stNewBufferSize);

	EXPECT_TRUE(tBufferManager.HasFreeBuffer());
	EXPECT_EQ((tBufferManager.GetMaxMemorySize() / tBufferManager.GetSimpleBufferSize()), tBufferManager.GetCountOfFreeBuffers());
}
