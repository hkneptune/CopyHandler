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
#ifndef __TSTRINGPATTERNARRAY_H__
#define __TSTRINGPATTERNARRAY_H__

#include "TStringPattern.h"

namespace chcore
{
	class LIBCHCORE_API TStringPatternArray
	{
	public:
		TStringPatternArray();
		~TStringPatternArray();

		// general api
		void Add(const TStringPattern& strPattern);
		void InsertAt(size_t stIndex, const TStringPattern& strPattern);
		void SetAt(size_t stIndex, const TStringPattern& strPattern);
		void RemoveAt(size_t stIndex);
		void Clear();

		const TStringPattern& GetAt(size_t stIndex) const;
		size_t GetCount() const;

		// pattern api
		bool MatchesAny(const TString& strTextToMatch) const;
		bool MatchesAll(const TString& strTextToMatch) const;

		// string parsing
		void FromString(const TString& strPatterns, TStringPattern::EPatternType eDefaultPatternType = TStringPattern::EPatternType::eType_Wildcard);
		void FromStringArray(const TStringArray& strPatterns, TStringPattern::EPatternType eDefaultPatternType = TStringPattern::EPatternType::eType_Wildcard);
		TString ToString() const;

		// serialization
		void FromSerializedStringArray(const TStringArray& arrSerializedPatterns);
		TStringArray ToSerializedStringArray() const;

	private:
#pragma warning(push)
#pragma warning(disable: 4251)
		std::vector<TStringPattern> m_vPatterns;
#pragma warning(pop)
	};
}

#endif
