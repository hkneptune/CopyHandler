/***************************************************************************
 *   Copyright (C) 2004 by Józef Starosczyk                                *
 *   copyhandler@o2.pl                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
/** \file engine_config.h
 *  \brief Contains some compile-time settings for the whole engine.
 */
#ifndef __ENGINE_CONFIG_H__
#define __ENGINE_CONFIG_H__

#ifdef HAVE_CONFIG_H
	#include "config.h"
#endif

#ifdef HAVE_INTTYPES_H
	#include <inttypes.h>
#else
	#include <stddef.h>
#endif

// formatting-related macros
// chars
/// Printf-style format string for displaying char_t value (as char)
#define CHARFMT		"%c"
/// Printf-style format string for displaying uchar_t value (as char)
#define UCHARFMT	CHARFMT

// char related numbers (workaround for (u)chars - values are(should be) converted to (u)short_t)
/// Printf-style format string for displaying char_t as a number (the number has to be converted to short_t)
#define CFMT		"%hd"
/// Printf-style format string for displaying char_t as a hexadecimal number (the number has to be converted to short_t)
#define CXFMT		"0x%.2hx"
/// Printf-style format string for displaying uchar_t as a number (the number has to be converted to ushort_t)
#define UCFMT		"%hu"
/// Printf-style format string for displaying uchar_t as a hexadecimal number (the number has to be converted to ushort_t)
#define UCXFMT		CXFMT

// numbers
// 16-bit
/// Printf-style format string for displaying short_t as a number
#define SFMT		"%hd"
/// Printf-style format string for displaying short_t as a hex number
#define SXFMT		"0x%.4hx"
/// Printf-style format string for displaying ushort_t as a number
#define USFMT		"%hu"
/// Printf-style format string for displaying ushort_t as a hex number
#define USXFMT		SXFMT

// 32-bit
#ifdef _WIN32
	/// Printf-style format string for displaying long_t
	#define LFMT		"%ld"
	/// Printf-style format string for displaying long_t as a hex number
	#define LXFMT		"0x%.8lx"
	/// Printf-style format string for displaying ulong_t
	#define ULFMT		"%lu"
#else
	/// Printf-style format string for displaying long_t
	#define LFMT		"%d"
	/// Printf-style format string for displaying long_t as a hex number
	#define LXFMT		"0x%.8x"
	/// Printf-style format string for displaying ulong_t
	#define ULFMT		"%u"
#endif

/// Printf-style format string for displaying int_t
#define IFMT		LFMT
/// Printf-style format string for displaying int_t as a hex number
#define IXFMT		LXFMT
/// Printf-style format string for displaying uint_t
#define UIFMT		ULFMT
/// Printf-style format string for displaying ulong_t as a hex number
#define ULXFMT		LXFMT
/// Printf-style format string for displaying uint_t as a hex number
#define UIXFMT		ULXFMT

// 64-bit & system dependent
#ifdef _WIN32
	/// Printf-style format string for displaying ulonglong_t as a number
	#define ULLFMT		"%I64u"
	/// Printf-style format string for displaying ulonglong_t as a hex number
	#define ULLXFMT		"0x%.16I64x"
	/// Printf-style format string for displaying longlong_t
	#define LLFMT		"%I64d"
	/// Printf-style format string for displaying longlong_t as a hex number
	#define LLXFMT		ULLXFMT
	
	#ifdef _WIN64
		/// Printf-style format string for displaying intptr_t
		#define IPTRFMT		LLFMT
		/// Printf-style format string for displaying longptr_t
		#define LPTRFMT		LLFMT
		/// Printf-style format string for displaying intptr_t as a hex number
		#define IPTRXFMT	LLXFMT
		/// Printf-style format string for displaying longptr_t as a hex number
		#define LPTRXFMT	LLXFMT
		/// Printf-style format string for displaying uintptr_t
		#define UIPTRFMT	ULLFMT
		/// Printf-style format string for displaying ulongptr_t
		#define ULPTRFMT	ULLFMT
		/// Printf-style format string for displaying uintptr_t as a hex number
		#define UIPTRXFMT	ULLXFMT
		/// Printf-style format string for displaying ulongptr_t as a hex number
		#define ULPTRXFMT	ULLXFMT
	#else
		/// Printf-style format string for displaying intptr_t
		#define IPTRFMT		LFMT
		/// Printf-style format string for displaying longptr_t
		#define LPTRFMT		LFMT
		/// Printf-style format string for displaying intptr_t as a hex number
		#define IPTRXFMT	LXFMT
		/// Printf-style format string for displaying longptr_t as a hex number
		#define LPTRXFMT	LXFMT
		/// Printf-style format string for displaying uintptr_t
		#define UIPTRFMT	ULFMT
		/// Printf-style format string for displaying ulongptr_t
		#define ULPTRFMT	ULFMT
		/// Printf-style format string for displaying uintptr_t as a hex number
		#define UIPTRXFMT	ULXFMT
		/// Printf-style format string for displaying ulongptr_t as a hex number
		#define ULPTRXFMT	ULXFMT
	#endif
#else
	/// Printf-style format string for displaying ulonglong_t as a number
	#define ULLFMT		"%llu"
	/// Printf-style format string for displaying ulonglong_t as a hex number
	#define ULLXFMT		"0x%.16llx"
	/// Printf-style format string for displaying longlong_t
	#define LLFMT		"%lld"
	/// Printf-style format string for displaying longlong_t as a hex number
	#define LLXFMT		ULLXFMT
	
	// FIXME: distinguish between linux 32-bit architecture and 64-bit architecture here
	/// Printf-style format string for displaying intptr_t
	#define IPTRFMT		"%ld"
	/// Printf-style format string for displaying longptr_t
	#define LPTRFMT		IPTRFMT
	/// Printf-style format string for displaying intptr_t as a hex number
	#define IPTRXFMT	"0x%.8lx"
	/// Printf-style format string for displaying longptr_t as a hex number
	#define LPTRXFMT	IPTRXFMT
	/// Printf-style format string for displaying uintptr_t
	#define UIPTRFMT	"%lu"
	/// Printf-style format string for displaying ulongptr_t
	#define ULPTRFMT	UIPTRFMT
	/// Printf-style format string for displaying uintptr_t as a hex number
	#define UIPTRXFMT	"0x%.8lx"
	/// Printf-style format string for displaying ulongptr_t as a hex number
	#define ULPTRXFMT	UIPTRXFMT
#endif

// strings
/// Printf-style format string for displaying ansi strings (char_t based strings)
#define STRFMT		"%s"
#ifdef _WIN32
	/// Printf-style format string for displaying wide strings (wchar_t based strings)
	#define WSTRFMT		"%S"
#else
	/// Printf-style format string for displaying wide strings (wchar_t based strings)
	#define WSTRFMT		"%ls"
#endif

// pointer
/// Printf-style format string for displaying pointers
#define PTRFMT		"%p"

// standard types and formats used throughout the library
// exactly 1 byte
/// Byte type (8bit unsigned int)
typedef unsigned char 	byte_t;

// chars
/// 8bit signed char
typedef char			char_t;
/// 8bit unsigned char
typedef unsigned char	uchar_t;

// system/configuration dependent chars
#if (defined(_WIN32) || defined(_WIN64)) && defined(_UNICODE)
	/// System/configuration dependent character (either wide char or normal one)
	typedef wchar_t		tchar_t;
	/// Macro to be appended to each text in code to be either composed of wide characters or normal ones
	#define _t(text)	L(text)
	/// String formatting string - depending on configuration could display wide char string or normal one.
	#define TSTRFMT		WSTRFMT
#else
	// description as above
	typedef char_t		tchar_t;
	#define _t(text)	text
	#define TSTRFMT		STRFMT
#endif

// 16-bit integers
/// 16bit short integer
typedef short			short_t;
/// 16bit unsigned short integer
typedef unsigned short	ushort_t;

// 32-bit integers
#ifdef _WIN32
	#ifdef _WIN64
		/// 32bit integer
		typedef long			int_t;
		/// 32bit integer
		typedef long			long_t;
		/// 32bit unsigned integer
		typedef unsigned long	uint_t;
		/// 32bit unsigned long
		typedef unsigned long	ulong_t
	#else
		/// 32bit integer
		typedef int				int_t;
		/// 32bit integer
		typedef long			long_t;
		/// 32bit unsigned integer
		typedef unsigned int	uint_t;
		/// 32bit unsigned integer
		typedef unsigned long	ulong_t;
	#endif
#else
	/// 32bit integer
	typedef int					int_t;
	/// 32bit integer
	typedef int					long_t;
	/// 32bit unsigned integer
	typedef unsigned int		uint_t;
	/// 32bit unsigned integer
	typedef unsigned int		ulong_t;
#endif

// 64-bit integers
/// 64bit long long
typedef long long				longlong_t;
/// 64bit unsigned long long
typedef unsigned long long		ulonglong_t;
/// 64bit long long
typedef  longlong_t				ll_t;
/// 64bit unsigned long long
typedef  ulonglong_t			ull_t;

// platform dependent integers (32-bit on 32-bit platforms, 64-bit on 64-bit platforms)
#ifdef _WIN32
	/// platform-dependent size signed integer (32-bit on 32-bit platforms, 64-bit on 64-bit platforms)
//	typedef int				intptr_t;
	/// platform-dependent size signed integer (32-bit on 32-bit platforms, 64-bit on 64-bit platforms)
	typedef intptr_t		longptr_t;
	/// platform-dependent size unsigned integer (32-bit on 32-bit platforms, 64-bit on 64-bit platforms)
//	typedef unsigned int	uintptr_t;
	/// platform-dependent size unsigned integer (32-bit on 32-bit platforms, 64-bit on 64-bit platforms)
	typedef uintptr_t		ulongptr_t;
#else
	// linux and other
	/// platform-dependent size signed integer (32-bit on 32-bit platforms, 64-bit on 64-bit platforms)
//	typedef long				intptr_t;
	/// platform-dependent size signed integer (32-bit on 32-bit platforms, 64-bit on 64-bit platforms)
	typedef intptr_t		longptr_t;
	/// platform-dependent size unsigned integer (32-bit on 32-bit platforms, 64-bit on 64-bit platforms)
//	typedef unsigned long		uintptr_t;
	/// platform-dependent size unsigned integer (32-bit on 32-bit platforms, 64-bit on 64-bit platforms)
	typedef uintptr_t		ulongptr_t;
#endif

// pointer
/// untyped pointer
typedef void*				ptr_t;

#endif
