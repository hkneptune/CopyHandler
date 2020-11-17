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
#pragma once

#include "TStringPattern.h"
#include "../common/GenericTemplates/RandomAccessIterators.h"
#include "../common/GenericTemplates/RandomAccessContainerWrapper.h"

namespace chcore
{
	template class LIBCHCORE_API RandomAccessIteratorWrapper<TStringPattern>;
	class LIBCHCORE_API TStringPatternArrayIterator : public RandomAccessIteratorWrapper<TStringPattern>
	{
	};

	template class LIBCHCORE_API RandomAccessConstIteratorWrapper<TStringPattern>;
	class LIBCHCORE_API TStringPatternArrayConstIterator : public RandomAccessConstIteratorWrapper<TStringPattern>
	{
	};

	template class LIBCHCORE_API RandomAccessContainerWrapper<TStringPattern>;

	class LIBCHCORE_API TStringPatternArray : public RandomAccessContainerWrapper<TStringPattern>
	{
	public:
		// pattern api
		bool MatchesAny(const string::TString& strTextToMatch) const;
		bool MatchesAll(const string::TString& strTextToMatch) const;

		// string parsing
		void FromString(const string::TString& strPatterns, TStringPattern::EPatternType eDefaultPatternType = TStringPattern::EPatternType::eType_Wildcard);
		void FromStringArray(const string::TStringArray& strPatterns, TStringPattern::EPatternType eDefaultPatternType = TStringPattern::EPatternType::eType_Wildcard);
		string::TString ToString() const;

		// serialization
		void FromSerializedStringArray(const string::TStringArray& arrSerializedPatterns);
		string::TStringArray ToSerializedStringArray() const;
	};
}
