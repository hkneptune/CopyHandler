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
#ifndef __UPDATEVERSIONINFO_H__
#define __UPDATEVERSIONINFO_H__

#include <boost/date_time/date.hpp>
#include <boost/property_tree/ptree.hpp>

class UpdateVersionInfo
{
public:
	enum EVersionType
	{
		eStable,
		eReleaseCandidate,
		eBeta,
		eAlpha,

		eMax
	};

public:
	UpdateVersionInfo();
	UpdateVersionInfo(std::wstring strNumericVersion, std::wstring strReadableVersion, boost::gregorian::date dateRelease, std::wstring strDownloadLink, std::wstring strReleaseNotes);

	void Merge(UpdateVersionInfo vi);

	unsigned long long GetFullNumericVersion() const;
	std::wstring GetNumericVersion() const;
	void SetNumericVersion(std::wstring val);

	std::wstring GetReadableVersion() const;
	void SetReadableVersion(std::wstring val);

	boost::gregorian::date GetDateRelease() const;
	void SetDateRelease(boost::gregorian::date val);

	std::wstring GetDownloadLink() const;
	void SetDownloadLink(std::wstring val);

	std::wstring GetReleaseNotes() const;
	void SetReleaseNotes(std::wstring val);

private:
	static unsigned long long ParseNumericVersion(const std::wstring& wstrNumericVersion);

private:
	std::wstring m_strNumericVersion;
	unsigned long long m_ullNumericVersion = 0;

	std::wstring m_strReadableVersion;
	boost::gregorian::date m_dateRelease;

	std::wstring m_strDownloadLink;
	std::wstring m_strReleaseNotes;
};

#endif
