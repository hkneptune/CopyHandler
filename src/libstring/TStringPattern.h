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
#ifndef __TSTRINGPATTERN_H__
#define __TSTRINGPATTERN_H__

#include "TString.h"

namespace string
{
	class LIBSTRING_API TStringPattern
	{
	public:
		enum class EPatternType
		{
			eType_Wildcard
		};

	public:
		explicit TStringPattern(EPatternType ePatternType = EPatternType::eType_Wildcard);
		explicit TStringPattern(const TString& strPattern, EPatternType ePatternType = EPatternType::eType_Wildcard);

		void SetPattern(const TString& strPattern, EPatternType ePatternType = EPatternType::eType_Wildcard);
		bool Matches(const TString& strTextToMatch) const;

		EPatternType GetPatternType() const { return m_ePatternType; }
		TString GetPattern() const { return m_strPattern; }

		// string parsing
		static TStringPattern CreateFromString(const TString& strPattern, EPatternType eDefaultPatternType = EPatternType::eType_Wildcard);

		void FromString(const TString& strPattern, EPatternType eDefaultPatternType = EPatternType::eType_Wildcard);
		TString ToString() const;

	private:
		bool MatchMask(LPCTSTR lpszMask, LPCTSTR lpszString) const;
		bool Scan(LPCTSTR& lpszMask, LPCTSTR& lpszString) const;

	private:
		TString m_strPattern;
		EPatternType m_ePatternType;
	};
}

#endif
