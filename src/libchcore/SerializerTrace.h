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
#ifndef __SERIALIZERTRACE_H__
#define __SERIALIZERTRACE_H__

#include <atlstr.h>

// enables tracing
#define ENABLE_TRACE

// general tracking
#define TRACK_GENERAL

// db-related tracking
#define TRACK_DB_QUERIES
//#define TRACK_DB_QUERIES_DETAILED

#ifdef ENABLE_TRACE
	inline void trace0(PCTSTR pszFmt)
	{
		OutputDebugString(pszFmt);
	}

	template<class T>
	inline void trace1(PCTSTR pszFmt, const T& tData)
	{
		CString strVal;
		strVal.Format(pszFmt, tData);
		OutputDebugString((PCTSTR)strVal);
	}

	template<class T1, class T2>
	inline void trace2(PCTSTR pszFmt, const T1& tData1, const T2& tData2)
	{
		CString strVal;
		strVal.Format(pszFmt, tData1, tData2);
		OutputDebugString((PCTSTR)strVal);
	}

	#define MYTRACE0 trace0
	#define MYTRACE1 trace1
	#define MYTRACE2 trace2
#else
	#define MYTRACE0(fmt) __noop
	#define MYTRACE1(fmt, val) __noop
	#define MYTRACE2(fmt, val1, val2) __noop
#endif

#ifdef TRACK_GENERAL
	#define GTRACE0 MYTRACE0
	#define GTRACE1 MYTRACE1
	#define GTRACE2 MYTRACE2
#else
	#define GTRACE0(fmt) __noop
	#define GTRACE1(fmt, val) __noop
	#define GTRACE2(fmt, val1, val2) __noop
#endif


#ifdef TRACK_DB_QUERIES
	#define DBTRACE0 MYTRACE0
	#define DBTRACE1 MYTRACE1
	#define DBTRACE2 MYTRACE2

	#ifdef TRACK_DB_QUERIES_DETAILED
		#define DBTRACE0_D MYTRACE0
		#define DBTRACE1_D MYTRACE1
		#define DBTRACE2_D MYTRACE2
	#else
		#define DBTRACE0_D(fmt) __noop
		#define DBTRACE1_D(fmt, val) __noop
		#define DBTRACE2_D(fmt, val1, val2) __noop
	#endif
#else
	#define DBTRACE0(fmt) __noop
	#define DBTRACE1(fmt, val) __noop
	#define DBTRACE2(fmt, val1, val2) __noop
	#define DBTRACE0_D(fmt) __noop
	#define DBTRACE1_D(fmt, val) __noop
	#define DBTRACE2_D(fmt, val1, val2) __noop
#endif

#endif
