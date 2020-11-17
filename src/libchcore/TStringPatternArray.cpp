// ============================================================================
//  Copyright (C) 2001-2020 by Jozef Starosczyk
//  ixen {at} copyhandler [dot] com
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
#include "../libstring/TStringArray.h"

using namespace string;

namespace chcore
{
	bool TStringPatternArray::MatchesAny(const TSmartPath& pathToMatch) const
	{
		for (const TStringPattern& pattern : m_vItems)
		{
			if (pattern.Matches(pathToMatch))
				return true;
		}

		return false;
	}

	bool TStringPatternArray::MatchesAll(const TSmartPath& pathToMatch) const
	{
		for (const TStringPattern& pattern : m_vItems)
		{
			if (!pattern.Matches(pathToMatch))
				return false;
		}

		return true;
	}

	void TStringPatternArray::FromString(const TString& strPatterns)
	{
		TStringArray arrPatterns;
		strPatterns.Split(_T("|"), arrPatterns);
		FromStringArray(arrPatterns);
	}

	void TStringPatternArray::FromSerializedStringArray(const TStringArray& arrSerializedPatterns)
	{
		m_vItems.clear();

		for (size_t stIndex = 0; stIndex < arrSerializedPatterns.GetCount(); ++stIndex)
		{
			m_vItems.push_back(TStringPattern::CreateFromString(arrSerializedPatterns.GetAt(stIndex)));
		}
	}

	void TStringPatternArray::FromStringArray(const TStringArray& arrPatterns)
	{
		for (size_t stIndex = 0; stIndex < arrPatterns.GetCount(); ++stIndex)
		{
			Add(TStringPattern::CreateFromString(arrPatterns.GetAt(stIndex)));
		}
	}

	TString TStringPatternArray::ToString() const
	{
		TString strMask;
		size_t stCount = GetCount();
		if (stCount > 0)
		{
			strMask = GetAt(0).ToString();
			for (size_t stIndex = 1; stIndex < stCount; stIndex++)
			{
				strMask += _T("|") + GetAt(stIndex).ToString();
			}
		}

		return strMask;
	}

	TStringArray TStringPatternArray::ToSerializedStringArray() const
	{
		TStringArray arrSerialized;
		for (const TStringPattern& pattern : m_vItems)
		{
			arrSerialized.Add(pattern.ToString());
		}

		return arrSerialized;
	}
}
