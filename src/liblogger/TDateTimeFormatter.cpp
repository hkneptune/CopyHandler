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
#include "TDateTimeFormatter.h"
#include <boost/log/attributes/clock.hpp>
#include <boost/date_time/local_time/custom_time_zone.hpp>

namespace logger
{
	std::wstring TDateTimeFormatter::GetCurrentTime()
	{
		boost::posix_time::ptime currentTime = boost::posix_time::microsec_clock::local_time();
		std::wstringstream wss;
		boost::posix_time::time_facet* facet = new boost::posix_time::time_facet();
		facet->format("%Y-%m-%d %H:%M:%S.%f");
		wss.imbue(std::locale(std::locale::classic(), facet));
		wss << currentTime;

		return wss.str();
	}

}
