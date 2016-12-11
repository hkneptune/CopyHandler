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
#ifndef __TOVERLAPPEDPROCESSORRANGE_H__
#define __TOVERLAPPEDPROCESSORRANGE_H__

#include <boost/signals2/signal.hpp>

namespace chengine
{
	class TOverlappedProcessorRange
	{
	public:
		TOverlappedProcessorRange();
		explicit TOverlappedProcessorRange(unsigned long long ullResumePosition);

		void SetResumePosition(unsigned long long ullResumePosition);
		unsigned long long GetResumePosition() const;

		boost::signals2::signal<void(unsigned long long)>& GetNotifier();

	private:
		unsigned long long m_ullResumePosition = 0;
		boost::signals2::signal<void(unsigned long long)> m_notifier;
	};

	using TOverlappedProcessorRangePtr = std::shared_ptr<TOverlappedProcessorRange>;
}

#endif
