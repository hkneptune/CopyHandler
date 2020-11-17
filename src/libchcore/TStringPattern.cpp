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
#include "TStringPattern.h"
#include <tchar.h>
#include <regex>
#include <boost/algorithm/string/replace.hpp>

using namespace string;

namespace chcore
{
	namespace
	{
		bool _tcicmp(TCHAR c1, TCHAR c2)
		{
			TCHAR ch1[2] = { c1, 0 }, ch2[2] = { c2, 0 };
			return (_tcsicmp(ch1, ch2) == 0);
		}
	}

	TStringPattern::TStringPattern() :
		m_ePatternType(EPatternType::eType_FilenameWildcard)
	{
	}

	TStringPattern::TStringPattern(const TString& strPattern)
	{
		FromString(strPattern);
	}
	
	TStringPattern::TStringPattern(const TString& strPattern, EPatternType ePatternType) :
		m_strPattern(strPattern),
		m_ePatternType(ePatternType)
	{
	}

	TStringPattern TStringPattern::CreateFromString(const TString& strPattern)
	{
		TStringPattern pattern;
		pattern.FromString(strPattern);
		return pattern;
	}

	void TStringPattern::FromString(const TString& strPattern)
	{
		m_ePatternType = EPatternType::eType_FilenameWildcard;
		if(strPattern.StartsWith(L"file:"))
		{
			m_strPattern = strPattern.Mid(5);
			m_ePatternType = EPatternType::eType_FilenameWildcard;
		}
		else if(strPattern.StartsWith(L"path:"))
		{
			m_strPattern = strPattern.Mid(5);
			m_ePatternType = EPatternType::eType_FullPathWildcard;
		}
		else if(strPattern.StartsWith(L"rfile:"))
		{
			m_strPattern = strPattern.Mid(6);
			m_ePatternType = EPatternType::eType_FilenameRegex;
		}
		else if(strPattern.StartsWith(L"rpath:"))
		{
			m_strPattern = strPattern.Mid(6);
			m_ePatternType = EPatternType::eType_FullPathRegex;
		}
		else
			m_strPattern = strPattern;
	}

	TString TStringPattern::ToString() const
	{
		TString strPrefix;
		switch (m_ePatternType)
		{
		case EPatternType::eType_FilenameWildcard:
			break;	// wildcard won't have any prefix (it's implicit)

		case EPatternType::eType_FullPathWildcard:
			strPrefix = L"path:";
			break;

		case EPatternType::eType_FilenameRegex:
			strPrefix = L"rfile:";
			break;

		case EPatternType::eType_FullPathRegex:
			strPrefix = L"rpath:";
			break;

		default:
			throw std::invalid_argument("Pattern type not supported");
		}

		return TString(strPrefix + m_strPattern);
	}

	bool TStringPattern::operator!=(const TStringPattern& rSrc) const
	{
		return m_ePatternType != rSrc.m_ePatternType || m_strPattern != rSrc.m_strPattern;
	}

	bool TStringPattern::operator==(const TStringPattern& rSrc) const
	{
		return m_ePatternType == rSrc.m_ePatternType && m_strPattern == rSrc.m_strPattern;
	}

	bool TStringPattern::Matches(const TSmartPath& pathToMatch) const
	{
		switch(m_ePatternType)
		{
		case EPatternType::eType_FilenameWildcard:
		{
			if(m_strPattern == L"*" || m_strPattern == L"*.*")
				return true;

			std::wstring strPattern = ConvertGlobToRegex();

			std::wstring strText(pathToMatch.GetFileName().ToString());
			std::wregex pattern(strPattern, std::regex_constants::icase | std::regex_constants::ECMAScript);

			return std::regex_match(strText, pattern);
		}

		case EPatternType::eType_FullPathWildcard:
		{
			if(m_strPattern == L"*" || m_strPattern == L"*.*")
				return true;

			std::wstring strPattern = ConvertGlobToRegex();
			std::wstring strText(pathToMatch.ToString());
			std::wregex pattern(strPattern, std::regex_constants::icase | std::regex_constants::ECMAScript);

			return std::regex_match(strText, pattern);
		}

		case EPatternType::eType_FilenameRegex:
		{
			std::wstring strText(pathToMatch.GetFileName().ToString());
			std::wregex pattern(m_strPattern.c_str(), std::regex_constants::icase | std::regex_constants::ECMAScript);

			return std::regex_match(strText, pattern);
		}

		case EPatternType::eType_FullPathRegex:
		{
			std::wstring strText(pathToMatch.ToString());
			std::wregex pattern(m_strPattern.c_str(), std::regex_constants::icase | std::regex_constants::ECMAScript);

			return std::regex_match(strText, pattern);
		}

		default:
			throw std::invalid_argument("Unsupported pattern type");
		}
	}

	void TStringPattern::SetPattern(const TString& strPattern, EPatternType ePatternType)
	{
		m_ePatternType = ePatternType;
		m_strPattern = strPattern;
	}

	std::wstring TStringPattern::ConvertGlobToRegex() const
	{
		std::wstring strPattern = m_strPattern.c_str();

		boost::replace_all(strPattern, L"\\", L"\\\\");
		boost::replace_all(strPattern, L"^", L"\\^");
		boost::replace_all(strPattern, L".", L"\\.");
		boost::replace_all(strPattern, L"$", L"\\$");
		boost::replace_all(strPattern, L"|", L"\\|");
		boost::replace_all(strPattern, L"(", L"\\(");
		boost::replace_all(strPattern, L")", L"\\)");
		boost::replace_all(strPattern, L"{", L"\\{");
		boost::replace_all(strPattern, L"{", L"\\}");
		boost::replace_all(strPattern, L"[", L"\\[");
		boost::replace_all(strPattern, L"]", L"\\]");
		boost::replace_all(strPattern, L"+", L"\\+");
		boost::replace_all(strPattern, L"/", L"\\/");
		boost::replace_all(strPattern, L"*", L".*");
		boost::replace_all(strPattern, L"?", L".");

		return strPattern;
	}
}
