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
#ifndef __UPDATERESPONSE_H__
#define __UPDATERESPONSE_H__

#include "UpdateVersionInfo.h"
#include "UpdateMultipleVersionInfo.h"

class UpdateResponse
{
public:
	UpdateResponse(std::stringstream& tDataStream);

	UpdateMultipleVersionInfo& GetVersions();

private:
	UpdateVersionInfo::EVersionType ParseVersionName(const std::wstring& strVersionName);
	UpdateVersionInfo ParseVersionInfo(const boost::property_tree::wiptree& node);

	boost::gregorian::date ConvertDate(const std::wstring& wstrReleaseDate);

private:
	UpdateMultipleVersionInfo m_tVersions;
};

#endif
