#include "stdafx.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../TDataBuffer.h"
#include "../TCoreException.h"

// fixtures
class BasicBufferFixture : public ::testing::Test
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

class DetailedBufferFixture : public ::testing::Test
{
protected:
	virtual void SetUp()
	{
		size_t stMaxMemory = 1048034;
		size_t stPageSize = 262144;
		size_t stBufferSize = 65536;

		chcore::TDataBufferManager::CheckBufferConfig(stMaxMemory, stPageSize, stBufferSize);

		tBufferManager.Initialize(stMaxMemory, stPageSize, stBufferSize);
	}

	chcore::TDataBufferManager tBufferManager;
};

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
	size_t stMaxMem(0);
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

TEST_F(BasicBufferFixture, FailedResize)
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

TEST_F(BasicBufferFixture, ResizeToSameSizeWithSimpleBufferChecks)
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

TEST_F(DetailedBufferFixture, SmallBufferOperations)
{
	// get a single buffer, check internals
	chcore::TSimpleDataBuffer tBuffer;
	EXPECT_TRUE(tBufferManager.GetFreeBuffer(tBuffer));
	EXPECT_EQ(tBuffer.GetDataSize(), 0);
	EXPECT_EQ(tBuffer.GetBufferSize(), 65536);

	// check whether there are still free buffers inside manager
	EXPECT_EQ(tBufferManager.GetCountOfFreeBuffersNA(), 3);
	EXPECT_EQ(tBufferManager.GetCountOfFreeBuffers(), 15);
}

TEST_F(DetailedBufferFixture, WithoutAdditionalAllocTest)
{
	// do this in separate scope to allow auto-release of buffers at the end
	{
		std::vector<chcore::TSimpleDataBufferPtr> vBuffers;
		// get first 4 buffers and check state
		for(size_t stIndex = 0; stIndex < 4; ++stIndex)
		{
			chcore::TSimpleDataBufferPtr spBuf(new chcore::TSimpleDataBuffer);

			EXPECT_TRUE(tBufferManager.GetFreeBuffer(*spBuf.get()));
			vBuffers.push_back(spBuf);
		}

		EXPECT_EQ(tBufferManager.GetCountOfFreeBuffersNA(), 0);
		EXPECT_EQ(tBufferManager.GetCountOfFreeBuffers(), 12);
		EXPECT_EQ(tBufferManager.GetRealAllocatedMemorySize(), 262144);	// only one page should be allocated at the moment
		EXPECT_FALSE(tBufferManager.HasFreeBufferNA());
		EXPECT_TRUE(tBufferManager.HasFreeBuffer());
	}

	// check that everything was freed
	EXPECT_EQ(tBufferManager.GetCountOfFreeBuffersNA(), 4);
	EXPECT_EQ(tBufferManager.GetCountOfFreeBuffers(), 16);
	EXPECT_EQ(tBufferManager.GetRealAllocatedMemorySize(), 262144);	// only one page should be allocated at the moment
	EXPECT_TRUE(tBufferManager.HasFreeBuffer());
	EXPECT_TRUE(tBufferManager.HasFreeBufferNA());
}

TEST_F(DetailedBufferFixture, FullBufferTest)
{
	// do this in separate scope to allow auto-release of buffers at the end
	{
		// retrieve all the buffers that are inside
		std::vector<chcore::TSimpleDataBufferPtr> vBuffers;
		for(size_t stIndex = 0; stIndex < 16; ++stIndex)
		{
			chcore::TSimpleDataBufferPtr spBuf(new chcore::TSimpleDataBuffer);

			EXPECT_TRUE(tBufferManager.GetFreeBuffer(*spBuf.get()));
			vBuffers.push_back(spBuf);
		}

		// ensure everything was taken
		EXPECT_EQ(tBufferManager.GetCountOfFreeBuffersNA(), 0);
		EXPECT_EQ(tBufferManager.GetCountOfFreeBuffers(), 0);
		EXPECT_EQ(tBufferManager.GetRealAllocatedMemorySize(), 1024*1024);	// only one page should be allocated at the moment
		EXPECT_FALSE(tBufferManager.HasFreeBufferNA());
		EXPECT_FALSE(tBufferManager.HasFreeBuffer());

		// try to get one more buffer
		chcore::TSimpleDataBuffer tFailBuffer;
		EXPECT_FALSE(tBufferManager.GetFreeBuffer(tFailBuffer));
	}

	// ensure everything was freed as expected
	EXPECT_EQ(tBufferManager.GetCountOfFreeBuffers(), 16);
	EXPECT_EQ(tBufferManager.GetCountOfFreeBuffersNA(), 16);
	EXPECT_EQ(tBufferManager.GetRealAllocatedMemorySize(), 1024*1024);	// only one page should be allocated at the moment
	EXPECT_TRUE(tBufferManager.HasFreeBufferNA());
	EXPECT_TRUE(tBufferManager.HasFreeBuffer());
}

TEST_F(DetailedBufferFixture, FullBufferWithResizeTest)
{
	// do this in separate scope to allow auto-release of buffers at the end
	{
		// get all buffers
		std::vector<chcore::TSimpleDataBufferPtr> vBuffers;
		for(size_t stIndex = 0; stIndex < 16; ++stIndex)
		{
			chcore::TSimpleDataBufferPtr spBuf(new chcore::TSimpleDataBuffer);

			EXPECT_TRUE(tBufferManager.GetFreeBuffer(*spBuf.get()));
			vBuffers.push_back(spBuf);
		}

		// now resize to quarter the size
		size_t stNewSize = 262144;
		tBufferManager.CheckResizeSize(stNewSize);
		EXPECT_NO_FATAL_FAILURE(tBufferManager.ChangeMaxMemorySize(stNewSize));

		// since all of the memory was already taken, there was no real freeing
		EXPECT_EQ(tBufferManager.GetCountOfFreeBuffers(), 0);
		EXPECT_EQ(tBufferManager.GetCountOfFreeBuffersNA(), 0);
		EXPECT_EQ(tBufferManager.GetRealAllocatedMemorySize(), 1024*1024);	// only one page should be allocated at the moment
		EXPECT_FALSE(tBufferManager.HasFreeBufferNA());
		EXPECT_FALSE(tBufferManager.HasFreeBuffer());
		EXPECT_EQ(tBufferManager.GetMaxMemorySize(), 256*1024);	// only a single page is available

		// get rid of the small buffers (except for one)
		for(size_t stIndex = 0; stIndex < 15; ++stIndex)
		{
			vBuffers.erase(vBuffers.end() - 1);
		}

		// now only one page should be left
		EXPECT_EQ(tBufferManager.GetCountOfFreeBuffers(), 3);
		EXPECT_EQ(tBufferManager.GetCountOfFreeBuffersNA(), 3);
		EXPECT_EQ(tBufferManager.GetRealAllocatedMemorySize(), 256*1024);	// only one page should be allocated at the moment
		EXPECT_TRUE(tBufferManager.HasFreeBufferNA());
		EXPECT_TRUE(tBufferManager.HasFreeBuffer());
		EXPECT_EQ(tBufferManager.GetMaxMemorySize(), 256*1024);
	}

	EXPECT_EQ(tBufferManager.GetCountOfFreeBuffers(), 4);
	EXPECT_EQ(tBufferManager.GetCountOfFreeBuffersNA(), 4);
	EXPECT_EQ(tBufferManager.GetRealAllocatedMemorySize(), 256*1024);	// only one page should be allocated at the moment
	EXPECT_TRUE(tBufferManager.HasFreeBufferNA());
	EXPECT_TRUE(tBufferManager.HasFreeBuffer());
}

/*
static bool CheckBufferConfig(size_t& stMaxMemory, size_t& stPageSize, size_t& stBufferSize);
static bool CheckBufferConfig(size_t& stMaxMemory);

// initialization
void Initialize(size_t stMaxMemory);
void Initialize(size_t stMaxMemory, size_t stPageSize, size_t stBufferSize);
bool IsInitialized() const;

bool CheckResizeSize(size_t& stNewMaxSize);
void ChangeMaxMemorySize(size_t stNewMaxSize);

// current settings
size_t GetMaxMemorySize() const { return m_stMaxMemory; }
size_t GetPageSize() const { return m_stPageSize; }
size_t GetSimpleBufferSize() const { return m_stBufferSize; }

size_t GetRealAllocatedMemorySize() const;

// buffer retrieval
bool HasFreeBuffer() const;		// checks if a buffer is available without allocating any new memory
size_t GetCountOfFreeBuffers() const;	// how many free buffers are there that can be used without allocating additional memory

bool GetFreeBuffer(TSimpleDataBuffer& rSimpleBuffer);
void ReleaseBuffer(TSimpleDataBuffer& rSimpleBuffer);
*/
