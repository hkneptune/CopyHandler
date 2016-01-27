// ============================================================================
//  Copyright (C) 2001-2011 by Jozef Starosczyk
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
/// @file  TStringArray.cpp
/// @date  2011/06/05
/// @brief Contains implementation of string array.
// ============================================================================
#include "stdafx.h"
#include "TStringArray.h"
#include "TCoreException.h"
#include "ErrorCodes.h"

namespace chcore
{
	///////////////////////////////////////////////////////////////////////////////////////////////////////
	// class TStringArrayIterator

	TStringArrayIterator::TStringArrayIterator(std::vector<TString>::iterator iterArray) :
		m_iterArray(iterArray)
	{
	}

	TStringArrayIterator::TStringArrayIterator()
	{
	}

	TStringArrayIterator::~TStringArrayIterator()
	{
	}

	TStringArrayIterator TStringArrayIterator::operator++(int)
	{
		TStringArrayIterator iterCurrent = *this;
		++m_iterArray;
		return iterCurrent;
	}

	TStringArrayIterator& TStringArrayIterator::operator++()
	{
		++m_iterArray;
		return *this;
	}

	bool TStringArrayIterator::operator==(const TStringArrayIterator& rSrc) const
	{
		return m_iterArray == rSrc.m_iterArray;
	}

	bool TStringArrayIterator::operator!=(const TStringArrayIterator& rSrc) const
	{
		return m_iterArray != rSrc.m_iterArray;
	}

	TString& TStringArrayIterator::operator*()
	{
		return *m_iterArray;
	}

	const TString& TStringArrayIterator::operator*() const
	{
		return *m_iterArray;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////
	// class TStringArrayConstIterator

	TStringArrayConstIterator::TStringArrayConstIterator(std::vector<TString>::const_iterator iterArray) :
		m_iterArray(iterArray)
	{
	}

	TStringArrayConstIterator::TStringArrayConstIterator()
	{
	}

	TStringArrayConstIterator::~TStringArrayConstIterator()
	{
	}

	TStringArrayConstIterator TStringArrayConstIterator::operator++(int)
	{
		TStringArrayConstIterator iterCurrent = *this;
		++m_iterArray;
		return iterCurrent;
	}

	TStringArrayConstIterator& TStringArrayConstIterator::operator++()
	{
		++m_iterArray;
		return *this;
	}

	bool TStringArrayConstIterator::operator==(const TStringArrayConstIterator& rSrc) const
	{
		return m_iterArray == rSrc.m_iterArray;
	}

	bool TStringArrayConstIterator::operator!=(const TStringArrayConstIterator& rSrc) const
	{
		return m_iterArray != rSrc.m_iterArray;
	}

	const TString& TStringArrayConstIterator::operator*()
	{
		return *m_iterArray;
	}

	const TString& TStringArrayConstIterator::operator*() const
	{
		return *m_iterArray;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////
	// class TStringArray
	TStringArray::TStringArray()
	{
	}

	TStringArray::~TStringArray()
	{
	}

	void TStringArray::Add(const TString& str)
	{
		m_vItems.push_back(str);
	}

	void TStringArray::InsertAt(size_t stIndex, const TString& str)
	{
		if (stIndex >= m_vItems.size())
			throw TCoreException(eErr_BoundsExceeded, L"stIndex", LOCATION);

		m_vItems.insert(m_vItems.begin() + stIndex, str);
	}

	void TStringArray::SetAt(size_t stIndex, const TString& str)
	{
		if (stIndex >= m_vItems.size())
			throw TCoreException(eErr_BoundsExceeded, L"stIndex", LOCATION);

		m_vItems[stIndex] = str;
	}

	void TStringArray::RemoveAt(size_t stIndex)
	{
		if (stIndex >= m_vItems.size())
			throw TCoreException(eErr_BoundsExceeded, L"stIndex", LOCATION);

		m_vItems.erase(m_vItems.begin() + stIndex);
	}

	void TStringArray::Clear()
	{
		m_vItems.clear();
	}

	const TString& TStringArray::GetAt(size_t stIndex) const
	{
		if (stIndex >= m_vItems.size())
			throw TCoreException(eErr_BoundsExceeded, L"stIndex", LOCATION);

		return m_vItems.at(stIndex);
	}

	size_t TStringArray::GetCount() const
	{
		return m_vItems.size();
	}

	TStringArrayIterator TStringArray::Begin()
	{
		return TStringArrayIterator(m_vItems.begin());
	}

	TStringArrayIterator TStringArray::End()
	{
		return TStringArrayIterator(m_vItems.end());
	}

	TStringArrayConstIterator TStringArray::Begin() const
	{
		return TStringArrayConstIterator(m_vItems.begin());
	}

	TStringArrayConstIterator TStringArray::End() const
	{
		return TStringArrayConstIterator(m_vItems.end());
	}

	bool TStringArray::operator==(const TStringArray& rSrc) const
	{
		if (rSrc.GetCount() != GetCount())
			return false;

		size_t stCount = GetCount();
		while (stCount-- > 0)
		{
			if (m_vItems[stCount] != rSrc.m_vItems[stCount])
				return false;
		}

		return true;
	}

	bool TStringArray::operator!=(const TStringArray& rSrc) const
	{
		if (rSrc.GetCount() != GetCount())
			return true;

		size_t stCount = GetCount();
		while (stCount-- > 0)
		{
			if (m_vItems[stCount] != rSrc.m_vItems[stCount])
				return true;
		}

		return false;
	}
}
