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

#include "../libstring/TString.h"
#include "libchcore.h"
#include "TPath.h"

namespace chcore
{
	class LIBCHCORE_API TStringPattern
	{
	public:
		enum class EPatternType
		{
			eType_FilenameWildcard,
			eType_FullPathWildcard,
			eType_FilenameRegex,
			eType_FullPathRegex,
		};

	public:
		TStringPattern();
		explicit TStringPattern(const string::TString& strPattern);
		explicit TStringPattern(const string::TString& strPattern, EPatternType ePatternType);

		void SetPattern(const string::TString& strPattern, EPatternType ePatternType = EPatternType::eType_FilenameWildcard);
		bool Matches(const TSmartPath& pathToMatch) const;

		EPatternType GetPatternType() const { return m_ePatternType; }
		string::TString GetPattern() const { return m_strPattern; }

		// string parsing
		static TStringPattern CreateFromString(const string::TString& strPattern);

		void FromString(const string::TString& strPattern);
		string::TString ToString() const;

		bool operator==(const TStringPattern& rSrc) const;
		bool operator!=(const TStringPattern& rSrc) const;

	private:
		std::wstring ConvertGlobToRegex() const;

	private:
		string::TString m_strPattern;
		EPatternType m_ePatternType;
	};
}
