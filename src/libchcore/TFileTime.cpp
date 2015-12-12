// ============================================================================
//  Copyright (C) 2001-2014 by Jozef Starosczyk
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
#include "TFileTime.h"

namespace chcore
{
	TFileTime::TFileTime()
	{
		m_ftTime.dwHighDateTime = 0;
		m_ftTime.dwLowDateTime = 0;
	}

	TFileTime::TFileTime(const TFileTime& rSrc) :
		m_ftTime(rSrc.m_ftTime)
	{
	}

	TFileTime::TFileTime(const FILETIME& rftTime) :
		m_ftTime(rftTime)
	{
	}

	TFileTime::~TFileTime()
	{
	}

	TFileTime& TFileTime::operator=(const TFileTime& rSrc)
	{
		if (this != &rSrc)
		{
			m_ftTime = rSrc.m_ftTime;
		}

		return *this;
	}

	TFileTime& TFileTime::operator=(const FILETIME& rSrc)
	{
		m_ftTime = rSrc;

		return *this;
	}


	bool TFileTime::operator==(const TFileTime& rSrc) const
	{
		return m_ftTime.dwHighDateTime == rSrc.m_ftTime.dwHighDateTime && m_ftTime.dwLowDateTime == rSrc.m_ftTime.dwLowDateTime;
	}

	bool TFileTime::operator!=(const TFileTime& rSrc) const
	{
		return m_ftTime.dwHighDateTime != rSrc.m_ftTime.dwHighDateTime || m_ftTime.dwLowDateTime != rSrc.m_ftTime.dwLowDateTime;
	}

	void TFileTime::FromUInt64(unsigned long long ullTime)
	{
		ULARGE_INTEGER uli;
		uli.QuadPart = ullTime;
		m_ftTime.dwLowDateTime = uli.LowPart;
		m_ftTime.dwHighDateTime = uli.HighPart;
	}

	unsigned long long TFileTime::ToUInt64() const
	{
		ULARGE_INTEGER uli;
		uli.HighPart = m_ftTime.dwHighDateTime;
		uli.LowPart = m_ftTime.dwLowDateTime;

		return uli.QuadPart;
	}

	const FILETIME& TFileTime::GetAsFiletime() const
	{
		return m_ftTime;
	}
}
