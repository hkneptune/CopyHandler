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
#ifndef __LIBLOGGER_H__
#define __LIBLOGGER_H__

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the LIBLOGGER_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// LIBLOGGER_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#if defined(_WIN32) || defined(_WIN64)
	#ifdef LIBLOGGER_EXPORTS
		#define LIBLOGGER_API __declspec(dllexport)
	#else
		#define LIBLOGGER_API __declspec(dllimport)
	#endif
#else
	/** \brief Import/export macros
	*
	*  These macros are being used throughout the whole code. They are meant to
	*  export symbols (if the LIBLOGGER_EXPORTS is defined) from this library
	*  (also for importing (when LIBLOGGER_EXPORTS macro is undefined) in other apps).
	*/
	#define LIBLOGGER_API
#endif

#endif
