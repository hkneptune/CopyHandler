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
#include "TSimpleTimer.h"
#include "TTimestampProviderTickCount.h"

BEGIN_CHCORE_NAMESPACE

TSimpleTimer::TSimpleTimer(bool bAutostart, const ITimestampProviderPtr& spTimestampProvider) :
	m_spTimestampProvider(spTimestampProvider),
	m_bStarted(false),
	m_ullLastTime(0),
	m_ullTotalTime(0)
{
	if(!spTimestampProvider)
		m_spTimestampProvider = ITimestampProviderPtr(new TTimestampProviderTickCount);

	if(bAutostart)
		Start();
}

TSimpleTimer::~TSimpleTimer()
{
}

void TSimpleTimer::Start()
{
	if(!m_bStarted)
	{
		m_bStarted = true;
		m_ullLastTime = m_spTimestampProvider->GetCurrentTimestamp();
	}
}

unsigned long long TSimpleTimer::Stop()
{
	if(m_bStarted)
	{
		Tick();
		m_bStarted = false;
	}

	return m_ullTotalTime;
}

unsigned long long TSimpleTimer::Tick()
{
	unsigned long long ullCurrent = m_spTimestampProvider->GetCurrentTimestamp();
	if(m_bStarted)
		m_ullTotalTime += ullCurrent - m_ullLastTime;
	m_ullLastTime = ullCurrent;

	return ullCurrent;
}

void TSimpleTimer::Reset()
{
	m_bStarted = false;
	m_ullLastTime = 0;
	m_ullTotalTime = 0;
}

END_CHCORE_NAMESPACE
