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
#include <vector>
#include <list>

BEGIN_CHCORE_NAMESPACE

class TDataBufferManager;

class TSimpleDataBuffer
{
public:
	TSimpleDataBuffer();
	~TSimpleDataBuffer();

	LPVOID GetBufferPtr();
	void ReleaseBuffer();

private:
	TSimpleDataBuffer(const TSimpleDataBuffer&);
	TSimpleDataBuffer& operator=(const TSimpleDataBuffer&);

	void Initialize(TDataBufferManager& rBufferManager, LPVOID pBuffer);

private:
	LPVOID m_pBuffer;
	TDataBufferManager* m_pBufferManager;

	friend class TDataBufferManager;
};

class TDataBufferManager
{
public:
	TDataBufferManager();
	~TDataBufferManager();

	void Initialize(size_t stBufferSize, size_t stBlockSize, size_t stSimpleBufferSize);

	bool HasFreeBuffer() const;
	bool GetFreeBuffer(TSimpleDataBuffer& rSimpleBuffer);
	void ReleaseBuffer(TSimpleDataBuffer& rSimpleBuffer);

private:
	void FreeBuffers();

private:

	std::vector<LPVOID> m_vVirtualAllocBlocks;
	std::list<LPVOID> m_listUnusedSimpleBuffers;
	
	size_t m_stAllocBlockSize;	// size of the memory block in m_vVirtualAllocBlocks
	size_t m_stChunkSize;		// size of the simple buffer (part of the real buffer)
	size_t m_stCountOfSimpleBuffers;
};

END_CHCORE_NAMESPACE

#endif
