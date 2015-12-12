// ============================================================================
//  Copyright (C) 2001-2015 by Jozef Starosczyk
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
#include "TStringPatternArray.h"
#include "TCoreException.h"
#include "ErrorCodes.h"
#include "TStringArray.h"

namespace chcore
{
	TStringPatternArray::TStringPatternArray()
	{
	}

	TStringPatternArray::~TStringPatternArray()
	{
	}

	void TStringPatternArray::Add(const TStringPattern& strPattern)
	{
		m_vPatterns.push_back(strPattern);
	}

	void TStringPatternArray::InsertAt(size_t stIndex, const TStringPattern& strPattern)
	{
		if (stIndex > m_vPatterns.size())
			THROW_CORE_EXCEPTION(eErr_BoundsExceeded);

		m_vPatterns.insert(m_vPatterns.begin() + stIndex, strPattern);
	}

	void TStringPatternArray::SetAt(size_t stIndex, const TStringPattern& strPattern)
	{
		if (stIndex >= m_vPatterns.size())
			THROW_CORE_EXCEPTION(eErr_BoundsExceeded);

		m_vPatterns[stIndex] = strPattern;
	}

	void TStringPatternArray::RemoveAt(size_t stIndex)
	{
		if (stIndex >= m_vPatterns.size())
			THROW_CORE_EXCEPTION(eErr_BoundsExceeded);

		m_vPatterns.erase(m_vPatterns.begin() + stIndex);
	}

	void TStringPatternArray::Clear()
	{
		m_vPatterns.clear();
	}

	const TStringPattern& TStringPatternArray::GetAt(size_t stIndex) const
	{
		if (stIndex >= m_vPatterns.size())
			THROW_CORE_EXCEPTION(eErr_BoundsExceeded);

		return m_vPatterns[stIndex];
	}

	size_t TStringPatternArray::GetCount() const
	{
		return m_vPatterns.size();
	}

	bool TStringPatternArray::MatchesAny(const TString& strTextToMatch) const
	{
		for (const TStringPattern& pattern : m_vPatterns)
		{
			if (pattern.Matches(strTextToMatch))
				return true;
		}

		return false;
	}

	bool TStringPatternArray::MatchesAll(const TString& strTextToMatch) const
	{
		for (const TStringPattern& pattern : m_vPatterns)
		{
			if (!pattern.Matches(strTextToMatch))
				return false;
		}

		return true;
	}

	void TStringPatternArray::FromStringArray(const TStringArray& arrSerializedPatterns)
	{
		m_vPatterns.clear();

		for (size_t stIndex = 0; stIndex < arrSerializedPatterns.GetCount(); ++stIndex)
		{
			m_vPatterns.push_back(TStringPattern::CreateFromSerializedString(arrSerializedPatterns.GetAt(stIndex)));
		}
	}

	TStringArray TStringPatternArray::ToStringArray() const
	{
		TStringArray arrSerialized;
		for (const TStringPattern& pattern : m_vPatterns)
		{
			arrSerialized.Add(pattern.ToSerializedString());
		}

		return arrSerialized;
	}
}
