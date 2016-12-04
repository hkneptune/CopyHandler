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
#include "UpdateResponse.h"
#include <codecvt>
#include <locale>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <sstream>

UpdateResponse::UpdateResponse(std::stringstream& tDataStream)
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> conversion;
	std::wstringstream wssData(conversion.from_bytes(tDataStream.str()));

	using namespace boost::property_tree;
	wiptree pt;
	read_xml(wssData, pt);

	// add version information; note that assumption here that we receive version informations
	// sorted by stability (decreasing) - that way we only present the user suggestions to download
	// newest versions with highest stability
	unsigned long long ullLastVersionNumber = 0;
	for(const wiptree::value_type& node : pt.get_child(L"UpdateInfo"))
	{
		try
		{
			UpdateVersionInfo::EVersionType eType = ParseVersionName(node.first);
			UpdateVersionInfo vi = ParseVersionInfo(node.second);

			if(vi.GetFullNumericVersion() > ullLastVersionNumber)
				m_tVersions.Add(eType, std::move(vi));

			ullLastVersionNumber = vi.GetFullNumericVersion();
		}
		catch(const std::exception&)	// ignore exceptions from version parsing code
		{
		}
	}
}

UpdateMultipleVersionInfo& UpdateResponse::GetVersions()
{
	return m_tVersions;
}

UpdateVersionInfo::EVersionType UpdateResponse::ParseVersionName(const std::wstring& strVersionName)
{
	if(boost::iequals(strVersionName, L"Stable"))
		return UpdateVersionInfo::eStable;
	if(boost::iequals(strVersionName, L"RC"))
		return UpdateVersionInfo::eReleaseCandidate;
	if(boost::iequals(strVersionName, L"Beta"))
		return UpdateVersionInfo::eBeta;
	if(boost::iequals(strVersionName, L"Alpha"))
		return UpdateVersionInfo::eAlpha;

	throw std::runtime_error("Unknown version received");
}

UpdateVersionInfo UpdateResponse::ParseVersionInfo(const boost::property_tree::wiptree& node)
{
	return UpdateVersionInfo(node.get<std::wstring>(L"NumericVersion"),
		node.get<std::wstring>(L"ReadableVersion"),
		ConvertDate(node.get<std::wstring>(L"ReleaseDateUtc")),
		node.get<std::wstring>(L"DownloadLink"),
		node.get<std::wstring>(L"ReleaseNotes"));
}

boost::gregorian::date UpdateResponse::ConvertDate(const std::wstring& wstrReleaseDate)
{
	using namespace boost::gregorian;

	std::wstringstream ss;
	wdate_input_facet * fac = new wdate_input_facet(L"%Y-%m-%d");
	ss.imbue(std::locale(std::locale::classic(), fac));

	boost::gregorian::date dtRelease;
	ss << wstrReleaseDate;
	ss >> dtRelease;

	return dtRelease;
}
