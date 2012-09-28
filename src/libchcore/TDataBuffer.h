// ============================================================================
//  Copyright (C) 2001-2012 by Jozef Starosczyk
//  ixen@copyhandler.com
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU Library General Public License
//  (version 2) as published by the Free Software Foundation;
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU Library General Public
//  License along with this program; if not, write to the
//  Free Software Foundation, Inc.,
//  59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// ============================================================================
/// @file  TDataBuffer.h
/// @date  2012/03/04
/// @brief Contains class representing buffer for data.
// ============================================================================
#ifndef __TDATABUFFER_H__
#define __TDATABUFFER_H__

#include "libchcore.h"

BEGIN_CHCORE_NAMESPACE

namespace details
{
	class TVirtualAllocMemoryBlock
	{
	public:
		TVirtualAllocMemoryBlock(size_t stSize, size_t stChunkSize);
		~TVirtualAllocMemoryBlock();

		void GetFreeChunks(std::list<LPVOID>& rListChunks);
		void ReleaseChunks(std::list<LPVOID>& rListChunks);
		bool ReleaseChunk(LPVOID pChunk);

		size_t CountOwnChunks(const std::list<LPVOID>& rListChunks);

		bool IsChunkOwner(LPVOID pChunk) const;
		bool AreAllChunksFree() const;
		bool HasFreeChunks() const;

	private:
		TVirtualAllocMemoryBlock(const TVirtualAllocMemoryBlock&);
		TVirtualAllocMemoryBlock& operator=(const TVirtualAllocMemoryBlock&);

		void AllocBlock(size_t stSize, size_t stChunkSize);
		void FreeBlock();

		bool IsValidChunk(LPVOID pChunk) const;

	private:
		LPVOID m_pMemory;
		std::set<LPVOID> m_setFreeChunks;
		size_t m_stMemorySize;
		size_t m_stChunkSize;
	};

	typedef boost::shared_ptr<TVirtualAllocMemoryBlock> TVirtualAllocMemoryBlockPtr;
}

template<class T> T RoundUp(T number, T roundValue) { return ((number + roundValue - 1) & ~(roundValue - 1)); }

class TDataBufferManager;

class LIBCHCORE_API TSimpleDataBuffer
{
public:
	TSimpleDataBuffer();
	~TSimpleDataBuffer();

	LPVOID GetBufferPtr();
	void ReleaseBuffer();

	void SetDataSize(size_t stDataSize);
	size_t GetDataSize() const { return m_stDataSize; }

	void CutDataFromBuffer(size_t stCount);

private:
	TSimpleDataBuffer(const TSimpleDataBuffer&);
	TSimpleDataBuffer& operator=(const TSimpleDataBuffer&);

	void Initialize(TDataBufferManager& rBufferManager, LPVOID pBuffer, size_t stBufferSize);

private:
	LPVOID m_pBuffer;
	TDataBufferManager* m_pBufferManager;
	size_t m_stBufferSize;
	size_t m_stDataSize;

	friend class TDataBufferManager;
};

typedef boost::shared_ptr<TSimpleDataBuffer> TSimpleDataBufferPtr;

class LIBCHCORE_API TDataBufferManager
{
public:
	static const size_t DefaultAllocGranularity = 4096;
	static const size_t DefaultBufferSize = 65536;
	static const size_t DefaultPageSize = 1024*1024;
	static const size_t DefaultMaxMemory = 1024*1024;

public:
	TDataBufferManager();
	~TDataBufferManager();

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

private:
	void FreeBuffers();

	bool CanAllocPage() const;	// checks if a buffer can be returned after allocating new page of memory
	bool AllocNewPage();

	void FreeAllocatedPages(size_t stPagesCount);
	void FreePage(const details::TVirtualAllocMemoryBlockPtr& spAllocBlock);

private:
#pragma warning(push)
#pragma warning(disable: 4251)
	std::vector<details::TVirtualAllocMemoryBlockPtr> m_vVirtualAllocBlocks;
	std::vector<details::TVirtualAllocMemoryBlockPtr> m_vAllocBlocksToFree;
	std::list<LPVOID> m_listUnusedBuffers;
#pragma warning(pop)
	
	size_t m_stMaxMemory;		// maximum amount of memory to use
	size_t m_stPageSize;		// size of a single page of real memory to be allocated (allocation granularity)
	size_t m_stBufferSize;		// size of a single chunk of memory retrievable by caller
};

END_CHCORE_NAMESPACE

#endif
