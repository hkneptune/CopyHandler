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
/// @file  TDataBuffer.cpp
/// @date  2012/03/04
/// @brief Contains class representing buffer for data.
// ============================================================================
#include "stdafx.h"
#include "TDataBuffer.h"
#include <boost/bind.hpp>

BEGIN_CHCORE_NAMESPACE

namespace details
{
	TVirtualAllocMemoryBlock::TVirtualAllocMemoryBlock(size_t stSize, size_t stChunkSize) :
		m_pMemory(NULL),
		m_stMemorySize(0),
		m_stChunkSize(0)
	{
		AllocBlock(stSize, stChunkSize);
	}

	TVirtualAllocMemoryBlock::~TVirtualAllocMemoryBlock()
	{
		try
		{
			FreeBlock();
		}
		catch(...)
		{
		}
	}

	void TVirtualAllocMemoryBlock::GetFreeChunks(std::list<LPVOID>& rListChunks)
	{
		rListChunks.insert(rListChunks.end(), m_setFreeChunks.begin(), m_setFreeChunks.end());
		m_setFreeChunks.clear();
	}

	void TVirtualAllocMemoryBlock::ReleaseChunks(std::list<LPVOID>& rListChunks)
	{
		std::list<LPVOID>::iterator iterList = rListChunks.begin();
		while(iterList != rListChunks.end())
		{
			if(ReleaseChunk(*iterList))
				iterList = rListChunks.erase(iterList);
			else
				++iterList;
		}
	}

	bool TVirtualAllocMemoryBlock::ReleaseChunk(LPVOID pChunk)
	{
		if(IsValidChunk(pChunk))
		{
			m_setFreeChunks.insert(pChunk);
			return true;
		}
		return false;
	}

	size_t TVirtualAllocMemoryBlock::CountOwnChunks(const std::list<LPVOID>& rListChunks)
	{
		std::set<LPVOID> setChunks;
		for(std::list<LPVOID>::const_iterator iterList = rListChunks.begin(); iterList != rListChunks.end(); ++iterList)
		{
			if(IsValidChunk(*iterList))
				setChunks.insert(*iterList);
		}

		// include chunks already owned
		return setChunks.size() + m_setFreeChunks.size();
	}

	bool TVirtualAllocMemoryBlock::IsChunkOwner(LPVOID pChunk) const
	{
		LPVOID pMemoryEnd = (BYTE*)m_pMemory + m_stMemorySize;
		return(pChunk >= m_pMemory && pChunk < pMemoryEnd);
	}

	bool TVirtualAllocMemoryBlock::AreAllChunksFree() const
	{
		if(m_stChunkSize == 0)
			return true;
		return m_setFreeChunks.size() == m_stMemorySize / m_stChunkSize;
	}

	bool TVirtualAllocMemoryBlock::HasFreeChunks() const
	{
		return !m_setFreeChunks.empty();
	}

	void TVirtualAllocMemoryBlock::AllocBlock(size_t stSize, size_t stChunkSize)
	{
		FreeBlock();

		// allocate
		LPVOID pBuffer = VirtualAlloc(NULL, stSize, MEM_COMMIT, PAGE_READWRITE);
		if(!pBuffer)
			THROW_CORE_EXCEPTION(eErr_CannotAllocateMemory);

		m_pMemory = pBuffer;
		m_stMemorySize = stSize;
		m_stChunkSize = stChunkSize;

		// slice the page to buffers
		size_t stSliceCount = m_stMemorySize / m_stChunkSize;
		for(size_t stIndex = 0; stIndex < stSliceCount; ++stIndex)
		{
			LPVOID pSimpleBuffer = (BYTE*)pBuffer + stIndex * stChunkSize;
			m_setFreeChunks.insert(pSimpleBuffer);
		}
	}

	void TVirtualAllocMemoryBlock::FreeBlock()
	{
		if(m_pMemory)
		{
			VirtualFree(m_pMemory, 0, MEM_RELEASE);
			m_stMemorySize = 0;
			m_stChunkSize = 0;
		}
	}

	bool TVirtualAllocMemoryBlock::IsValidChunk(LPVOID pChunk) const
	{
		if(IsChunkOwner(pChunk))
		{
			bool bValidPtr = (((BYTE*)pChunk - (BYTE*)m_pMemory) % m_stChunkSize) == 0;
			_ASSERTE(bValidPtr);
			return bValidPtr;
		}
		else
			return false;
		
	}
}

///////////////////////////////////////////////////////////////////////////////////
// class TSimpleDataBuffer

TSimpleDataBuffer::TSimpleDataBuffer() :
	m_pBuffer(NULL),
	m_pBufferManager(NULL),
	m_stBufferSize(0),
	m_stDataSize(0)
{
}

TSimpleDataBuffer::~TSimpleDataBuffer()
{
	ReleaseBuffer();
}

LPVOID TSimpleDataBuffer::GetBufferPtr()
{
	return m_pBuffer;
}

void TSimpleDataBuffer::ReleaseBuffer()
{
	if(m_pBufferManager && m_pBuffer)
		m_pBufferManager->ReleaseBuffer(*this);
}

void TSimpleDataBuffer::Initialize(TDataBufferManager& rBufferManager, LPVOID pBuffer, size_t stBufferSize)
{
	ReleaseBuffer();

	m_pBufferManager = &rBufferManager;
	m_pBuffer = pBuffer;
	m_stBufferSize = stBufferSize;
}

void TSimpleDataBuffer::SetDataSize(size_t stDataSize)
{
	if(stDataSize != 0 && (stDataSize > m_stBufferSize || !m_pBuffer))
		THROW_CORE_EXCEPTION(eErr_InvalidArgument);

	m_stDataSize = stDataSize;
}

void TSimpleDataBuffer::CutDataFromBuffer(size_t stCount)
{
	if(stCount >= m_stBufferSize || !m_pBuffer)
		return;	// nothing to do

	memmove(m_pBuffer, (BYTE*)m_pBuffer + stCount, m_stBufferSize - stCount);
}

///////////////////////////////////////////////////////////////////////////////////
// class TDataBufferManager

TDataBufferManager::TDataBufferManager() :
	m_stMaxMemory(0),
	m_stPageSize(0),
	m_stBufferSize(0),
	m_stAllocBlocksToFree(0)
{
}

TDataBufferManager::~TDataBufferManager()
{
	try
	{
		FreeAllAllocBlocks();
	}
	catch(...)
	{
	}
}

bool TDataBufferManager::CheckBufferConfig(size_t& stMaxMemory, size_t& stPageSize, size_t& stBufferSize)
{
	bool bResult = true;

	// first the user-facing buffer size
	if(stBufferSize == 0)
	{
		stBufferSize = DefaultBufferSize;
		bResult = false;
	}
	else
	{
		size_t stNewSize = RoundUp(stBufferSize, DefaultAllocGranularity);
		if(stBufferSize != stNewSize)
		{
			stBufferSize = stNewSize;
			bResult = false;
		}
	}

	// now the page size
	if(stPageSize == 0)
	{
		stPageSize = std::max(DefaultPageSize, RoundUp(DefaultPageSize, stBufferSize));
		bResult = false;
	}
	else
	{
		size_t stNewSize = RoundUp(stPageSize, stBufferSize);
		if(stPageSize != stNewSize)
		{
			stPageSize = stNewSize;
			bResult = false;
		}
	}

	if(stMaxMemory == 0)
	{
		stMaxMemory = std::max(DefaultMaxMemory, RoundUp(DefaultMaxMemory, stPageSize));
		bResult = false;
	}
	else
	{
		size_t stNewSize = RoundUp(stMaxMemory, stPageSize);
		if(stMaxMemory != stNewSize)
		{
			stMaxMemory = stNewSize;
			bResult = false;
		}
	}

	return bResult;
}

bool TDataBufferManager::CheckBufferConfig(size_t& stMaxMemory)
{
	size_t stDefaultPageSize = DefaultPageSize;
	size_t stDefaultBufferSize = DefaultBufferSize;
	return CheckBufferConfig(stMaxMemory, stDefaultPageSize, stDefaultBufferSize);
}

void TDataBufferManager::Initialize(size_t stMaxMemory)
{
	Initialize(stMaxMemory, DefaultPageSize, DefaultBufferSize);
}

void TDataBufferManager::Initialize(size_t stMaxMemory, size_t stPageSize, size_t stBufferSize)
{
	FreeAllAllocBlocks();

	// validate input (note that input parameters should already be checked by caller)
	if(!CheckBufferConfig(stMaxMemory, stPageSize, stBufferSize))
		THROW_CORE_EXCEPTION(eErr_InvalidArgument);

	m_stMaxMemory = stMaxMemory;
	m_stPageSize = stPageSize;
	m_stBufferSize = stBufferSize;

	// allocate
	if(!AllocNewPage())
		THROW_CORE_EXCEPTION(eErr_CannotAllocateMemory);
}

bool TDataBufferManager::IsInitialized() const
{
	if(m_stPageSize == 0 || m_stMaxMemory == 0 || m_stBufferSize == 0)
		return false;
	return true;
}

bool TDataBufferManager::CheckResizeSize(size_t& stNewMaxSize)
{
	if(!IsInitialized())
	{
		stNewMaxSize = 0;
		return false;
	}

	size_t stPageSize = m_stPageSize;
	size_t stBufferSize = m_stBufferSize;

	bool bRes = CheckBufferConfig(stNewMaxSize, stPageSize, stBufferSize);
	// make sure the page size and buffer size are unchanged after the call
	_ASSERTE(stPageSize == m_stPageSize && stBufferSize == m_stBufferSize);
	if(stPageSize != m_stPageSize || stBufferSize != m_stBufferSize)
		THROW_CORE_EXCEPTION(eErr_InternalProblem);

	return bRes;
}

void TDataBufferManager::ChangeMaxMemorySize(size_t stNewMaxSize)
{
	if(!CheckResizeSize(stNewMaxSize))
		THROW_CORE_EXCEPTION(eErr_InvalidArgument);

	if(stNewMaxSize >= m_stMaxMemory)
		m_stMaxMemory = stNewMaxSize;
	else
	{
		size_t stCurrentMaxPages = m_stMaxMemory / m_stPageSize;
		size_t stNewMaxPages = stNewMaxSize / m_stPageSize;
		size_t stPagesToFree = stCurrentMaxPages - stNewMaxPages;
		size_t stPagesStillUnallocated = stCurrentMaxPages - m_vVirtualAllocBlocks.size();

		// first free the memory that has not been allocated yet
		if(stPagesStillUnallocated != 0 && stPagesToFree != 0)
		{
			size_t stUnallocatedPagesToFree = std::min(stPagesStillUnallocated, stPagesToFree);
			m_stMaxMemory -= stUnallocatedPagesToFree * m_stPageSize;
			stPagesToFree -= stUnallocatedPagesToFree;
		}

		// is there still too much memory that needs to be freed?
		if(stPagesToFree != 0)
		{
			// free pages that are already allocated
			FreeAllocatedPages(stPagesToFree);
		}
	}
}

size_t TDataBufferManager::GetRealAllocatedMemorySize() const
{
	return m_stPageSize * m_vVirtualAllocBlocks.size();
}

bool TDataBufferManager::HasFreeBuffer() const
{
	return !m_listUnusedBuffers.empty() || CanAllocPage();
}

size_t TDataBufferManager::GetCountOfFreeBuffers() const
{
	if(!IsInitialized())
		return 0;

	size_t stActivePages = m_vVirtualAllocBlocks.size() - m_stAllocBlocksToFree;

	// count of unallocated pages
	size_t stCurrentMaxPages = m_stMaxMemory / m_stPageSize;
	size_t stPagesStillUnallocated = stCurrentMaxPages - stActivePages;

	return m_listUnusedBuffers.size() + stPagesStillUnallocated * m_stPageSize / m_stBufferSize;
}

bool TDataBufferManager::HasFreeBufferNA() const
{
	return !m_listUnusedBuffers.empty();
}

size_t TDataBufferManager::GetCountOfFreeBuffersNA() const
{
	return m_listUnusedBuffers.size();
}

bool TDataBufferManager::CanAllocPage() const
{
	if(!IsInitialized())
		return false;

	size_t stActivePages = m_vVirtualAllocBlocks.size() - m_stAllocBlocksToFree;
	size_t stMaxPages = m_stMaxMemory / m_stPageSize;
	return stActivePages < stMaxPages;
}

bool TDataBufferManager::AllocNewPage()
{
	if(!CanAllocPage())
		return false;

	// re-use the old block if possible
	if(m_stAllocBlocksToFree != 0)
	{
		for(MemoryBlocksVector::iterator iterMem = m_vVirtualAllocBlocks.begin(); iterMem != m_vVirtualAllocBlocks.end(); ++iterMem)
		{
			if((*iterMem).second == eBlock_ToFree && (*iterMem).first->HasFreeChunks())
			{
				(*iterMem).second = eBlock_Active;
				--m_stAllocBlocksToFree;
				(*iterMem).first->GetFreeChunks(m_listUnusedBuffers);

				return true;
			}
		}
	}

	// alloc new block if can't re-use the old one
	details::TVirtualAllocMemoryBlockPtr spAllocBlock(new details::TVirtualAllocMemoryBlock(m_stPageSize, m_stBufferSize));
	m_vVirtualAllocBlocks.push_back(std::make_pair(spAllocBlock, eBlock_Active));
	spAllocBlock->GetFreeChunks(m_listUnusedBuffers);

	return true;
}

void TDataBufferManager::FreeAllocatedPages(size_t stPagesCount)
{
	if(stPagesCount == 0)
		return;

	std::vector<std::pair<details::TVirtualAllocMemoryBlockPtr, size_t> > vFreeBuffers;
	for(MemoryBlocksVector::iterator iterAllocBlock = m_vVirtualAllocBlocks.begin();
		iterAllocBlock != m_vVirtualAllocBlocks.end() && (*iterAllocBlock).second == eBlock_Active;
		++iterAllocBlock)
	{
		vFreeBuffers.push_back(std::make_pair((*iterAllocBlock).first, (*iterAllocBlock).first->CountOwnChunks(m_listUnusedBuffers)));
	}

	// sort by the count of free blocks
	std::sort(vFreeBuffers.begin(), vFreeBuffers.end(),
		boost::bind(&std::pair<details::TVirtualAllocMemoryBlockPtr, size_t>::second, _1) > boost::bind(&std::pair<details::TVirtualAllocMemoryBlockPtr, size_t>::second, _2));

	// and free pages with the most free blocks inside
	size_t stPagesToProcess = std::min(stPagesCount, vFreeBuffers.size());
	for(size_t stIndex = 0; stIndex < stPagesToProcess; ++stIndex)
	{
		FreePage(vFreeBuffers[stIndex].first);
		m_stMaxMemory -= m_stPageSize;
	}
}

// function expects arrays to be sorted
bool TDataBufferManager::FreePage(const details::TVirtualAllocMemoryBlockPtr& spAllocBlock)
{
	// locate the entry
	MemoryBlocksVector::iterator iterBlock = std::find_if(m_vVirtualAllocBlocks.begin(), m_vVirtualAllocBlocks.end(),
		boost::bind(&std::pair<details::TVirtualAllocMemoryBlockPtr, EBlockState>::first, _1) == spAllocBlock);

	if(iterBlock == m_vVirtualAllocBlocks.end())
		THROW_CORE_EXCEPTION(eErr_InvalidArgument);

	// remove the entries from unused buffers
	spAllocBlock->ReleaseChunks(m_listUnusedBuffers);

	// if some buffers are still in use - mark the whole block as additional cleaning needed
	if(!spAllocBlock->AreAllChunksFree())
	{
		if((*iterBlock).second == eBlock_Active)
		{
			(*iterBlock).second = eBlock_ToFree;
			++m_stAllocBlocksToFree;
		}
	}
	else
	{
		if((*iterBlock).second == eBlock_ToFree)
			--m_stAllocBlocksToFree;
		m_vVirtualAllocBlocks.erase(iterBlock);
		return true;	// erased
	}

	return false;	// not erased
}

void TDataBufferManager::ReclaimPage(const details::TVirtualAllocMemoryBlockPtr& spAllocBlock)
{
	// locate the entry
	MemoryBlocksVector::iterator iterBlock = std::find_if(m_vVirtualAllocBlocks.begin(), m_vVirtualAllocBlocks.end(),
		boost::bind(&std::pair<details::TVirtualAllocMemoryBlockPtr, EBlockState>::first, _1) == spAllocBlock);

	if(iterBlock == m_vVirtualAllocBlocks.end())
		THROW_CORE_EXCEPTION(eErr_InvalidArgument);

	// remove the entries from unused buffers
	if((*iterBlock).second == eBlock_ToFree)
	{
		spAllocBlock->GetFreeChunks(m_listUnusedBuffers);
		(*iterBlock).second = eBlock_Active;
		--m_stAllocBlocksToFree;
	}
}

bool TDataBufferManager::GetFreeBuffer(TSimpleDataBuffer& rSimpleBuffer)
{
	if(m_listUnusedBuffers.empty())
	{
		// try to alloc new page; we won't get one if max memory would be exceeded or allocation failed
		// this one also populates the buffers list
		if(CanAllocPage())
		{
			if(!AllocNewPage())
				THROW_CORE_EXCEPTION(eErr_CannotAllocateMemory);
		}
	}

	if(!m_listUnusedBuffers.empty())
	{
		LPVOID pBuffer = m_listUnusedBuffers.front();
		m_listUnusedBuffers.pop_front();
		rSimpleBuffer.Initialize(*this, pBuffer, m_stBufferSize);
		return true;
	}

	return false;
}

void TDataBufferManager::ReleaseBuffer(TSimpleDataBuffer& rSimpleBuffer)
{
	if(rSimpleBuffer.m_pBuffer)
	{
		if(m_stAllocBlocksToFree != 0)
		{
			// return the buffer to the rightful owner in case we're trying to reduce memory footprint
			for(MemoryBlocksVector::iterator iterBlock = m_vVirtualAllocBlocks.begin(); iterBlock != m_vVirtualAllocBlocks.end() && (*iterBlock).second == eBlock_ToFree; ++iterBlock)
			{
				const details::TVirtualAllocMemoryBlockPtr& spAllocBlock = (*iterBlock).first;
				if(spAllocBlock->IsChunkOwner(rSimpleBuffer.m_pBuffer))
				{
					spAllocBlock->ReleaseChunk(rSimpleBuffer.m_pBuffer);
					if(spAllocBlock->AreAllChunksFree())
					{
						--m_stAllocBlocksToFree;
						m_vVirtualAllocBlocks.erase(iterBlock);
					}

					return;
				}
			}

			// at this point we know that the buffer belongs to an active page
			// and at the same time we have some pages still to be freed
			m_listUnusedBuffers.push_back(rSimpleBuffer.m_pBuffer);
			ReorganizePages();
			return;
		}

		m_listUnusedBuffers.push_back(rSimpleBuffer.m_pBuffer);
	}
}

void TDataBufferManager::ReorganizePages()
{
	// prepare sorted pages
	std::vector<std::pair<details::TVirtualAllocMemoryBlockPtr, size_t> > vFreeBuffers;
	for(MemoryBlocksVector::iterator iterAllocBlock = m_vVirtualAllocBlocks.begin();
		iterAllocBlock != m_vVirtualAllocBlocks.end();
		++iterAllocBlock)
	{
		vFreeBuffers.push_back(std::make_pair((*iterAllocBlock).first, (*iterAllocBlock).first->CountOwnChunks(m_listUnusedBuffers)));
	}

	// sort by the count of free blocks
	std::sort(vFreeBuffers.begin(), vFreeBuffers.end(),
		boost::bind(&std::pair<details::TVirtualAllocMemoryBlockPtr, size_t>::second, _1) > boost::bind(&std::pair<details::TVirtualAllocMemoryBlockPtr, size_t>::second, _2));

	// and free pages with the most free blocks inside
	size_t stPagesToFree = std::min(m_stAllocBlocksToFree, vFreeBuffers.size());
	
	size_t stIndex = 0;
	for(; stIndex < stPagesToFree; ++stIndex)
	{
		FreePage(vFreeBuffers[stIndex].first);
	}

	// activate some other pages
	size_t stPagesToActivate = std::min(stIndex + m_stAllocBlocksToFree, vFreeBuffers.size());
	for(; stIndex < stPagesToActivate; ++stIndex)
	{
		ReclaimPage(vFreeBuffers[stIndex].first);
	}
}

void TDataBufferManager::FreeAllAllocBlocks()
{
	for(MemoryBlocksVector::iterator iterAllocBlock = m_vVirtualAllocBlocks.begin(); iterAllocBlock != m_vVirtualAllocBlocks.end(); ++iterAllocBlock)
	{
		(*iterAllocBlock).first->ReleaseChunks(m_listUnusedBuffers);
		_ASSERTE((*iterAllocBlock).first->AreAllChunksFree());	// without throwing on this condition, because there might be a situation that
															// some hanged thread did not release the buffer
	}

	_ASSERTE(m_listUnusedBuffers.empty());	// and all buffers should be returned to the pool by the caller
	if(!m_listUnusedBuffers.empty())
		THROW_CORE_EXCEPTION(eErr_InternalProblem);

	m_vVirtualAllocBlocks.clear();

	m_stBufferSize = 0;
	m_stPageSize = 0;
	m_stMaxMemory = 0;
}

END_CHCORE_NAMESPACE
