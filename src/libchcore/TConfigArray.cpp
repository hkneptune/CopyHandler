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
#include "TConfigArray.h"

namespace chcore
{
	/////////////////////////////////////////////////////////////////////////////////////////////
	// class TConfigArray

	TConfigArray::TConfigArray()
	{
	}

	TConfigArray::TConfigArray(const TConfigArray& rSrc) :
		m_vConfigs(rSrc.m_vConfigs)
	{
	}

	TConfigArray::~TConfigArray()
	{
	}

	TConfigArray& TConfigArray::operator=(const TConfigArray& rSrc)
	{
		if (this != &rSrc)
		{
			m_vConfigs = rSrc.m_vConfigs;
		}

		return *this;
	}

	size_t TConfigArray::GetCount() const
	{
		return m_vConfigs.size();
	}

	bool TConfigArray::IsEmpty() const
	{
		return m_vConfigs.empty();
	}

	const TConfig& TConfigArray::GetAt(size_t stIndex) const
	{
		return m_vConfigs[stIndex];
	}

	TConfig& TConfigArray::GetAt(size_t stIndex)
	{
		return m_vConfigs[stIndex];
	}

	void TConfigArray::Add(const TConfig& rSrc)
	{
		m_vConfigs.push_back(rSrc);
	}

	void TConfigArray::RemoveAt(size_t stIndex)
	{
		m_vConfigs.erase(m_vConfigs.begin() + stIndex);
	}

	void TConfigArray::Clear()
	{
		m_vConfigs.clear();
	}
}
