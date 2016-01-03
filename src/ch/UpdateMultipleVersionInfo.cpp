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
#include "UpdateMultipleVersionInfo.h"
#include "../Common/version.h"

UpdateMultipleVersionInfo::UpdateMultipleVersionInfo()
{
}

void UpdateMultipleVersionInfo::Add(UpdateVersionInfo::EVersionType eType, UpdateVersionInfo vi)
{
	auto iterFind = m_mapVersions.find(eType);
	if(iterFind != m_mapVersions.end())
		iterFind->second.Merge(vi);
	else
		m_mapVersions.emplace(eType, std::move(vi));
}

bool UpdateMultipleVersionInfo::FindUpdateInfo(UpdateVersionInfo::EVersionType eUpdateChannel, UpdateVersionInfo& rOutVersionInfo) const
{
	unsigned long long ullFoundVersion = 0;
	const unsigned long long ullCurrentProgramVersion = PRODUCT_FULL_NUMERIC_VERSION;

	for(const std::pair<UpdateVersionInfo::EVersionType, UpdateVersionInfo>& pairInfo : m_mapVersions)
	{
		// ignore channels not fitting our settings
		if(eUpdateChannel >= pairInfo.first && 
			pairInfo.second.GetFullNumericVersion() > ullCurrentProgramVersion &&
			pairInfo.second.GetFullNumericVersion() > ullFoundVersion)
		{
			rOutVersionInfo = pairInfo.second;
			ullFoundVersion = pairInfo.second.GetFullNumericVersion();
		}
	}

	return ullFoundVersion > 0;
}
