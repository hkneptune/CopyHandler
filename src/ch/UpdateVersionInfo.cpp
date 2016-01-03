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
#include "UpdateVersionInfo.h"
#include <boost\algorithm\string\split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost\lexical_cast.hpp>

UpdateVersionInfo::UpdateVersionInfo(std::wstring strNumericVersion, std::wstring strReadableVersion, boost::gregorian::date dateRelease, std::wstring strDownloadLink, std::wstring strReleaseNotes) :
	m_strNumericVersion(strNumericVersion),
	m_strReadableVersion(strReadableVersion),
	m_dateRelease(dateRelease),
	m_strDownloadLink(strDownloadLink),
	m_strReleaseNotes(strReleaseNotes),
	m_ullNumericVersion(ParseNumericVersion(strNumericVersion))
{
}

UpdateVersionInfo::UpdateVersionInfo()
{

}

void UpdateVersionInfo::Merge(UpdateVersionInfo vi)
{
	if(m_ullNumericVersion < vi.m_ullNumericVersion)
		*this = std::move(vi);
}

unsigned long long UpdateVersionInfo::GetFullNumericVersion() const
{
	return m_ullNumericVersion;
}

std::wstring UpdateVersionInfo::GetNumericVersion() const
{
	return m_strNumericVersion;
}

void UpdateVersionInfo::SetNumericVersion(std::wstring val)
{
	m_strNumericVersion = val;
	m_ullNumericVersion = ParseNumericVersion(m_strNumericVersion);
}

std::wstring UpdateVersionInfo::GetReadableVersion() const
{
	return m_strReadableVersion;
}

void UpdateVersionInfo::SetReadableVersion(std::wstring val)
{
	m_strReadableVersion = val;
}

boost::gregorian::date UpdateVersionInfo::GetDateRelease() const
{
	return m_dateRelease;
}

void UpdateVersionInfo::SetDateRelease(boost::gregorian::date val)
{
	m_dateRelease = val;
}

std::wstring UpdateVersionInfo::GetDownloadLink() const
{
	return m_strDownloadLink;
}

void UpdateVersionInfo::SetDownloadLink(std::wstring val)
{
	m_strDownloadLink = val;
}

std::wstring UpdateVersionInfo::GetReleaseNotes() const
{
	return m_strReleaseNotes;
}

void UpdateVersionInfo::SetReleaseNotes(std::wstring val)
{
	m_strReleaseNotes = val;
}

unsigned long long UpdateVersionInfo::ParseNumericVersion(const std::wstring& wstrNumericVersion)
{
	std::vector<std::wstring> vParts;
	boost::split(vParts, wstrNumericVersion, boost::is_any_of("."), boost::algorithm::token_compress_on);

	if(vParts.size() != 4)
		throw std::runtime_error("Wrong version number received");

	unsigned long long ullVersion =
		(unsigned long long)boost::lexical_cast<unsigned short>(vParts[ 0 ]) << 48 |
		(unsigned long long)boost::lexical_cast<unsigned short>(vParts[ 1 ]) << 32 |
		(unsigned long long)boost::lexical_cast<unsigned short>(vParts[ 2 ]) << 16 |
		(unsigned long long)boost::lexical_cast<unsigned short>(vParts[ 3 ]);

	return ullVersion;
}
