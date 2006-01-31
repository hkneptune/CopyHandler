/***************************************************************************
 *   Copyright (C) 2004 by J�zef Starosczyk                                *
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
#ifndef __MACROS_H__
#define __MACROS_H__

/** \file macros.h
 *  \brief Contains some helper macros used throughout other files
 */
 
// macros for rounding up and down some values to the nearest ?*chunk
/// Rounding up value to the nearest chunk multiplicity
#define ROUNDUP(val,chunk) ((val + chunk - 1) & ~(chunk-1))
/// Rounding down value to the nearest chunk multiplicity
#define ROUNDDOWN(val,chunk) (val & ~(chunk-1))

// cross-platform __FUNCTION__ macro
#ifndef _WIN32
	/// Some helper for non-windoze systems (unified cross-platform __FUNCTION__ macro)
	#define __FUNCTION__ __PRETTY_FUNCTION__
#endif

// minimum/maximum macros
/// Returns the lesser value from two given as params
#define minval(a,b) ((a) < (b) ? (a) : (b))
/// Returns the greater value from two given as params
#define maxval(a,b) ((a) > (b) ? (a) : (b))

#endif
