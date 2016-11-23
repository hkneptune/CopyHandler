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
#include "TConfigNotifier.h"
#include "TCoreException.h"
#include "ErrorCodes.h"

namespace chcore
{
	///////////////////////////////////////////////////////////////////////////////////////////////
	// class TConfigNotifier

	TConfigNotifier::TConfigNotifier(void(*pfnCallback)(const TStringSet&, void*), void* pParam) :
		m_pfnCallback(pfnCallback),
		m_pParam(pParam)
	{
	}

	TConfigNotifier::~TConfigNotifier()
	{
	}

	void TConfigNotifier::operator()(const TStringSet& rsetPropNames)
	{
		if (!m_pfnCallback)
			throw TCoreException(eErr_InvalidPointer, L"m_pfnCallback", LOCATION);

		(*m_pfnCallback)(rsetPropNames, m_pParam);
	}

	TConfigNotifier& TConfigNotifier::operator=(const TConfigNotifier& rNotifier)
	{
		if (this != &rNotifier)
		{
			m_pfnCallback = rNotifier.m_pfnCallback;
			m_pParam = rNotifier.m_pParam;
		}
		return *this;
	}

	bool TConfigNotifier::operator==(const TConfigNotifier& rNotifier) const
	{
		return m_pfnCallback == rNotifier.m_pfnCallback/* && m_pParam == rNotifier.m_pParam*/;
	}
}
