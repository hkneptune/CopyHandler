// ============================================================================
//  Copyright (C) 2001-2016 by Jozef Starosczyk
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
#include "TLoggerRotationInfo.h"
#include <boost\thread\lock_types.hpp>

namespace logger
{
	TLoggerRotationInfo::TLoggerRotationInfo()
	{
	}

	TLoggerRotationInfo::TLoggerRotationInfo(unsigned long long ullMaxLogSize, unsigned long ulMaxRotatedCount) :
		m_ullMaxLogSize(ullMaxLogSize),
		m_ulMaxRotatedCount(ulMaxRotatedCount)
	{
	}

	void TLoggerRotationInfo::SetRotationInfo(unsigned long long ullMaxLogSize, unsigned long ulMaxRotatedCount)
	{
		boost::unique_lock<boost::shared_mutex> lock(m_mutex);
		m_ullMaxLogSize = ullMaxLogSize;
		m_ulMaxRotatedCount = ulMaxRotatedCount;
	}

	void TLoggerRotationInfo::GetRotationInfo(unsigned long long& ullMaxLogSize, unsigned long& ulMaxRotatedCount) const
	{
		boost::shared_lock<boost::shared_mutex> lock(m_mutex);
		ullMaxLogSize = m_ullMaxLogSize;
		ulMaxRotatedCount = m_ulMaxRotatedCount;
	}

	unsigned long long TLoggerRotationInfo::GetMaxLogSize() const
	{
		boost::shared_lock<boost::shared_mutex> lock(m_mutex);
		return m_ullMaxLogSize;
	}

	unsigned long TLoggerRotationInfo::GetMaxRotatedCount() const
	{
		boost::shared_lock<boost::shared_mutex> lock(m_mutex);
		return m_ulMaxRotatedCount;
	}
}
