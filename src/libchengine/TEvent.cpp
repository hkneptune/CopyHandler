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
#include "TEvent.h"
#include "../libchcore/TCoreException.h"

namespace chengine
{
	TEvent::TEvent(bool bManualReset, bool bInitialState)
	{
		m_hEvent = CreateEvent(nullptr, bManualReset, bInitialState, nullptr);
		if (m_hEvent == nullptr)
			throw chcore::TCoreException(chcore::eErr_CannotCreateEvent, L"Failed to create event", LOCATION);
#ifdef _DEBUG
		m_bSignaled = bInitialState;
#endif
	}

	TEvent::~TEvent()
	{
		CloseHandle(m_hEvent);
	}

	void TEvent::SetEvent(bool bSet)
	{
		if(bSet)
			SetEvent();
		else
			ResetEvent();
	}

	void TEvent::SetEvent()
	{
		::SetEvent(m_hEvent);
#ifdef _DEBUG
		m_bSignaled = true;
#endif
	}

	void TEvent::ResetEvent()
	{
		::ResetEvent(m_hEvent);
#ifdef _DEBUG
		m_bSignaled = false;
#endif
	}

}
