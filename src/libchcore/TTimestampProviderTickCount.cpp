// ============================================================================
//  Copyright (C) 2001-2013 by Jozef Starosczyk
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
#include "TTimestampProviderTickCount.h"

namespace chcore
{
	TTimestampProviderTickCount::TTimestampProviderTickCount() :
		m_ullTimestampAdjustment(0),
		m_dwLastTimestamp(0)
	{
	}

	unsigned long long TTimestampProviderTickCount::GetCurrentTimestamp() const
	{
		DWORD dwTimestamp = GetTickCount();
		if (dwTimestamp < m_dwLastTimestamp)
		{
			m_ullTimestampAdjustment += (1ULL << 32);
		}
		m_dwLastTimestamp = dwTimestamp;

		return m_ullTimestampAdjustment + dwTimestamp;
	}
}
