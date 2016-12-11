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
#include "TOverlappedProcessorRange.h"

namespace chengine
{
	TOverlappedProcessorRange::TOverlappedProcessorRange()
	{
	}

	TOverlappedProcessorRange::TOverlappedProcessorRange(unsigned long long ullResumePosition) :
		m_ullResumePosition(ullResumePosition)
	{
	}

	void TOverlappedProcessorRange::SetResumePosition(unsigned long long ullResumePosition)
	{
		if(m_ullResumePosition != ullResumePosition)
		{
			m_ullResumePosition = ullResumePosition;
			m_notifier(ullResumePosition);
		}
	}

	unsigned long long TOverlappedProcessorRange::GetResumePosition() const
	{
		return m_ullResumePosition;
	}

	boost::signals2::signal<void(unsigned long long)>& TOverlappedProcessorRange::GetNotifier()
	{
		return m_notifier;
	}
}
