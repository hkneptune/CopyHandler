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
#ifndef __TLOGGER_H__
#define __TLOGGER_H__

#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/attributes/named_scope.hpp>
#include "HResultFormatter.h"

// ugly way to include severity_level in the global namespace
using namespace boost::log::trivial;

typedef boost::log::sources::wseverity_logger_mt<severity_level> TLogger;

BOOST_LOG_GLOBAL_LOGGER(Logger, TLogger)

// logging helpers
namespace details
{
	constexpr severity_level HRESULT2Severity(HRESULT hResult)
	{
		return (SUCCEEDED(hResult)) ? debug : error;
	}
}

#define BOOST_LOG_HRESULT(lg, hr)\
	BOOST_LOG_SEV(lg, details::HRESULT2Severity(hr)) << L" <" << HResultFormatter::FormatHResult(hr) << L"> "

#define LOG_PARAM(param)\
	#param << "=" << param

#define LOG_PARAMS2(a1, a2)\
	LOG_PARAM(a1) << ", " << LOG_PARAM(a2)
#define LOG_PARAMS3(a1, a2, a3)\
	LOG_PARAMS2(a1, a2) << ", " << LOG_PARAM(a3)
#define LOG_PARAMS4(a1, a2, a3, a4)\
	LOG_PARAMS3(a1, a2, a3) << ", " << LOG_PARAM(a4)
#define LOG_PARAMS5(a1, a2, a3, a4, a5)\
	LOG_PARAMS4(a1, a2, a3, a4) << ", " << LOG_PARAM(a5)
#define LOG_PARAMS6(a1, a2, a3, a4, a5, a6)\
	LOG_PARAMS5(a1, a2, a3, a4, a5) << ", " << LOG_PARAM(a6)
#define LOG_PARAMS7(a1, a2, a3, a4, a5, a6, a7)\
	LOG_PARAMS6(a1, a2, a3, a4, a5, a6) << ", " << LOG_PARAM(a7)
#define LOG_PARAMS8(a1, a2, a3, a4, a5, a6, a7, a8)\
	LOG_PARAMS7(a1, a2, a3, a4, a5, a6, a7) << ", " << LOG_PARAM(a8)
#define LOG_PARAMS9(a1, a2, a3, a4, a5, a6, a7, a8, a9)\
	LOG_PARAMS8(a1, a2, a3, a4, a5, a6, a7, a8) << ", " << LOG_PARAM(a9)
#define LOG_PARAMS10(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10)\
	LOG_PARAMS9(a1, a2, a3, a4, a5, a6, a7, a8, a9) << ", " << LOG_PARAM(a10)

#endif
