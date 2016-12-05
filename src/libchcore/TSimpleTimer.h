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
#ifndef __TSIMPLETIMER_H__
#define __TSIMPLETIMER_H__

#include "ITimestampProvider.h"

namespace chcore
{
	class LIBCHCORE_API TSimpleTimer
	{
	public:
		explicit TSimpleTimer(bool bAutostart = false, const ITimestampProviderPtr& spTimestampProvider = ITimestampProviderPtr());
		TSimpleTimer(const TSimpleTimer&) = delete;
		~TSimpleTimer();

		TSimpleTimer& operator=(const TSimpleTimer&) = delete;

		void Init(unsigned long long ullTotalTime);

		void Start();
		unsigned long long Stop();		// returns total time
		unsigned long long Tick();		// returns current timestamp

		unsigned long long Checkpoint();	// returns current total time and restarts the timer

		void Reset();

		bool IsRunning() const;
		unsigned long long GetTotalTime() const { return m_ullTotalTime; }
		unsigned long long GetLastTimestamp() const { return m_ullLastTime; }

	private:
#pragma warning(push)
#pragma warning(disable: 4251)
		ITimestampProviderPtr m_spTimestampProvider;
#pragma warning(pop)

		bool m_bStarted;
		unsigned long long m_ullTotalTime;		// total time measured
		unsigned long long m_ullLastTime;		// last processed time
	};
}

#endif
