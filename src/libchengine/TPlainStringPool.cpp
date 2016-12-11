// ============================================================================
//  Copyright (C) 2001-2014 by Jozef Starosczyk
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
#include "stdafx.h"
#include "TPlainStringPool.h"
#include "../libchcore/TCoreException.h"

using namespace chcore;

namespace chengine
{
	TPlainStringPool::TPlainStringPool()
	{
	}

	TPlainStringPool::~TPlainStringPool()
	{
		Clear();
	}

	wchar_t* TPlainStringPool::Alloc(size_t stCount)
	{
		if (stCount > BlockSize)
			throw chcore::TCoreException(chcore::eErr_InvalidArgument, L"stCount", LOCATION);

		// find block where the new string would fit
		size_t stBlockCount = m_vBlocks.size();
		for (size_t stIndex = 0; stIndex < stBlockCount; ++stIndex)
		{
			if (m_vBlocks[stIndex].second >= stCount)
			{
				wchar_t* pszResult = m_vBlocks[stIndex].first + BlockSize - m_vBlocks[stIndex].second;
				m_vBlocks[stIndex].second -= stCount;
				return pszResult;
			}
		}

		wchar_t* pszBuffer = AllocNewBlock();
		m_vBlocks.back().second -= stCount;

		return pszBuffer;
	}

	wchar_t* TPlainStringPool::AllocForString(const wchar_t* pszString)
	{
		size_t stLen = wcslen(pszString) + 1;

		wchar_t* pszBuffer = Alloc(stLen);
		wmemcpy_s(pszBuffer, BlockSize, pszString, stLen);

		return pszBuffer;
	}

	void TPlainStringPool::Clear(bool bLeaveSingleEmptyBlock)
	{
		size_t stBlockCount = m_vBlocks.size();
		for (size_t stIndex = 0; stIndex < stBlockCount; ++stIndex)
		{
			if (!bLeaveSingleEmptyBlock || stIndex != 0)
			{
				delete[] m_vBlocks[stIndex].first;
			}
		}

		if (bLeaveSingleEmptyBlock)
		{
			if (m_vBlocks.size() > 1)
				m_vBlocks.erase(m_vBlocks.begin() + 1, m_vBlocks.end());
		}
		else
			m_vBlocks.clear();
	}

	wchar_t* TPlainStringPool::AllocNewBlock()
	{
		wchar_t* pszBlock = new wchar_t[BlockSize];
		m_vBlocks.push_back(std::make_pair(pszBlock, BlockSize));

		return pszBlock;
	}
}
