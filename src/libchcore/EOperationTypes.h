// ============================================================================
//  Copyright (C) 2001-2011 by Jozef Starosczyk
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
/// @file  EOperationTypes.h
/// @date  2011/05/26
/// @brief Defines an enumeration type with supported operations.
// ============================================================================
#ifndef __EOPERATIONTYPES_H__
#define __EOPERATIONTYPES_H__

namespace chcore
{
	// enum represents type of the operation handled by the task
	enum EOperationType
	{
		eOperation_None,
		eOperation_Copy,
		eOperation_Move,

		// add new operation types before this enum value
		eOperation_Max
	};
}

#endif
