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

#include <atltrace.h>

#define TRACK_DB_QUERIES
//#define TRACK_DB_QUERIES_DETAILED

#ifdef TRACK_DB_QUERIES
	#define DBTRACE(fmt, val) ATLTRACE(fmt, val)
	#define DBTRACE0(fmt) ATLTRACE(fmt)
	#define DBTRACE2(fmt, val1, val2) ATLTRACE(fmt, val1, val2)

	#ifdef TRACK_DB_QUERIES_DETAILED
		#define DBTRACE_D(fmt, val) ATLTRACE(fmt, val)
		#define DBTRACE0_D(fmt) ATLTRACE(fmt)
		#define DBTRACE2_D(fmt, val1, val2) ATLTRACE(fmt, val1, val2)
	#else
		#define DBTRACE_D(fmt, val) __noop;
		#define DBTRACE0_D(fmt) __noop
		#define DBTRACE2_D(fmt, val1, val2) __noop;
	#endif
#else
	#define DBTRACE(fmt, val) __noop;
	#define DBTRACE0(fmt) __noop
	#define DBTRACE2(fmt, val1, val2) __noop;
	#define DBTRACE_D(fmt, val) __noop;
	#define DBTRACE0_D(fmt) __noop
	#define DBTRACE2_D(fmt, val1, val2) __noop;
#endif

#endif
