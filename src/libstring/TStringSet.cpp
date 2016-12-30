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
/// @file  TStringSet.cpp
/// @date  2011/06/05
/// @brief Contains implementation of string set.
// ============================================================================
#include "stdafx.h"
#include "TStringSet.h"

namespace string
{
	///////////////////////////////////////////////////////////////////////////////////////////
	// class TStringSetIterator

	TStringSetIterator::TStringSetIterator(std::set<TString>::iterator iterSet) :
		m_iterSet(iterSet)
	{
	}

	TStringSetIterator::TStringSetIterator()
	{

	}

	TStringSetIterator::~TStringSetIterator()
	{

	}

	TStringSetIterator TStringSetIterator::operator++(int)
	{
		TStringSetIterator iterCurrent(m_iterSet);
		++m_iterSet;
		return iterCurrent;
	}

	TStringSetIterator& TStringSetIterator::operator++()
	{
		++m_iterSet;
		return *this;
	}

	bool TStringSetIterator::operator==(const TStringSetIterator& rSrc) const
	{
		return m_iterSet == rSrc.m_iterSet;
	}

	bool TStringSetIterator::operator!=(const TStringSetIterator& rSrc) const
	{
		return m_iterSet != rSrc.m_iterSet;
	}

	const TString& TStringSetIterator::operator*() const
	{
		return *m_iterSet;
	}

	///////////////////////////////////////////////////////////////////////////////////////////
	// class TStringSetConstIterator

	TStringSetConstIterator::TStringSetConstIterator(std::set<TString>::const_iterator iterSet) :
		m_iterSet(iterSet)
	{
	}

	TStringSetConstIterator::TStringSetConstIterator()
	{

	}

	TStringSetConstIterator::~TStringSetConstIterator()
	{

	}

	TStringSetConstIterator TStringSetConstIterator::operator++(int)
	{
		TStringSetConstIterator iterCurrent(m_iterSet);
		++m_iterSet;
		return iterCurrent;
	}

	TStringSetConstIterator& TStringSetConstIterator::operator++()
	{
		++m_iterSet;
		return *this;
	}

	bool TStringSetConstIterator::operator==(const TStringSetConstIterator& rSrc) const
	{
		return m_iterSet == rSrc.m_iterSet;
	}

	bool TStringSetConstIterator::operator!=(const TStringSetConstIterator& rSrc) const
	{
		return m_iterSet != rSrc.m_iterSet;
	}

	const TString& TStringSetConstIterator::operator*() const
	{
		return *m_iterSet;
	}

	///////////////////////////////////////////////////////////////////////////////////////////
	// class TStringSet
	void TStringSet::Insert(const TString& str)
	{
		m_setItems.insert(str);
	}

	void TStringSet::Insert(const TStringSet& setStrings)
	{
		m_setItems.insert(setStrings.m_setItems.begin(), setStrings.m_setItems.end());
	}

	void TStringSet::Remove(const TString& str)
	{
		std::set<TString>::iterator iter = m_setItems.find(str);
		if (iter != m_setItems.end())
			m_setItems.erase(iter);
	}

	void TStringSet::Clear()
	{
		m_setItems.clear();
	}

	bool TStringSet::HasValue(const TString& str) const
	{
		std::set<TString>::const_iterator iter = m_setItems.find(str);
		return (iter != m_setItems.end());
	}

	size_t TStringSet::GetCount() const
	{
		return m_setItems.size();
	}

	bool TStringSet::IsEmpty() const
	{
		return m_setItems.empty();
	}

	TStringSet::iterator TStringSet::begin()
	{
		return TStringSetIterator(m_setItems.begin());
	}

	TStringSet::iterator TStringSet::end()
	{
		return TStringSetIterator(m_setItems.end());
	}

	TStringSet::const_iterator TStringSet::begin() const
	{
		return TStringSetConstIterator(m_setItems.begin());
	}

	TStringSet::const_iterator TStringSet::end() const
	{
		return TStringSetConstIterator(m_setItems.end());
	}

	TStringSet::const_iterator TStringSet::cbegin() const
	{
		return TStringSetConstIterator(m_setItems.begin());
	}

	TStringSet::const_iterator TStringSet::cend() const
	{
		return TStringSetConstIterator(m_setItems.end());
	}
}
