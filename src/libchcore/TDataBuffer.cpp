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

BEGIN_CHCORE_NAMESPACE

namespace
{
	const size_t c_DefaultAllocGranularity = 4096;
	const size_t c_DefaultBufferSize = 65536;
	const size_t c_DefaultPageSize = 1024*1024;
	const size_t c_DefaultMaxMemory = 1024*1024;

	template<class T> T RoundUp(T number, T roundValue) { return ((number + roundValue - 1) & ~(roundValue - 1)); }
}

///////////////////////////////////////////////////////////////////////////////////
// class TSimpleDataBuffer

TSimpleDataBuffer::TSimpleDataBuffer() :
	m_pBuffer(NULL),
	m_pBufferManager(NULL),
	m_stBufferSize(0)
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

///////////////////////////////////////////////////////////////////////////////////
// class TDataBufferManager

TDataBufferManager::TDataBufferManager() :
	m_stMaxMemory(0),
	m_stPageSize(0),
	m_stBufferSize(0)
{
}

TDataBufferManager::~TDataBufferManager()
{
	FreeBuffers();
}

bool TDataBufferManager::CheckBufferConfig(size_t& stMaxMemory, size_t& stPageSize, size_t& stBufferSize)
{
	bool bResult = true;

	// first the user-facing buffer size
	if(stBufferSize == 0)
	{
		stBufferSize = c_DefaultMaxMemory;
		bResult = false;
	}
	else
	{
		size_t stNewSize = RoundUp(stBufferSize, c_DefaultAllocGranularity);
		if(stBufferSize != stNewSize)
		{
			stBufferSize = stNewSize;
			bResult = false;
		}
	}

	// now the page size
	if(stPageSize == 0)
	{
		stPageSize = std::max(c_DefaultPageSize, RoundUp(c_DefaultPageSize, stBufferSize));
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
		stMaxMemory = std::max(c_DefaultMaxMemory, RoundUp(c_DefaultMaxMemory, stPageSize));
		bResult = false;
	}
	else if(stMaxMemory < stPageSize)
	{
		size_t stNewSize = RoundUp(stMaxMemory, stBufferSize);
		if(stNewSize != stMaxMemory)
		{
			bResult = false;
			stMaxMemory = stPageSize;
		}
	}

	return bResult;
}

bool TDataBufferManager::CheckBufferConfig(size_t& stMaxMemory)
{
	size_t stDefaultPageSize = c_DefaultPageSize;
	size_t stDefaultBufferSize = c_DefaultBufferSize;
	return CheckBufferConfig(stMaxMemory, stDefaultPageSize, stDefaultBufferSize);
}

void TDataBufferManager::Initialize(size_t stMaxMemory)
{
	Initialize(stMaxMemory, c_DefaultPageSize, c_DefaultBufferSize);
}

void TDataBufferManager::Initialize(size_t stMaxMemory, size_t stPageSize, size_t stBufferSize)
{
	FreeBuffers();

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

bool TDataBufferManager::HasFreeBuffer() const
{
	return !m_listUnusedBuffers.empty();
}

bool TDataBufferManager::CanAllocPage() const
{
	if(!IsInitialized())
		return false;

	size_t stMaxPages = m_stMaxMemory / m_stPageSize;
	return m_vVirtualAllocBlocks.size() < stMaxPages;
}

bool TDataBufferManager::GetFreeBuffer(TSimpleDataBuffer& rSimpleBuffer)
{
	if(m_listUnusedBuffers.empty())
	{
		// try to alloc new page; we won't get one if max memory would be exceeded or allocation failed
		// this one also populates the buffers list
		if(!AllocNewPage())
			return false;
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
		m_listUnusedBuffers.push_back(rSimpleBuffer.m_pBuffer);
}

void TDataBufferManager::FreeBuffers()
{
	// check if all buffers were returned to the pool
	size_t stTotalBufferCount = m_stMaxMemory / m_stBufferSize;

	_ASSERTE(m_listUnusedBuffers.size() == stTotalBufferCount);
	if(m_listUnusedBuffers.size() != stTotalBufferCount)
		THROW_CORE_EXCEPTION(eErr_InternalProblem);

	for(std::vector<LPVOID>::iterator iterMem = m_vVirtualAllocBlocks.begin(); iterMem != m_vVirtualAllocBlocks.end(); ++iterMem)
	{
		VirtualFree(*iterMem, 0, MEM_RELEASE);
	}

	m_vVirtualAllocBlocks.clear();
	m_listUnusedBuffers.clear();

	m_stBufferSize = 0;
	m_stPageSize = 0;
	m_stMaxMemory = 0;
}

bool TDataBufferManager::AllocNewPage()
{
	// check if we're allowed to alloc any new memory under current settings
	// there is also an initialization check inside
	if(!CanAllocPage())
		return false;

	// allocate
	LPVOID pBuffer = VirtualAlloc(NULL, m_stPageSize, MEM_COMMIT, PAGE_READWRITE);
	if(!pBuffer)
		return false;

	m_vVirtualAllocBlocks.push_back(pBuffer);

	// slice the page to buffers
	size_t stSliceCount = m_stPageSize / m_stBufferSize;
	for(size_t stIndex = 0; stIndex < stSliceCount; ++stIndex)
	{
		LPVOID pSimpleBuffer = (BYTE*)pBuffer + stIndex * m_stBufferSize;
		m_listUnusedBuffers.push_back(pSimpleBuffer);
	}

	return true;
}

END_CHCORE_NAMESPACE
