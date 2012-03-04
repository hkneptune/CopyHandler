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
	const size_t c_DefaultSimpleBufferSize = 65536;
	const size_t c_DefaultBlockSize = 1024*1024;
	const size_t c_DefaultBufferSize = 1024*1024;

	template<class T> T RoundUp(T number, T roundValue) { return ((number + roundValue - 1) & ~(roundValue - 1)); }
}

///////////////////////////////////////////////////////////////////////////////////
// class TSimpleDataBuffer

TSimpleDataBuffer::TSimpleDataBuffer() :
	m_pBuffer(NULL),
	m_pBufferManager(NULL)
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

void TSimpleDataBuffer::Initialize(TDataBufferManager& rBufferManager, LPVOID pBuffer)
{
	ReleaseBuffer();
	m_pBufferManager = &rBufferManager;
	m_pBuffer = pBuffer;
}

///////////////////////////////////////////////////////////////////////////////////
// class TDataBufferManager

TDataBufferManager::TDataBufferManager() :
	m_stChunkSize(0),
	m_stAllocBlockSize(0),
	m_stCountOfSimpleBuffers(0)
{
}

TDataBufferManager::~TDataBufferManager()
{
	FreeBuffers();
}

void TDataBufferManager::Initialize(size_t stBufferSize, size_t stBlockSize, size_t stSimpleBufferSize)
{
	FreeBuffers();

	if(stSimpleBufferSize == 0)
		stSimpleBufferSize = c_DefaultSimpleBufferSize;
	if(stBlockSize == 0)
		stBlockSize = c_DefaultBlockSize;
	if(stBufferSize == 0)
		stBufferSize = c_DefaultBufferSize;

	m_stCountOfSimpleBuffers = 0;
	m_stChunkSize = RoundUp(stSimpleBufferSize, c_DefaultAllocGranularity);
	m_stAllocBlockSize = RoundUp(stBlockSize, m_stChunkSize);

	size_t stSimpleBuffersPerBlock = m_stAllocBlockSize / m_stChunkSize;
	size_t stBlockCount = RoundUp(stBufferSize, m_stAllocBlockSize) / m_stAllocBlockSize;

	for(size_t stIndex = 0; stIndex < stBlockCount; ++stIndex)
	{
		// allocate
		LPVOID pBuffer = VirtualAlloc(NULL, m_stAllocBlockSize, MEM_COMMIT, PAGE_READWRITE);
		if(!pBuffer)
			THROW_CORE_EXCEPTION(eErr_CannotAllocateMemory);

		m_vVirtualAllocBlocks.push_back(pBuffer);

		// and slice
		for(size_t stSimpleIndex = 0; stSimpleIndex < stSimpleBuffersPerBlock; ++stSimpleIndex)
		{
			LPVOID pSimpleBuffer = (BYTE*)pBuffer + stSimpleIndex * m_stChunkSize;
			m_listUnusedSimpleBuffers.push_back(pSimpleBuffer);
			++m_stCountOfSimpleBuffers;
		}
	}
}

bool TDataBufferManager::HasFreeBuffer() const
{
	return !m_listUnusedSimpleBuffers.empty();
}

bool TDataBufferManager::GetFreeBuffer(TSimpleDataBuffer& rSimpleBuffer)
{
	if(!m_listUnusedSimpleBuffers.empty())
	{
		LPVOID pBuffer = m_listUnusedSimpleBuffers.front();
		m_listUnusedSimpleBuffers.pop_front();
		rSimpleBuffer.Initialize(*this, pBuffer);
		return true;
	}

	return false;
}

void TDataBufferManager::ReleaseBuffer(TSimpleDataBuffer& rSimpleBuffer)
{
	if(rSimpleBuffer.m_pBuffer)
		m_listUnusedSimpleBuffers.push_back(rSimpleBuffer.m_pBuffer);
}

void TDataBufferManager::FreeBuffers()
{
	_ASSERTE(m_listUnusedSimpleBuffers.size() == m_stCountOfSimpleBuffers);
	if(m_listUnusedSimpleBuffers.size() != m_stCountOfSimpleBuffers)
		THROW_CORE_EXCEPTION(eErr_InternalProblem);

	for(std::vector<LPVOID>::iterator iterMem = m_vVirtualAllocBlocks.begin(); iterMem != m_vVirtualAllocBlocks.end(); ++iterMem)
	{
		VirtualFree(*iterMem, 0, MEM_RELEASE);
	}

	m_vVirtualAllocBlocks.clear();
	m_listUnusedSimpleBuffers.clear();
	m_stCountOfSimpleBuffers = 0;
	m_stAllocBlockSize = 0;
	m_stChunkSize = 0;
}

END_CHCORE_NAMESPACE
