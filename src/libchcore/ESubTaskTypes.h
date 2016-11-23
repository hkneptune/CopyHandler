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
/// @file  SubTaskTypes.h
/// @date  2011/11/12
/// @brief File contains subtask types enumeration.
// ============================================================================
#ifndef __SUBTASKTYPES_H__
#define __SUBTASKTYPES_H__

namespace chcore
{
	enum ESubOperationType
	{
		eSubOperation_None,
		eSubOperation_FastMove,
		eSubOperation_Scanning,
		eSubOperation_Copying,
		eSubOperation_Deleting,

		// add new operation types before this one
		eSubOperation_Max
	};
}

#endif
